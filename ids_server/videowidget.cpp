#include "videowidget.h"

#ifndef IDS_SERVER_RENDER_SDL
void videowidget_render_frame_cb(gpointer priv, ImageInfo *piinfo)
{
    VideoWidget *window = (VideoWidget *)priv;
    //window lock
    window->mImgInfo = piinfo;
    window->mUpdateFlag = true;
    //window unlock
}
#endif

VideoWidget::VideoWidget(QWidget *parent) :
#ifdef IDS_SERVER_RENDER_OPENGL
    QGLWidget((QGLWidget *)parent)
#else
    QWidget(parent)
#endif
{
    setWindowFlags(Qt::FramelessWindowHint);
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(30,30,30));
    this->setPalette(palette);
    this->setAutoFillBackground(true);

    mStatusText = "";//blank

#ifndef IDS_SERVER_RENDER_SDL
    mImgInfo = NULL;
    mUpdateFlag = false;
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(renderOneFrame()));
    mTimer->start(TIME_PER_FRAME); //fixme
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

    if (mStatusText != "")
    {
        QFont font = QApplication::font();
        font.setPixelSize(20);
        painter.setFont(font);
        QRect rect;
        rect = QRect(5, 5, width()-10, height()-10);
        painter.setPen(QColor(80,80,80));
        painter.drawRect(rect);
        painter.setPen(QColor(180,180,180));
        painter.drawText(rect, Qt::AlignCenter, mStatusText);
    }

#ifdef IDS_SERVER_RENDER_USER
    if (mImgInfo != NULL)
    {
        QImage image = QImage((const unsigned char*)mImgInfo->buf,
                             mImgInfo->width, mImgInfo->height, mImgInfo->linesize, QImage::Format_RGB32); //mImgInfo->fmt
        QPixmap pixmap = QPixmap::fromImage( image.scaled(size(), Qt::KeepAspectRatio) );
        double ratio = MIN((double)width()/mImgInfo->width, (double)height()/mImgInfo->height);
        int dx = (width() - ratio * mImgInfo->width) / 2;
        int dy = (height() - ratio * mImgInfo->height) / 2;
        painter.drawPixmap(dx,dy, pixmap);
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

    if (mUpdateFlag)
    {
        //VideoWidget lock
        mUpdateFlag = false;
        Q_ASSERT(mImgInfo != NULL);
#ifdef IDS_SERVER_RENDER_USER
        repaint();
#elif defined IDS_SERVER_RENDER_OPENGL

        if (mImgInfo->cv_img_flag == CV_IMG_TYPE_OPENCV_CPU)
            this->imshow(*(cv::Mat *)(mImgInfo->cv_img));
        else
            this->imshow(*(cv::cuda::GpuMat*)(mImgInfo->cv_img));
#endif
        //VideoWidget unlock
    }
}
#endif

#ifdef IDS_SERVER_RENDER_OPENGL
void GLVideoWidget::imshow(InputArray _img)
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

void GLVideoWidget::initializeGL() {
    glClearColor(0, 0, 0, 0);
}

void GLVideoWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    glLoadIdentity();
}

void GLVideoWidget::paintGL() {
    if (mImgBuf == NULL)
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
