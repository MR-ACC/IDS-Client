#ifndef GLWIDGET_H
#define GLWIDGET_H

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
#include "ids.h"

void glwidget_render_frame_cb(gpointer priv, gpointer buf, gint buf_flag);
class GLWidget : public QGLWidget {

    Q_OBJECT // must include this if you use Qt signals/slots

public:
    GLWidget(QGLWidget *parent = NULL);

    void                *mImgBuf;
    int                 mImgBufFlag;
    bool                mUpdateFlag;

public slots:
    void renderOneFrame();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    void imshow(InputArray _img);

    QTimer *            mTimer;
    cv::ogl::Texture2D  mOglTex;
    cv::ogl::Buffer     mOglBuf;
};

#endif // GLWIDGET_H
