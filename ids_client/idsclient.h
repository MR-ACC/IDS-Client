#ifndef IDSCLIENT_H
#define IDSCLIENT_H

#include <QMainWindow>
#include "ids.h"
#include "roverarea.h"
#include<QUdpSocket>
#include<QSettings>

namespace Ui {
class idsclient;
}

class idsclient : public QMainWindow
{
    Q_OBJECT

public:
    explicit idsclient(QWidget *parent = 0);
    ~idsclient();

    char mIp[32];
    gpointer mIdsEndpoint;
    gpointer mIdsEndpoint1;

    int                             mMsgRet;
    IdsServerInfo          mServerInfo;

private slots:
    void on_pushButton_connect_clicked();
    void on_pushButton_netcfg_clicked();
    void on_pushButton_dispcfg_clicked();
    void connect_server(int prompt_first); 

    void on_pushButton_chncfg_clicked();

    void on_pushButton_upgrade_clicked();

    void on_pushButton_layoutcfg_clicked();

    void on_pushButton_stitch_clicked();

    void setbtnEnable(bool enable);

    void on_pushButton_serverReboot_clicked();

    void on_pushButton_layoutSwitch_clicked();

    void on_pushButton_reConnect_clicked();

    void mouseDoubleClickEvent(QMouseEvent *event);

//    void on_openGLWidget_resized();

    void keyPressEvent(QKeyEvent *event);

    void on_pushButton_connect_2_clicked();

    void on_pushButton_connect_3_clicked();

    void on_pushButton_connect_4_clicked();

    void on_pushButton_connect_5_clicked();

signals:
    void connect_network(int);

private:
    Ui::idsclient *ui;

    //added by zhoushihua
    RoverArea *roverArea;

    //配置文件参数
    QSettings *defaultSettings;

    //网络通讯
    int port;
    QUdpSocket *udpSocket;

    int groupId,groupIdPre;
    int xxx,yyy;


};

#endif // IDSCLIENT_H
