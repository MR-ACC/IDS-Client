#include "videowidget.h"

void videowidget_render_frame_cb(gpointer priv, ImageInfo *piinfo)
{
    VideoWidget *window = (VideoWidget *)priv;

    if (false == window->mImgMutex.tryLock())
        return;

    window->mUpdateFlag = true;
    window->mImgInfoClone.img_flag = piinfo->img_flag;

    if (piinfo->img_flag == CV_IMG_TYPE_DEFAULT)
    {
        window->mImgInfoClone.fmt = piinfo->fmt;
        window->mImgInfoClone.width = piinfo->width;
        window->mImgInfoClone.height = piinfo->height;
        window->mImgInfoClone.linesize = piinfo->linesize;
        if (window->mImgInfoClone.buf == NULL)
            window->mImgInfoClone.buf = (unsigned char *)malloc(window->mImgInfoClone.linesize * window->mImgInfoClone.height);
        memcpy(window->mImgInfoClone.buf, piinfo->buf, window->mImgInfoClone.linesize * window->mImgInfoClone.height);
    }
    else if (piinfo->img_flag == CV_IMG_TYPE_OPENCV_CPU)
    {
        qDebug () << __func__ << __LINE__ << "error. bad use (old version). do not use CV_IMG_TYPE_OPENCV_CPU img_flag.";
        Q_ASSERT(0);
    }
    else if (piinfo->img_flag == CV_IMG_TYPE_OPENCV_CUDA_GPU)
    {
#ifdef HAVE_OPENCV_CUDA
//        if (window->mImgInfoClone.cv_img == NULL)
//            window->mImgInfoClone.cv_img = (gpointer)new GpuMat(((GpuMat *)piinfo->cv_img)->size(), CV_8UC3);
//        ((GpuMat *)piinfo->cv_img)->copyTo(*(GpuMat *)(window->mImgInfoClone.cv_img));
        window->mImgInfoClone.cv_img = piinfo->cv_img;
#else
        qDebug () << __func__ << __LINE__ << "VIDEOWIDGET is compiled without opencv cuda support.";
        Q_ASSERT(0);
#endif
    }

    window->mImgMutex.unlock();
}

#ifdef HAVE_OPENCV_OPENGL
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

    mUpdateFlag = false;
    mImgInfoClone.buf = NULL;
    mImgInfoClone.cv_img = NULL;

    mPlayer = NULL;
    mStatusText = QString("连接...");
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(renderOneFrame()));
    mTimer.start(TIME_PER_FRAME);

#ifdef HAVE_OPENCV_OPENGL
    m_OglTexValid = false;
#endif
}

VideoWidget::~VideoWidget()
{
    if (mImgInfoClone.buf)
        free(mImgInfoClone.buf);
}

void VideoWidget::startPlay(gchar *rtsp_url)
{
    gchar *rtsp_urls[] = {rtsp_url};
    startPlay(rtsp_urls, 1);
}

void VideoWidget::startPlay(gchar *rtsp_urls[], gint nums)
{
    gint i;
    gint win_flags, player_flags, draw_fmt;
    for (i=0; i<nums; i++)
     if (rtsp_urls[i] == NULL || rtsp_urls[i][0] == 0)
         break;
    nums = i;
    if (nums == 1)
    {
        player_flags = 0;
        win_flags = 0;
    }
    else
    {
        player_flags = IDS_TYPE(IDS_TYPE_STITCH);
        win_flags = IDS_USE_THE_SAME_WINDOW;
    #ifdef HAVE_OPENCV_CUDA
        win_flags |= IDS_ENABLE_CV_CUDA_ACCEL;
    #endif
    }
#ifdef HAVE_OPENCV_OPENGL
    draw_fmt = IDS_FMT_RGB24;
#else
    draw_fmt = IDS_FMT_RGB32; //fixme, YUV420 should be ok too.
#endif

    startPlayExperts(rtsp_urls, nums, win_flags, player_flags, draw_fmt);
}

void VideoWidget::startPlayExperts(gchar *rtsp_urls[], gint nums, gint win_flags, gint player_flags, gint draw_fmt)
{
    Q_ASSERT(mPlayer == NULL);
    if (nums < 1)
        mStatusText = "路径无效";
    else
    {
        gchar urls[IPC_CFG_STITCH_CNT][256];
        int i;
        WindowInfo winfo[IPC_CFG_STITCH_CNT];
        for (i=0; i<nums; i++)
        {
            strcpy(urls[i], rtsp_urls[i]);
            winfo[i].media_url = urls[i];
            winfo[i].win_w = width()/16*16; //fixme.
            winfo[i].win_h = height();
            winfo[i].win_id = GUINT_TO_POINTER(winId());
            winfo[i].win_id = GUINT_TO_POINTER(winId()); //hack.... do not delete me!!!
            winfo[i].win_id = GUINT_TO_POINTER(winId()); //hack.... do not delete me!!!
            winfo[i].flags = win_flags;
            winfo[i].draw_fmt = draw_fmt;
            if (draw_fmt == IDS_FMT_YUV420P)
            {
                winfo[i].draw = 0;
                winfo[i].priv = 0;
            }
            else
            {
                winfo[i].draw = videowidget_render_frame_cb;
                winfo[i].priv = this;
            }
        }

        int ret = ids_play_stream(&winfo[0], nums, player_flags, NULL, this, &mPlayer);
        if (ret < nums)
        {
            if (ret > 0) //ret > 0 but ret <nums means stitching failed, some of the urls is not actived.
            {
                Q_ASSERT(mPlayer != NULL);
                ids_stop_stream(mPlayer);
                mPlayer = NULL;
            }
            mStatusText = QString("连接失败");
            for (i=0; i<nums; i++)
            {
                mStatusText += QString("\n");
                mStatusText += QString(rtsp_urls[i]);
            }
        }
        else
            mStatusText = QString("");
    }

    this->update();
}

