#ifndef IDSCLIENT_H
#define IDSCLIENT_H

#include <QMainWindow>
#include "ids.h"
#include "../common/displaycfgdialog.h"
#include "../common/netcfgdialog.h"

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

private slots:
    void on_pushButton_connect_clicked();
    void on_pushButton_netcfg_clicked();
    void on_pushButton_dispcfg_clicked();

private:
    Ui::idsclient *ui;
};

#endif // IDSCLIENT_H
