#include "videowidget.h"

#ifndef IDS_SERVER_RENDER_SDL
void videowidget_render_frame_cb(gpointer priv, ImageInfo *piinfo)
{
    VideoWidget *window = (VideoWidget *)priv;

    if (false == window->mMutex.tryLock())
        return;
    window->mUpdateFlag = true;
#ifdef IDS_SERVER_RENDER_USER
    if (window->mImgInfoClone == NULL)
    {
        window->mImgInfoClone = new ImageInfo;
        window->mImgInfoClone->fmt = piinfo->fmt;
        window->mImgInfoClone->width = piinfo->width;
        window->mImgInfoClone->height = piinfo->height;
        window->mImgInfoClone->linesize = piinfo->linesize;
        window->mImgInfoClone->buf = (guchar *)malloc(window->mImgInfoClone->linesize * window->mImgInfoClone->height);
    }
    memcpy(window->mImgInfoClone->buf, piinfo->buf, window->mImgInfoClone->linesize * window->mImgInfoClone->height);
#elif defined IDS_SERVER_RENDER_OPENGL
    if (window->mImgInfoClone == NULL)
    {
        window->mImgInfoClone = new ImageInfo;
        window->mImgInfoClone->cv_img = new Mat(piinfo->height, piinfo->width, CV_8UC3);
    }
    if (piinfo->cv_img_flag == CV_IMG_TYPE_OPENCV_CPU)
        ((Mat *)piinfo->cv_img)->copyTo(*(Mat *)(window->mImgInfoClone->cv_img));
#ifdef IDS_SERVER_RENDER_OPENGL_CUDA
    else
        window->mImgInfoClone->cv_img = piinfo->cv_img;
#endif
    window->mImgInfoClone->cv_img_flag = piinfo->cv_img_flag;
#endif
    window->mMutex.unlock();
}
#endif

#ifdef IDS_SERVER_RENDER_OPENGL
VideoWidget::VideoWidget(QWidget *parent) :
    QGLWidget(parent)
#else
VideoWidget::VideoWidget(QWidget *parent) :
    QWidget(parent)
#endif
{
    setWindowFlags(Qt::FramelessWindowHint);
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(5,5,5));
    this->setPalette(palette);
    this->setAutoFillBackground(true);

    mStatusText = "        ";//blank

#ifndef IDS_SERVER_RENDER_SDL
    mUpdateFlag = false;
    mImgInfoClone = NULL;
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(renderOneFrame()));
    mTimer->start(TIME_PER_FRAME); //fixme
#endif
}

VideoWidget::~VideoWidget()
{
#ifndef IDS_SERVER_RENDER_SDL
    if (mImgInfoClone != NULL)
        delete mImgInfoClone;
#endif
}

void VideoWidget::stopRender()
{
#ifndef IDS_SERVER_RENDER_SDL
    mTimer->stop();
#endif
}

void VideoWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    QFont font = QApplication::font();
    font.setPixelSize(20);
    painter.setFont(font);
    QRect rect;
//        rect = QRect(1, 1, width()-2, height()-2);
    rect = QRect(0, 0, width(), height());
    painter.setPen(QColor(10,10,10));
    painter.drawRect(rect);
    painter.setPen(QColor(180,180,180));
    painter.drawText(rect, Qt::AlignCenter, mStatusText);

#ifdef IDS_SERVER_RENDER_USER
    if (mImgInfoClone != NULL && mImgInfoClone->buf  != NULL)
    {
        QImage image = QImage((const unsigned char*)mImgInfoClone->buf,
                             mImgInfoClone->width, mImgInfoClone->height, mImgInfoClone->linesize, QImage::Format_RGB32); //mImgInfoClone->fmt
        QPixmap pixmap = QPixmap::fromImage( image.scaled(size(), Qt::KeepAspectRatio) );
        int w = width()-2;
        int h = height()-2;
        double ratio = MIN((double)w/mImgInfoClone->width, (double)h/mImgInfoClone->height);
        int dx = (w - ratio * mImgInfoClone->width) / 2;
        int dy = (h - ratio * mImgInfoClone->height) / 2;
        painter.drawPixmap(dx+1,dy+1, pixmap);
    }
#endif
}

#ifndef IDS_SERVER_RENDER_SDL
void VideoWidget::renderOneFrame()
{
//    if (mInitFlag == false)
//    {
//        mInitFlag = true;
//        this->makeCurrent();
//        this->updateGL();
//        return;
//    }

    if (false == mMutex.tryLock())
        return;

    if (mUpdateFlag)
    {
        mUpdateFlag = false;
        //VideoWidget lock
        Q_ASSERT(mImgInfoClone != NULL);
#ifdef IDS_SERVER_RENDER_USER
        repaint();
#elif defined IDS_SERVER_RENDER_OPENGL
        if (mImgInfoClone->cv_img_flag == CV_IMG_TYPE_OPENCV_CPU)
            this->imshow(*(Mat *)(mImgInfoClone->cv_img));
        else
            this->imshow(*(cuda::GpuMat*)(mImgInfoClone->cv_img));
#endif
    }
    mMutex.unlock();
}
#endif

#ifdef IDS_SERVER_RENDER_OPENGL
void VideoWidget::imshow(InputArray _img)
{
    this->makeCurrent();

    if (_img.kind() == _InputArray::CUDA_GPU_MAT)
    {
        mOglBuf.copyFrom(_img);
        mOglBuf.setAutoRelease(false);

        mOglTex.copyFrom(mOglBuf);
        mOglTex.setAutoRelease(false);
    }
    else
    {
        mOglTex.copyFrom(_img);
        mOglTex.setAutoRelease(false);
    }

    this->updateGL();
}

void VideoWidget::initializeGL() {
    glClearColor(0, 0, 0, 0);
}

void VideoWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    glLoadIdentity();
}

void VideoWidget::paintGL() {
    if (mImgInfoClone == NULL)
    {
        glClear(GL_COLOR_BUFFER_BIT);
    }
    else
    {
        double ratio = MIN( (double)this->width() / mOglTex.cols(), (double)this->height() / mOglTex.rows());
        double w = ratio * mOglTex.cols() / this->width();
        double h = ratio * mOglTex.rows() / this->height();
        Rect_<double> wndRect((1-w)/2, (1-h)/2, w, h);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cv::ogl::render(mOglTex, wndRect);
    }
}
#endif
