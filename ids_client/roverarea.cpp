#include "roverarea.h"
#include<QPainter>
#include<QMessageBox>
#include<qhostaddress.h>
#include<QDebug>
#include<QtMath>

RoverArea::RoverArea(QWidget *parent) : QWidget(parent)
{
    setPalette(QPalette(Qt::gray));
    setAutoFillBackground(true);

    pen.setWidth(5);
    pen.setBrush(QBrush(QColor(Qt::yellow)));

//    //读取图像
//    srcImage = cv::imread("d:/4.jpg");

    //读取视频
    defaultSettings = new QSettings("default.ini", QSettings::IniFormat);

    int msec = defaultSettings->value("view/Display_msec",0).toString().toInt();

    connect(&theTimer, &QTimer::timeout, this, updateImage);
    if(videoCap.open(defaultSettings->value("view/Camera_IP").toString().toStdString()))
    //if(videoCap.open("rtsp://system:system@192.168.1.5/bs0"))
     {
         srcImage = cv::Mat::zeros(videoCap.get(CV_CAP_PROP_FRAME_HEIGHT), videoCap.get (CV_CAP_PROP_FRAME_WIDTH), CV_8UC3);
         theTimer.start(msec);//theTimer.start(0);//40
     }

    xxx = 0;
    yyy = 0;

//    //网络初始化
//    port = 5555;
//    udpSocket = new QUdpSocket(this);

//    //初始化相机IP对应关系
//    hashCamera.insert("01","192.168.1.11");
//    hashCamera.insert("02","192.168.1.12");
//    hashCamera.insert("03","192.168.1.9");
//    hashCamera.insert("04","192.168.1.8");
//    hashCamera.insert("05","192.168.1.17");
//    hashCamera.insert("06","192.168.1.16");
//    hashCamera.insert("07","192.168.1.15");
//    hashCamera.insert("08","192.168.1.50");
//    hashCamera.insert("09","192.168.1.56");
//    hashCamera.insert("10","192.168.1.36");
//    hashCamera.insert("11","192.168.1.18");
//    hashCamera.insert("12","192.168.1.19");
//    hashCamera.insert("13","192.168.1.21");
//    hashCamera.insert("14","192.168.1.22");
//    hashCamera.insert("15","192.168.1.27");
//    hashCamera.insert("16","192.168.1.28");
//    hashCamera.insert("17","192.168.1.32");
//    hashCamera.insert("18","192.168.1.33");
//    hashCamera.insert("19","192.168.1.14");
//    hashCamera.insert("20","192.168.1.35");
//    hashCamera.insert("21","192.168.1.34");
//    hashCamera.insert("22","192.168.1.38");
//    hashCamera.insert("23","192.168.1.40");
//    hashCamera.insert("24","192.168.1.42");
//    hashCamera.insert("25","192.168.1.44");
//    hashCamera.insert("26","192.168.1.45");
//    hashCamera.insert("27","192.168.1.46");
//    hashCamera.insert("28","192.168.1.48");
//    hashCamera.insert("29","192.168.1.53");
//    hashCamera.insert("30","192.168.1.47");
//    hashCamera.insert("31","192.168.1.49");
//    hashCamera.insert("32","192.168.1.52");
//    hashCamera.insert("33","192.168.1.54");
//    hashCamera.insert("34","192.168.1.55");
//    hashCamera.insert("35","192.168.1.57");
//    hashCamera.insert("36","192.168.1.59");
//    hashCamera.insert("37","192.168.1.60");
//    hashCamera.insert("38","192.168.1.31");
}

RoverArea::~RoverArea()
{
    delete defaultSettings;
}

void RoverArea::updateImage()
{
    videoCap>>srcImage;
    if(srcImage.data)
    {
        cvtColor(srcImage, srcImage, CV_BGR2RGB);//Qt中支持的是RGB图像, OpenCV中支持的是BGR
        this->update();  //发送刷新消息
    }
}

void RoverArea::resizeEvent(QResizeEvent *event)
{
    widthArea = this->parentWidget()->width();
    heightArea = this->parentWidget()->height();
    setMinimumSize(widthArea,heightArea);
}

void RoverArea::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    image1 = QImage((uchar*) (srcImage.data), srcImage.cols, srcImage.rows, QImage::Format_RGB888);//::Format_RGB888
    p.drawImage(QRect(0,0,width(),height()),image1);

    //绘制辅助线
    p.setPen(pen);
    QRect rect(xxx*width()/3,yyy*height()/2,width()/3,height()/2);
    p.drawRect(rect);
    p.drawLine(xMouseDbClickPos-10,yMouseDbClickPos,xMouseDbClickPos+10,yMouseDbClickPos);
    p.drawLine(xMouseDbClickPos,yMouseDbClickPos-10,xMouseDbClickPos,yMouseDbClickPos+10);

}

//void RoverArea::mouseDoubleClickEvent(QMouseEvent *event)
//{
//    xMouseDbClickPos=event->pos().x();
//    yMouseDbClickPos=event->pos().y();

//    int wDivision = (int)(widthArea/10);
//    int hDivision = (int)(heightArea/4);

//    QString IP = hashCamera[QString::number(qFloor(yMouseDbClickPos/hDivision)) + QString::number(qCeil((xMouseDbClickPos-wDivision/2)/wDivision))];

//    //QString IP = hashCamera[QString::number((int)(yMouseDbClickPos/hDivision))+QString::number((int)((xMouseDbClickPos-wDivision/2)/wDivision))+1];
//    //qDebug()<<"Hash Key = "<<QString::number((int)(yMouseDbClickPos/hDivision))+QString::number((int)((xMouseDbClickPos-wDivision/2)/wDivision));

//    //发送消息显示端
//    QString msg = IP + " " + QString::number(xMouseDbClickPos) + " " + QString::number(yMouseDbClickPos);
//    int length = udpSocket->writeDatagram(msg.toLatin1(),msg.length(),QHostAddress::Broadcast,port);

//    //qDebug()<<"IP="<<IP<<"msg ="<<msg;
//}
