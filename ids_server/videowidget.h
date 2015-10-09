#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#define HAVE_OPENCV_OPENGL
#define HAVE_OPENCV_CUDA

#define TIME_PER_FRAME 30

#include "ids.h"
#include "app_amp.h"
#include <QTimer>
#include <QtGui>
#include <QDebug>
#include <QApplication>
#include <QWidget>
#include <QPixmap>

#ifdef HAVE_OPENCV_OPENGL
    #include "opencv2/opencv_modules.hpp"
    #include <opencv2/core.hpp>
    #include <opencv2/core/opengl.hpp>
    #include <QtOpenGL/QGLWidget>
    #include "GL/gl.h"
    #include "GL/glu.h"
    using namespace cv;

    #ifdef HAVE_OPENCV_CUDA
        #include <opencv2/core/cuda.hpp>
        using namespace cuda;
    #endif
#endif

void video_widget_frame_cb(gpointer priv, ImageInfo *piinfo);

#ifdef HAVE_OPENCV_OPENGL
class VideoWidget : public QGLWidget {
#else
class VideoWidget : public QWidget {
#endif

    Q_OBJECT // must include this if you use Qt signals/slots
public:
    VideoWidget(QWidget *parent = NULL);
    ~VideoWidget();
    void startPlay(gchar *rtsp_url);
    void startPlay(gchar *rtsp_urls[], gint nums);
    void startPlayExperts(gchar *rtsp_urls[], gint nums, gint win_flags, gint player_flags, gint draw_fmt);
    void stopPlay();

    bool            mUpdateFlag;
    ImageInfo       mImgInfoClone;
    QMutex          mMutex;

protected:
    void paintEvent(QPaintEvent* event);
private:
    IdsPlayer*      mPlayer;
    QString         mStatusText;
    QTimer          mTimer;
private slots:
    void renderOneFrame();

#ifdef HAVE_OPENCV_OPENGL
protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
private:
    void imshow(InputArray _img);
    bool                m_OglTexValid;
    cv::ogl::Texture2D  mOglTex;
    cv::ogl::Buffer     mOglBuf;
#endif //HAVE_OPENCV
};

#endif // VIDEOWIDGET_H
