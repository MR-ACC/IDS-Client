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
#include<opencv.hpp>
#include<QDebug>

#define SERVER_PORT 1702

int groupMatrix[][3]=
{
    6,5,4,
    3,2,1
};

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
    this->ui->pushButton_upgrade->setEnabled(enable);
}

idsclient::idsclient(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::idsclient)
{
    groupIdPre = -1;
    groupId = 1;
    xxx=0;
    yyy=0;

    ui->setupUi(this);
    QApplication::setFont(QFont("Times New Roman",14));
    if (TRUE != ids_core_init())
        throw QString("init core library failed");

    if (TRUE != ids_modules_init())
        throw QString("init modules library failed");

    mIdsEndpoint = NULL;

    connect(this, SIGNAL(connect_network(int)), this, SLOT(connect_server(int)));

    //added by zhoushihua
    roverArea = new RoverArea(ui->openGLWidget);

    QDesktopWidget* desktop = QApplication::desktop();
    move((desktop->width() - this->width())/2, (desktop->height() - this->height())/2);

    //读取ini文件
    defaultSettings = new QSettings("default.ini", QSettings::IniFormat);

    //网络初始化
//    port = 9002;
    udpSocket = new QUdpSocket(this);


    setbtnEnable(false);
}

void idsclient::mouseDoubleClickEvent(QMouseEvent *event)
{
    //发送消息显示端
    QString msg = QString::number(ui->openGLWidget->width())+","+QString::number(ui->openGLWidget->height())+","+
            QString::number(event->globalX())+"," +QString::number(event->globalY())+",";//liyou在结尾加了一个逗号
    int length =
            udpSocket->writeDatagram(msg.toLatin1(),
                                     msg.length(),
                                     QHostAddress(defaultSettings->value("view/Display_UDP_IP","192.168.1.102").toString()),
                                     defaultSettings->value("view/Display_UDP_Port",9002).toInt());
    //return;

    int www = ui->openGLWidget->width()/3;
    int hhh = ui->openGLWidget->height()/2;
    xxx = event->globalX() / www;
    xxx = MAX(xxx,0);
    xxx = MIN(xxx,2);
    yyy = event->globalY() / hhh;
    yyy = MAX(yyy,0);
    yyy = MIN(yyy,1);

    roverArea->xxx = xxx;
    roverArea->yyy = yyy;
    //roverArea->www = www;
    //roverArea->hhh = hhh;
    roverArea->xMouseDbClickPos = event->globalX();
    roverArea->yMouseDbClickPos = event->globalY();


    groupId = groupMatrix[yyy][xxx];
    if(groupId == groupIdPre)//如果当前选择的组与上一组相同，则返回
    {
        return;
    }
    groupIdPre = groupId;

    char Ips[4][32];
    strcpy(Ips[0],"192.168.1.66");
    strcpy(Ips[1],"192.168.1.68");
    strcpy(Ips[2],"192.168.1.67");
    strcpy(Ips[3],"192.168.1.70");

    for(int i=0;i<4;i++)
    {

        //改通道配置
        strcpy(mIp,Ips[i]);
        //emit(connect_network(0));     //连接服务器
        connect_server(0);//连接服务器
        if (mIdsEndpoint == NULL)
        {
            QMessageBox::information(NULL, tr("提示"), tr("请先连接服务器！"));
            return ;
        }
        int ret;
        ChnCfgDialog chnCfg;
        ret = chnCfg.idsUpdate(mIdsEndpoint);//这里会将mIpcCfgAll.ipc_sti[i].url和mIpcCfgAll.ipc[i].url载入到对话框中！！！
        if (!ret)
        {
            QMessageBox::critical(NULL, tr("错误"), "获取通道信息失败!");
            return ;
        }
        chnCfg.setChannels(groupId,i+1);
        //改拼接配置
        if (mIdsEndpoint == NULL)
        {
            QMessageBox::information(NULL, tr("提示"), tr("请先连接服务器！"));
            return ;
        }
        stitchDialog stitchdlg;
        ret = stitchdlg.idsUpdate(mIdsEndpoint);
        if (!ret)
        {
            QMessageBox::critical(NULL, tr("错误"), "失败！");
            return ;
        }
        QString fileNme = defaultSettings->value(QString().sprintf("group%d/Server%d_cfgFile",groupId,i+1)).toString();
        stitchdlg.sendCfgFile(fileNme);
    }

}

