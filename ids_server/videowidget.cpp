#include "videowidget.h"

#ifdef IDS_SERVER_RENDER_OPENGL

#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glut.h"

void glvideowidget_render_frame_cb(gpointer priv, gpointer buf, gint buf_type)
{
    GLVideoWidget *window = (GLVideoWidget *)priv;

    //window lock
    window->mImgBuf = buf;
    window->mImgBufType  = buf_type;
    window->mUpdateFlag = true;
    //window unlock
}

GLVideoWidget::GLVideoWidget(QWidget *parent) :
    QGLWidget((QGLWidget *)parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(0,20,0));
    this->setPalette(palette);

    mStatusText = "    ";//blank
    mInitFlag = false;
    mImgBuf = NULL;
    mUpdateFlag = false;

    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(renderOneFrame()));
    mTimer->start(20); //fixme
}

void GLVideoWidget::stopRender()
{
    mTimer->stop();
}

void GLVideoWidget::renderOneFrame()
{
    if (mInitFlag == false)
    {
        mInitFlag = true;
        this->makeCurrent();
        this->updateGL();
        return;
    }
    if (mImgBuf == NULL)
        return;

    if (mUpdateFlag)
    {
        //GLVideoWidget lock
        mUpdateFlag = false;
        if (mImgBufType == CV_IMG_TYPE_OPENCV_CPU)
            this->imshow(*(cv::Mat *)mImgBuf);
        else
            this->imshow(*(cv::cuda::GpuMat*)mImgBuf);
        //GLVideoWidget unlock
    }
}

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
//    glDisable(GL_TEXTURE_2D);
//    glDisable(GL_DEPTH_TEST);
//    glDisable(GL_COLOR_MATERIAL);
//    glEnable(GL_BLEND);
//    glEnable(GL_POLYGON_SMOOTH);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0, 0, 0, 0);
}

void GLVideoWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    gluOrtho2D(0, w, 0, h); // set origin to bottom left corner
//    glMatrixMode(GL_MODELVIEW);
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

void GLVideoWidget::paintEvent(QPaintEvent* event)
{
    if (mStatusText != "")
    {
        QPainter painter(this);
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
}

#else // #ifdef IDS_SERVER_RENDER_OPENGL

VideoWidget::VideoWidget(QWidget *parent) :
    QWidget(parent)
{
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(0,50,0));
    this->setPalette(palette);
}

VideoWidget::~VideoWidget()
{
}

void VideoWidget::paintEvent(QPaintEvent* event)
{
    if (mStatusText != "")
    {
        QPainter painter(this);
        QFont font = QApplication::font();
        font.setPixelSize(20);
        painter.setFont(font);
        QRect rect;
        rect = QRect(5, 5, width()-10, height()-10);
        painter.setPen(QColor(30,30,30));
        painter.drawRect(rect);
        painter.setPen(QColor(180,180,180));
        painter.drawText(rect, Qt::AlignCenter, mStatusText);
    }
}

#endif //#ifdef IDS_SERVER_RENDER_OPENGL
