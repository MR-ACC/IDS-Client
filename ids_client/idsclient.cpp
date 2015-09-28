#include "idsclient.h"
#include "ui_idsclient.h"
#include "../common/displaycfgdialog.h"
#include "../common/chncfgdialog.h"
#include "../common/netcfgdialog.h"
#include "upgradedialog.h"
#include <QDebug>
#include <QMessageBox>

#define SERVER_PORT 1702

void ids_io_fin_cb(gpointer priv)
{
    qDebug() << "io fin cb";
    if (((idsclient*)priv)->mIdsEndpoint) {
        ((idsclient*)priv)->mIdsEndpoint = NULL;  //lock mIdsEndpoint before use it???
        emit ((idsclient*)priv)->connect_network(1);
    }
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

    connect(this, SIGNAL(connect_network(int)), this, SLOT(connect_server(int)));
}

idsclient::~idsclient()
{
    //id_modules_uninit();
    //ids_core_uninit();
    //delete ui;
}

void idsclient::connect_server(int prompt_first)
{
    if (prompt_first)
    {
        QMessageBox::StandardButton rb = QMessageBox::question(NULL, "Tips"
                           , "Currently the network is disconnected, connect it now?"
                           , QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(rb != QMessageBox::Yes)
            return ;
    }

    if (mIdsEndpoint != NULL) {
        gpointer io =  mIdsEndpoint;
        mIdsEndpoint = NULL;
        ids_destroy_remote_endpoint(io);
    }

    mIdsEndpoint = ids_create_remote_endpoint(mIp, SERVER_PORT, ids_io_fin_cb, this, NULL);
    if (NULL == mIdsEndpoint)
        QMessageBox::critical(NULL, "Error", QString().sprintf("connect to server(%s) failed.", mIp));
    else
        QMessageBox::information(NULL, "tips", QString().sprintf("connect to server(%s) succeed.", mIp));
}

void idsclient::on_pushButton_connect_clicked()
{
    strcpy(mIp, ui->lineEdit_ip->text().toLatin1());
    qDebug() <<"server ip:" << mIp;

    connect_server(0);
}


void idsclient::on_pushButton_chncfg_clicked()
{
    ChnCfgDialog chnCfg;

    if (mIdsEndpoint == NULL)
    {
        QMessageBox::information(NULL, "tips", "please connect first.");
        return ;
    }

    chnCfg.setGeometry(200, 200, 640, 480);
    if (chnCfg.update(mIdsEndpoint))
        chnCfg.exec();
    else
        qDebug() << QString().sprintf("ids get ipc cfg error. code = %d.", chnCfg.mMsgRet);
}

void idsclient::on_pushButton_dispcfg_clicked()
{
    displayCfgDialog dispCfg;

    if (mIdsEndpoint == NULL)
    {
        QMessageBox::information(NULL, "tips", "please connect first.");
        return ;
    }

    dispCfg.update(mIdsEndpoint);
    dispCfg.exec();
}

void idsclient::on_pushButton_netcfg_clicked()
{
    int ret;
    /*NetCfgDialog netCfg;

    if (mIdsEndpoint == NULL)
    {
        QMessageBox::information(NULL, "tips", "please connect first.");
        return ;
    }
    ret = netCfg.update(mIdsEndpoint);
    if (!ret)
    {
        QMessageBox::critical(NULL, "Error", "get network info failed!");
        return ;
    }

    netCfg.exec();*/
    Dialog upgrade(this, ui->lineEdit_ip->text());
    upgrade.exec();

}
