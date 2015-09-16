#include "idsclient.h"
#include "ui_idsclient.h"
#include "../common/displaycfgdialog.h"
#include <QDebug>
#include <QMessageBox>

#define SERVER_PORT 1702

void ids_io_fin_cb(gpointer priv)
{
    ((idsclient*)priv)->mIdsEndpoint = ids_create_remote_endpoint(((idsclient*)priv)->mIp, SERVER_PORT, ids_io_fin_cb, priv, NULL);
    qDebug() << "sdflksjdfsssssssssssssss";
    if (NULL == ((idsclient*)priv)->mIdsEndpoint)
    {
        QMessageBox::information(NULL, "tips", QString().sprintf("connect to server(%s) failed.", ((idsclient*)priv)->mIp), QMessageBox::Yes, NULL);
    }
    //QMessageBox::information(NULL, "tips", QString().sprintf("reconnect to server(%s) successed.", ((idsclient*)priv)->mIp), QMessageBox::Yes, NULL);
}

idsclient::idsclient(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::idsclient)
{
    ui->setupUi(this);

    if (TRUE != ids_core_init())
        throw QString("init core library failed");
    if (TRUE != ids_modules_init())
        throw QString("init modules library failed");
    mIdsEndpoint = NULL;
}

idsclient::~idsclient()
{
    delete ui;
}

void idsclient::on_pushButton_connect_clicked()
{
    strcpy(mIp, ui->lineEdit_ip->text().toLatin1());
    qDebug() <<"server ip:" << mIp;

    if (mIdsEndpoint != NULL)
        ids_destroy_remote_endpoint(mIdsEndpoint);
    mIdsEndpoint = ids_create_remote_endpoint(mIp, SERVER_PORT, ids_io_fin_cb, this, NULL);
    if (NULL == mIdsEndpoint)
    {
        QMessageBox::information(NULL, "tips", QString().sprintf("connect to server(%s) failed.", mIp), QMessageBox::Yes, NULL);
    }
    else
        QMessageBox::information(NULL, "tips", QString().sprintf("connect to server(%s) succeed.", mIp), QMessageBox::Yes, NULL);
}

void idsclient::on_pushButton_netcfg_clicked()
{
    NetCfgDialog netCfg;

    if (mIdsEndpoint == NULL)
    {
        QMessageBox::information(NULL, "tips", "please connect first.", QMessageBox::Yes, NULL);
        return ;
    }

    netCfg.update(mIdsEndpoint);
    netCfg.exec();
    strcpy(mIp, netCfg.mNetInfo.ip);
}

void idsclient::on_pushButton_dispcfg_clicked()
{
    if (mIdsEndpoint == NULL)
    {
        QMessageBox::information(NULL, "tips", "please connect first.", QMessageBox::Yes, NULL);
        return ;
    }
    displayCfgDialog dispCfg;
    dispCfg.mIdsServerWin = this;
    dispCfg.getInfo(mIdsEndpoint);
    dispCfg.show();
    dispCfg.exec();
}
