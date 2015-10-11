#include "idsclient.h"
#include "ui_idsclient.h"
#include "../common/displaycfgdialog.h"
#include "../common/chncfgdialog.h"
#include "../common/netcfgdialog.h"
#include "../common/layoutcfgdialog.h"
#include "upgradedialog.h"
#include "stitchdialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QDesktopWidget>
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
QApplication::setFont(QFont("Times New Roman",14));
    if (TRUE != ids_core_init())
        throw QString("init core library failed");

    if (TRUE != ids_modules_init())
        throw QString("init modules library failed");

    mIdsEndpoint = NULL;

    connect(this, SIGNAL(connect_network(int)), this, SLOT(connect_server(int)));

    QDesktopWidget* desktop = QApplication::desktop();
    move((desktop->width() - this->width())/2, (desktop->height() - this->height())/2);

    this->ui->pushButton_chncfg->setEnabled(false);
    this->ui->pushButton_dispcfg->setEnabled(false);
    this->ui->pushButton_layoutcfg->setEnabled(false);
    this->ui->pushButton_netcfg->setEnabled(false);
    this->ui->pushButton_stitch->setEnabled(false);
    //this->ui->pushButton_upgrade->setEnabled(false);
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
        QMessageBox::StandardButton rb = QMessageBox::question(NULL, tr("提示")
                           , "网络断开，是否重新连接？"
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
        QMessageBox::critical(NULL, tr("错误"), QString().sprintf("连接服务器(%s)失败.", mIp));
    else
    {
        this->ui->pushButton_chncfg->setEnabled(true);
        this->ui->pushButton_dispcfg->setEnabled(true);
        this->ui->pushButton_layoutcfg->setEnabled(true);
        this->ui->pushButton_netcfg->setEnabled(true);
        this->ui->pushButton_stitch->setEnabled(true);
        this->ui->pushButton_upgrade->setEnabled(true);
        QMessageBox::information(NULL, tr("提示"), QString().sprintf("连接服务器(%s)成功.", mIp));
    }
}

void idsclient::on_pushButton_connect_clicked()
{
    strcpy(mIp, ui->lineEdit_ip->text().toLatin1());
    qDebug() <<"server ip:" << mIp;

    connect_server(0);
}


void idsclient::on_pushButton_chncfg_clicked()
{
    if (mIdsEndpoint == NULL)
    {
        QMessageBox::information(NULL, tr("提示"), tr("请先连接服务器！"));
        return ;
    }
    int ret;
    ChnCfgDialog chnCfg;
    ret = chnCfg.idsUpdate(mIdsEndpoint);
    if (!ret)
    {
        QMessageBox::critical(NULL, tr("错误"), "获取通道信息失败!");
        return ;
    }
    chnCfg.exec();
}

void idsclient::on_pushButton_dispcfg_clicked()
{
    if (mIdsEndpoint == NULL)
    {
        QMessageBox::information(NULL, tr("提示"), tr("请先连接服务器！"));
        return ;
    }

    int ret;
    displayCfgDialog dispCfg;


    ret = dispCfg.idsUpdate(mIdsEndpoint);
    if (!ret)
    {
        QMessageBox::critical(NULL, tr("错误"), "获取显示模式失败！");
        return ;
    }
    dispCfg.exec();
}

void idsclient::on_pushButton_netcfg_clicked()
{
    if (mIdsEndpoint == NULL)
    {
        QMessageBox::information(NULL, tr("提示"), tr("请先连接服务器！"));
        return ;
    }
    int ret;
    NetCfgDialog netCfg;
    ret = netCfg.idsUpdate(mIdsEndpoint);
    if (!ret)
    {
        QMessageBox::critical(NULL, tr("错误"), "获取网络信息失败！");
        return ;
    }

    netCfg.exec();
}

void idsclient::on_pushButton_layoutcfg_clicked()
{
    if (mIdsEndpoint == NULL)
    {
        QMessageBox::information(NULL, tr("提示"), tr("请先连接服务器！"));
        return ;
    }
    int ret;
    layoutCfgDialog layoutDialog;
    ret = layoutDialog.idsUpdate(mIdsEndpoint);
    if (!ret)
    {
        QMessageBox::critical(NULL, tr("错误"), "获取布局信息失败！");
        return ;
    }

    layoutDialog.exec();
}

void idsclient::on_pushButton_upgrade_clicked()
{
    Dialog upgradeDialog(this, ui->lineEdit_ip->text());
    upgradeDialog.exec();
}


void idsclient::on_pushButton_stitch_clicked()
{
    if (mIdsEndpoint == NULL)
    {
        QMessageBox::information(NULL, tr("提示"), tr("请先连接服务器！"));
        return ;
    }
    int ret;
    stitchDialog stitchdlg;
    ret = stitchdlg.idsUpdate(mIdsEndpoint);
    if (!ret)
    {
        QMessageBox::critical(NULL, tr("错误"), "失败！");
        return ;
    }
    stitchdlg.exec();
}
