#ifndef ROVERAREA_H
#define ROVERAREA_H

#include <QWidget>
#include<QPen>
//#include<QBrush>
#include<QMouseEvent>
#include<opencv.hpp>
#include<QUdpSocket>
#include<QHash>
#include<QTimer>
#include<QSettings>

class RoverArea : public QWidget
{
    Q_OBJECT
public:
    explicit RoverArea(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);
    ~RoverArea();

signals:

public slots:
//    void mouseDoubleClickEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);

    //获取图像
    void updateImage();

private:
    QPen pen;
    int widthArea,heightArea;

    //显示图像
    QTimer theTimer;
    QImage image1;
    cv::Mat srcImage;
    cv::VideoCapture videoCap;

//    //网络通讯
//    int port;
//    QUdpSocket *udpSocket;

public:
    int xMouseDbClickPos=0,yMouseDbClickPos=0;
    QHash<QString,QString> hashCamera;

    QSettings *defaultSettings;

    int xxx,yyy;
    int www,hhh;
};

#endif // ROVERAREA_H
