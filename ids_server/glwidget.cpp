#include "glwidget.h"
#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glut.h"
#include <QDebug>

void glwidget_render_frame_cb(gpointer priv, gpointer buf, gint buf_flag)
{
    GLWidget *window = (GLWidget *)priv;

    //qDebug() << "render frame";

    //window lock
    window->mImgBuf = buf;
    window->mImgBufFlag  = buf_flag;
    window->mUpdateFlag = true;
    //window unlock
}

GLWidget::GLWidget(QGLWidget *parent) :
    QGLWidget(parent)
{
    mImgBuf = NULL;
    mUpdateFlag = false;

    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(renderOneFrame()));
    mTimer->start(20);
}

void GLWidget::renderOneFrame()
{
    //GLWidget lock
    if (mImgBuf == NULL)
    {
        this->makeCurrent();
        this->updateGL();
        return;
    }
    if (mUpdateFlag)
    {
        mUpdateFlag = false;
        if (mImgBufFlag == CV_IMG_TYPE_OPENCV_CPU)
            this->imshow(*(cv::Mat *)mImgBuf);
        else
            this->imshow(*(cv::cuda::GpuMat*)mImgBuf);
    }
    //GLWidget unlock
}

void GLWidget::imshow(InputArray _img)
{
    this->makeCurrent();

    if (_img.kind() == _InputArray::CUDA_GPU_MAT)
    {
        //qDebug() << "in gpu mat";
        mOglBuf.copyFrom(_img);
        mOglBuf.setAutoRelease(false);

        mOglTex.copyFrom(mOglBuf);
        mOglTex.setAutoRelease(false);
    }
    else
    {
        //qDebug() << "not in gpu mat";
        mOglTex.copyFrom(_img);
        mOglTex.setAutoRelease(false);
    }

    this->updateGL();
}

void GLWidget::initializeGL() {
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0, 0, 0, 0);
}

void GLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h); // set origin to bottom left corner
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void GLWidget::paintGL() {
    if (mImgBuf == NULL)
    {
        //qDebug() << "paintGL. opengl";
        glClear(GL_COLOR_BUFFER_BIT);
        glColor3f(0.5,0.5,0);
        glBegin(GL_POLYGON);
        glVertex2f(0,0);
        glVertex2f(100,500);
        glVertex2f(500,100);
        glEnd();
    }
    else
    {
        //qDebug() << "paintGL. show tex";
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cv::ogl::render(mOglTex);
    }
}
