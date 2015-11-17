#include "idsclient.h"
#include "ui_idsclient.h"
#include "../common/displaycfgdialog.h"
#include "../common/chncfgdialog.h"
#include "../common/netcfgdialog.h"
#include "../common/layoutcfgdialog.h"
#include "layoutswitchdialog.h"
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

static void server_info_get_cb(gpointer buf, gint buf_size, gpointer priv)
{
    if (sizeof(IdsServerInfo) != buf_size)
    {
        Q_ASSERT (buf_size == sizeof(IdsResponse));
        ((idsclient *)priv)->mMsgRet = ((IdsResponse*)buf)->ret;
    }
    else
    {
        IdsServerInfo *ids_server_info= (IdsServerInfo *)buf;
        ((idsclient *)priv)->mServerInfo = *ids_server_info;
        ((idsclient *)priv)->mMsgRet = MSG_EXECUTE_OK;
    }
}

static void ids_set_cb(gpointer buf, gint buf_size, gpointer priv)
{
    Q_ASSERT(buf_size == sizeof(IdsResponse));
    IdsResponse *pres = (IdsResponse *)buf;
    Q_ASSERT(pres->magic == IDS_RESPONSE_MAGIC);
    ((idsclient *)priv)->mMsgRet = pres->ret;
}

void idsclient::setbtnEnable(bool enable)
{
    this->ui->pushButton_chncfg->setEnabled(enable);
    this->ui->pushButton_dispcfg->setEnabled(enable);
    this->ui->pushButton_layoutcfg->setEnabled(enable);
    this->ui->pushButton_netcfg->setEnabled(enable);
    this->ui->pushButton_stitch->setEnabled(enable);
    this->ui->pushButton_layoutSwitch->setEnabled(enable);
    this->ui->pushButton_reConnect->setEnabled(enable);
    this->ui->pushButton_serverReboot->setEnabled(enable);
    //this->ui->pushButton_upgrade->setEnabled(enable);
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

    setbtnEnable(false);
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
        {
            setbtnEnable(false);
            return ;
        }
    }

    if (mIdsEndpoint != NULL) {
        gpointer io =  mIdsEndpoint;
        mIdsEndpoint = NULL;
        ids_destroy_remote_endpoint(io);
    }

    mIdsEndpoint = ids_create_remote_endpoint(mIp, SERVER_PORT, ids_io_fin_cb, this, NULL);
    if (NULL == mIdsEndpoint)
    {
        setbtnEnable(false);
        QMessageBox::critical(NULL, tr("错误"), QString().sprintf("连接服务器(%s)失败.", mIp));
    }
    else
    {
        setbtnEnable(true);
        ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_GET_SERVER_INFO, -1,
                               NULL, 0, server_info_get_cb, (void*)this, 3);
        if (mMsgRet != MSG_EXECUTE_OK)
        {
             ui->label_serverInfo->setText("获取服务器系统信息失败!");
            qDebug() << QString().sprintf("ids cmd set server info error. code = %d.", mMsgRet);
        }
        else
        {
            QString sys_ver = QString(tr("系统:  ")) + QString(mServerInfo.sys_ver);
            QString soft_ver = QString(tr("软件:  ")) + QString(mServerInfo.soft_ver);
//            QMessageBox::about(this, tr("关于"), tr("视频拼接服务器\n") + sys_ver + soft_ver);
            ui->label_serverInfo->setText(sys_ver + soft_ver);
        }
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

void idsclient::on_pushButton_serverReboot_clicked()
{
    qDebug()<<"on_pushButton_serverReboot_clicked";
    QMessageBox *msgBox = new QMessageBox(QMessageBox::Warning
                                              , tr("服务器维护")
                                              , tr("请选择是否需要重启或关闭服务器?")
                                              , QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel
                                              , NULL);
    msgBox->setButtonText(QMessageBox::Save, tr("重启"));
    msgBox->setButtonText(QMessageBox::Discard, tr("关机"));
    msgBox->setButtonText(QMessageBox::Cancel, tr("取消"));
    int ret = msgBox->exec();
    if (ret == QMessageBox::Save)
    {
        ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_SERVER_REBOOT, -1,
                               NULL, 0, ids_set_cb, (void*)this, 3);
        if (mMsgRet != MSG_EXECUTE_OK)
            qDebug() << QString().sprintf("ids cmd reboot error. code = %d.", mMsgRet);
    }
    else if(ret == QMessageBox::Discard)
    {
        ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_SERVER_SHUTDOWN, -1,
                               NULL, 0, ids_set_cb, (void*)this, 3);
        if (mMsgRet != MSG_EXECUTE_OK)
            qDebug() << QString().sprintf("ids cmd shut down error. code = %d.", mMsgRet);
    }

    delete msgBox;
}

void idsclient::on_pushButton_layoutSwitch_clicked()
{
    if (mIdsEndpoint == NULL)
    {
        QMessageBox::information(NULL, tr("提示"), tr("请先连接服务器！"));
        return ;
    }
    int ret;
    layoutSwitchDialog layoutSwitchDlg;
    ret = layoutSwitchDlg.idsUpdate(mIdsEndpoint);
    if (!ret)
    {
        QMessageBox::critical(NULL, tr("错误"), "获取布局信息失败！");
        return ;
    }

    layoutSwitchDlg.exec();
}

void idsclient::on_pushButton_reConnect_clicked()
{
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_AMP_REFRESH, -1,
                               NULL, 0, ids_set_cb, (void*)this, 3);
    if (mMsgRet != MSG_EXECUTE_OK)
        qDebug() << QString().sprintf("ids reconnect error. code = %d.", mMsgRet);
}
