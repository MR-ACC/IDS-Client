#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

//#define IDS_SERVER_RENDER_OPENGL
#define IDS_SERVER_RENDER_SDL
//#define IDS_SERVER_RENDER_USER

#define TIME_PER_FRAME 100

#include "ids.h"

#include <QTimer>
#include <QtGui>
#include <QDebug>
#include <QApplication>
#include <QWidget>

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
    #include "GL/gl.h"
    #include "GL/glu.h"
    #include "GL/glut.h"
#endif

#ifndef IDS_SERVER_RENDER_SDL
void videowidget_render_frame_cb(gpointer priv, ImageInfo *piinfo);
#endif

#ifdef IDS_SERVER_RENDER_OPENGL
class VideoWidget : public QGLWidget {
#else
class VideoWidget : public QWidget {
#endif

    Q_OBJECT // must include this if you use Qt signals/slots

public:
    VideoWidget(QWidget *parent = NULL);
    void    stopRender();

    QString           mStatusText;
    ImageInfo      *mImgInfo;
    bool                mUpdateFlag;

protected:
    void paintEvent(QPaintEvent* event);

#ifndef IDS_SERVER_RENDER_SDL
private:
    QTimer *            mTimer;
private slots:
    void renderOneFrame();
#endif

#ifdef IDS_SERVER_RENDER_OPENGL
protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
private:
    void imshow(InputArray _img);
    cv::ogl::Texture2D  mOglTex;
    cv::ogl::Buffer     mOglBuf;
#endif
};

#endif // VIDEOWIDGET_H
