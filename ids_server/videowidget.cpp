#include "videowidget.h"

void PlayerThread::run() {
    ((VideoWidget *)mPriv)->playerThreadFunc();
}

void videowidget_render_frame_cb(gpointer priv, ImageInfo *piinfo) {
    ((VideoWidget *)priv)->frameCallbackFunc(piinfo);
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

    connect(this, SIGNAL(playStatusChanged(QString)), this, SLOT(playStatusChangedSlot(QString)));

    mUpdateFlag = false;
    mImgInfoClone.buf = NULL;
    mImgInfoClone.cv_img = NULL;

    mPlayer = NULL;
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(renderFrameSlot()));
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
    mNums = nums;
    if (mNums < 1)
    {
        mStatusText = "路径无效";
        repaint();
        return;
    }

    mPlayerFlags = player_flags;
    int i;
    mStatusText = QString("连接中...");
    for (i=0; i<mNums; i++)
    {
        mStatusText += QString("\n") + QString(rtsp_urls[i]);

        strcpy(mUrls[i], rtsp_urls[i]);
        mWinfo[i].media_url = mUrls[i];
        mWinfo[i].win_w = width()/16*16; //fixme.
        mWinfo[i].win_h = height();
        mWinfo[i].win_id = GUINT_TO_POINTER(winId());
        mWinfo[i].win_id = GUINT_TO_POINTER(winId()); //hack.... do not delete me!!!
        mWinfo[i].win_id = GUINT_TO_POINTER(winId()); //hack.... do not delete me!!!
        mWinfo[i].flags = win_flags;
        mWinfo[i].draw_fmt = draw_fmt;
        if (draw_fmt == IDS_FMT_YUV420P)
        {
            mWinfo[i].draw = 0;
            mWinfo[i].priv = 0;
        }
        else
        {
            mWinfo[i].draw = videowidget_render_frame_cb;
            mWinfo[i].priv = this;
        }
    }
    repaint();

    mPlayerThread.mPriv = (void *)this;
    mPlayerThread.start();
}

void VideoWidget::stopPlay()
{
    mMutex.lock();
    mImgMutex.lock();

    mUpdateFlag = false;
    mTimer.stop();
    if (mPlayer != NULL)
    {
        ids_stop_stream(mPlayer);
        mPlayer = NULL;
    }

    mImgMutex.unlock();
    mMutex.unlock();
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

void VideoWidget::frameCallbackFunc(ImageInfo *piinfo)
{
    mMutex.lock();

    if (false == mImgMutex.tryLock())
    {
        mMutex.unlock();
        return;
    }

    mUpdateFlag = true;
    mImgInfoClone.img_flag = piinfo->img_flag;

    if (piinfo->img_flag == CV_IMG_TYPE_DEFAULT)
    {
        mImgInfoClone.fmt = piinfo->fmt;
         mImgInfoClone.width = piinfo->width;
        mImgInfoClone.height = piinfo->height;
        mImgInfoClone.linesize = piinfo->linesize;
        if (mImgInfoClone.buf == NULL)
            mImgInfoClone.buf = (unsigned char *)malloc(mImgInfoClone.linesize * mImgInfoClone.height);
        memcpy(mImgInfoClone.buf, piinfo->buf, mImgInfoClone.linesize * mImgInfoClone.height);
    }
    else if (piinfo->img_flag == CV_IMG_TYPE_OPENCV_CPU)
    {
        qDebug () << __func__ << __LINE__ << "error. bad use (old version). do not use CV_IMG_TYPE_OPENCV_CPU img_flag.";
        Q_ASSERT(0);
    }
    else if (piinfo->img_flag == CV_IMG_TYPE_OPENCV_CUDA_GPU)
    {
#ifdef HAVE_OPENCV_CUDA
//        if (mImgInfoClone.cv_img == NULL)
//            mImgInfoClone.cv_img = (gpointer)new GpuMat(((GpuMat *)piinfo->cv_img)->size(), CV_8UC3);
//        ((GpuMat *)piinfo->cv_img)->copyTo(*(GpuMat *)(mImgInfoClone.cv_img));
        mImgInfoClone.cv_img = piinfo->cv_img;
#else
        qDebug () << __func__ << __LINE__ << "VIDEOWIDGET is compiled without opencv cuda support.";
        Q_ASSERT(0);
#endif
    }
    mImgMutex.unlock();

    mMutex.unlock();
}

void VideoWidget::renderFrameSlot()
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

void VideoWidget::playerThreadFunc()
{
    Q_ASSERT(mPlayer == NULL);
    mMutex.lock();
    QString status;
    gint ret = -1;
    gint playerFlags = -1;

    ret = ids_play_stream(&mWinfo[0], mNums, mPlayerFlags, NULL, this, &mPlayer);
    if (ret != 0)
        playerFlags = ids_play_get_flags(mPlayer);

    if (ret == mNums && playerFlags == mPlayerFlags) //play success
    {
        status = QString("");
    }
    else
    {
        if (ret > 0) //ret > 0 but ret <nums means stitching failed, some of the urls is not actived.
        {
            Q_ASSERT(mPlayer != NULL);
            ids_stop_stream(mPlayer);
            mPlayer = NULL;
        }

        if (ret != mNums)
        {
            status = QString("连接失败");
            int i;
            for (i=0; i<mNums; i++)
            {
                status += QString("\n");
                status += QString(mUrls[i]);
            }
        }
        else if ( ( 0 != (mPlayerFlags & IDS_TYPE(IDS_TYPE_STITCH)) ) &&
                     ( 0 == (playerFlags & IDS_TYPE(IDS_TYPE_STITCH)) ) )
        {
            status = QString("拼接视频播放失败. 请检查(更新)拼接配置文件");
        }
    }

    emit playStatusChanged(status);
    mMutex.unlock();
}

void VideoWidget::playStatusChangedSlot(QString status)
{
    mStatusText = status;
    update();
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
//        glClear(GL_COLOR_BUFFER_BIT);
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
