#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#define IDS_CV_CUDA_HWACCEL_SUPPORTED
//#define IDS_SERVER_RENDER_OPENGL

#ifdef IDS_CV_CUDA_HWACCEL_SUPPORTED
    #define IDS_SERVER_RENDER_OPENGL
#else
    #define IDS_SERVER_RENDER_SDL
#endif

#include "ids.h"

#ifdef IDS_SERVER_RENDER_OPENGL

#include "opencv2/opencv_modules.hpp"
#include <opencv2/core.hpp>
#include <opencv2/core/opengl.hpp>
#include <opencv2/cudacodec.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudawarping.hpp>
using namespace cv;
using namespace cuda;
#include <QtOpenGL/QGLWidget>
#include <QTimer>
#include <QtGui>
#include <QDebug>
#include <QApplication>

#define VideoWidget GLVideoWidget

void glvideowidget_render_frame_cb(gpointer priv, gpointer buf, gint buf_type);

class GLVideoWidget : public QGLWidget {

    Q_OBJECT // must include this if you use Qt signals/slots

public:
    GLVideoWidget(QWidget *parent = NULL);
    void    stopRender();

    QString           mStatusText;
    void                *mImgBuf;
    int                   mImgBufType;
    bool                mInitFlag;
    bool                mUpdateFlag;

public slots:
    void renderOneFrame();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void paintEvent(QPaintEvent* event);

private:
    void imshow(InputArray _img);

    QTimer *            mTimer;
    cv::ogl::Texture2D  mOglTex;
    cv::ogl::Buffer     mOglBuf;
};

#else // #ifdef IDS_SERVER_RENDER_OPENGL

#include <QWidget>
#include <QtGui>
#include <QDebug>
#include <QApplication>

class VideoWidget : public QWidget {
    Q_OBJECT
    public:
    void    stopRender() {};
    VideoWidget(QWidget *parent);
    ~VideoWidget();
    QString           mStatusText;
    protected:
    void paintEvent(QPaintEvent* event);
};

#endif //#ifdef IDS_SERVER_RENDER_OPENGL

#endif // VIDEOWIDGET_H