void idsclient::keyPressEvent(QKeyEvent *event)//键盘响应
{
    QWidget::keyPressEvent(event);
    return;

    //QMessageBox::aboutQt(NULL,"ddddd");
    bool bValid = false;
    if((QApplication::keyboardModifiers()==Qt::ControlModifier) && (event->key()==Qt::Key_Left))
    {
        QMessageBox::aboutQt(NULL,"ddddd");
        xxx--;
        if(xxx<0)
        {
            xxx=0;
            return;
        }
        bValid = true;
        groupId = groupMatrix[yyy][xxx];
    }
    else if(event->key()==Qt::Key_Right)
    {
        xxx++;
        if(xxx>=3)
        {
            xxx=2;
            return;
        }
        bValid = true;
        groupId = groupMatrix[yyy][xxx];
    }
    else if(event->key()==Qt::Key_Up)
    {
        yyy--;
        if(yyy<0)
        {
            yyy=0;
            return;
        }
        bValid = true;
        groupId = groupMatrix[yyy][xxx];
    }
    else if(event->key()==Qt::Key_Down)
    {
        yyy++;
        if(yyy>=2)
        {
            yyy=1;
            return;
        }
        bValid = true;
        groupId = groupMatrix[yyy][xxx];
    }

    if(!bValid)
    {
        QWidget::keyPressEvent(event);
        return;
    }

    //更换通道ipc.cfg和配置文件sticth.cfg
    //下面照抄mouseDoubleClickEvent（）

    char Ips[4][32];
    strcpy(Ips[0],"192.168.1.66");
    strcpy(Ips[1],"192.168.1.68");
    strcpy(Ips[2],"192.168.1.67");
    strcpy(Ips[3],"192.168.1.70");

    for(int i=0;i<4;i++)
    {

        //改通道配置
        strcpy(mIp,Ips[i]);
        //emit(connect_network(0));     //连接服务器
        connect_server(0);//连接服务器
        if (mIdsEndpoint == NULL)
        {
            QMessageBox::information(NULL, tr("提示"), tr("请先连接服务器！"));
            return ;
        }
        int ret;
        ChnCfgDialog chnCfg;
        ret = chnCfg.idsUpdate(mIdsEndpoint);//这里会将mIpcCfgAll.ipc_sti[i].url和mIpcCfgAll.ipc[i].url载入到对话框中！！！
        if (!ret)
        {
            QMessageBox::critical(NULL, tr("错误"), "获取通道信息失败!");
            return ;
        }
        chnCfg.setChannels(groupId,i+1);
        //改拼接配置
        if (mIdsEndpoint == NULL)
        {
            QMessageBox::information(NULL, tr("提示"), tr("请先连接服务器！"));
            return ;
        }
        stitchDialog stitchdlg;
        ret = stitchdlg.idsUpdate(mIdsEndpoint);
        if (!ret)
        {
            QMessageBox::critical(NULL, tr("错误"), "失败！");
            return ;
        }
        QString fileNme = defaultSettings->value(QString().sprintf("group%d/Server%d_cfgFile",groupId,i+1)).toString();
        stitchdlg.sendCfgFile(fileNme);
    }

    QWidget::keyPressEvent(event);
}

idsclient::~idsclient()
{
    //id_modules_uninit();
    //ids_core_uninit();
    //delete ui;

    delete defaultSettings;
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
        //QMessageBox::information(NULL, tr("提示"), QString().sprintf("连接服务器(%s)成功.", mIp));
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

/*
void idsclient::on_openGLWidget_resized()
{

}
*/
void idsclient::on_pushButton_connect_2_clicked()
{
    strcpy(mIp, "192.168.1.66");
    qDebug() <<"server ip:" << mIp;

    connect_server(0);

    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_AMP_REFRESH, -1,
                               NULL, 0, ids_set_cb, (void*)this, 3);
    if (mMsgRet != MSG_EXECUTE_OK)
        qDebug() << QString().sprintf("ids reconnect error. code = %d.", mMsgRet);
}

void idsclient::on_pushButton_connect_3_clicked()
{
    strcpy(mIp, "192.168.1.68");
    qDebug() <<"server ip:" << mIp;

    connect_server(0);

    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_AMP_REFRESH, -1,
                               NULL, 0, ids_set_cb, (void*)this, 3);
    if (mMsgRet != MSG_EXECUTE_OK)
        qDebug() << QString().sprintf("ids reconnect error. code = %d.", mMsgRet);
}

void idsclient::on_pushButton_connect_4_clicked()
{
    strcpy(mIp, "192.168.1.67");
    qDebug() <<"server ip:" << mIp;

    connect_server(0);

    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_AMP_REFRESH, -1,
                               NULL, 0, ids_set_cb, (void*)this, 3);
    if (mMsgRet != MSG_EXECUTE_OK)
        qDebug() << QString().sprintf("ids reconnect error. code = %d.", mMsgRet);
}

void idsclient::on_pushButton_connect_5_clicked()
{
    strcpy(mIp, "192.168.1.70");
    qDebug() <<"server ip:" << mIp;

    connect_server(0);

    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_AMP_REFRESH, -1,
                               NULL, 0, ids_set_cb, (void*)this, 3);
    if (mMsgRet != MSG_EXECUTE_OK)
        qDebug() << QString().sprintf("ids reconnect error. code = %d.", mMsgRet);
}