void VideoWidget::stopPlay()
{
    mTimer.stop();
    if (mPlayer != NULL)
    {
        ids_stop_stream(mPlayer);
        mPlayer = NULL;
    }
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
        rect = QRect(0, 0, width(), height());
        painter.setPen(QColor(10,10,10));
        painter.drawRect(rect);
        painter.setPen(QColor(180,180,180));
        painter.drawText(rect, Qt::AlignCenter, mStatusText);
    }

#ifndef HAVE_OPENCV_OPENGL
    if (mImgInfoClone.buf != NULL && mImgInfoClone.img_flag == CV_IMG_TYPE_DEFAULT)
    {
        QImage image;
        if (mImgInfoClone.fmt == IDS_FMT_RGB32)
            image = QImage((const unsigned char*)mImgInfoClone.buf,
                                  mImgInfoClone.width, mImgInfoClone.height, mImgInfoClone.linesize, QImage::Format_RGB32); //mImgInfoClone.fmt
        else if (mImgInfoClone.fmt == IDS_FMT_RGB24)
            image = QImage((const unsigned char*)mImgInfoClone.buf,
                                  mImgInfoClone.width, mImgInfoClone.height, mImgInfoClone.linesize, QImage::Format_RGB888); //mImgInfoClone.fmt
        else
        {
            qDebug() << __func__ << __LINE__ << "error. does not supprot img fmt: " << mImgInfoClone.fmt << endl;
            return;
        }
        QPixmap pixmap = QPixmap::fromImage( image.scaled(size(), Qt::KeepAspectRatio) );
        int w = width()-2;
        int h = height()-2;
        double ratio = MIN((double)w/mImgInfoClone.width, (double)h/mImgInfoClone.height);
        int dx = (w - ratio * mImgInfoClone.width) / 2;
        int dy = (h - ratio * mImgInfoClone.height) / 2;
        painter.drawPixmap(dx+1,dy+1, pixmap);
    }
#endif
}

void VideoWidget::renderOneFrame()
{
    if (false == mImgMutex.tryLock())
        return;

    if (mUpdateFlag)
    {
        mUpdateFlag = false;
        if (mImgInfoClone.img_flag == CV_IMG_TYPE_DEFAULT)
        {
            Q_ASSERT(mImgInfoClone.fmt != IDS_FMT_YUV420P);
#ifdef HAVE_OPENCV_OPENGL
            if (mImgInfoClone.fmt == IDS_FMT_RGB24)
                showCvImg(Mat(mImgInfoClone.height, mImgInfoClone.width, CV_8UC3, mImgInfoClone.buf, mImgInfoClone.linesize));
            else if (mImgInfoClone.fmt == IDS_FMT_RGB32)
                showCvImg(Mat(mImgInfoClone.height, mImgInfoClone.width, CV_8UC4, mImgInfoClone.buf, mImgInfoClone.linesize));
            else
            {
                qDebug() << __func__ << __LINE__ << "error. does not support img fmt: " << mImgInfoClone.fmt << endl;
                Q_ASSERT(0);
            }
#else
            repaint();
#endif
        }
        else if (mImgInfoClone.img_flag == CV_IMG_TYPE_OPENCV_CUDA_GPU)
        {
#ifdef HAVE_OPENCV_CUDA
            this->showCvImg(*(GpuMat*)(mImgInfoClone.cv_img));
#else
            qDebug () << __func__ << __LINE__ << "VIDEOWIDGET is compiled without opencv cuda support.";
            Q_ASSERT(0);
#endif
        }
        else
        {
            qDebug () << __func__ << __LINE__ << "error. does not support img flag: " << mImgInfoClone.img_flag << endl;
            Q_ASSERT(0);
        }
    }
    mImgMutex.unlock();
}

#ifdef HAVE_OPENCV_OPENGL
void VideoWidget::showCvImg(InputArray _img)
{
    this->makeCurrent();

    if (_img.kind() == _InputArray::CUDA_GPU_MAT)
    {
        mOglBuf.copyFrom(_img);
        mOglBuf.setAutoRelease(false);

        mOglTex.copyFrom(mOglBuf);
        mOglTex.setAutoRelease(false);
    }
    else if  (_img.kind() == _InputArray::MAT)
    {
        mOglTex.copyFrom(_img);
        mOglTex.setAutoRelease(false);
    }
    else
        Q_ASSERT(0);
    m_OglTexValid = true;

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
    if (m_OglTexValid == false)
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
