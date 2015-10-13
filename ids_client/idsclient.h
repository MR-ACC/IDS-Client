#ifndef IDSCLIENT_H
#define IDSCLIENT_H

#include <QMainWindow>
#include "ids.h"

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

    void on_pushButton_serverShutdown_clicked();

    void on_pushButton_layoutSwitch_clicked();

signals:
    void connect_network(int);
private:
    Ui::idsclient *ui;
};

#endif // IDSCLIENT_H
