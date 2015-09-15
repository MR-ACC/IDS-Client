#include "netcfgdialog.h"
#include "ui_netcfgdialog.h"
#include <QDebug>
#include <QtNetwork/QNetworkInterface>

NetCfgDialog::NetCfgDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetCfgDialog)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint |Qt::WindowStaysOnTopHint);
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    connect(ui->NetCfgbuttonBox, SIGNAL(accepted()), this, SLOT(netcfgAccept()));
    connect(ui->NetCfgbuttonBox, SIGNAL(rejected()), this, SLOT(netcfgReject()));
}

NetCfgDialog::~NetCfgDialog()
{
    delete ui;
}

void NetCfgDialog::netcfgAccept()
{
    QString ip = ui->IPCfglineEdit->text();
    QString ma = ui->MaskCfglineEdit->text();
    QString ga = ui->GateCfglineEdit->text();
    QString cmd = "modify_net eth "+netname;
    qDebug() << "accept netname:" << netname;

    if (ip != "")
        cmd += " ip "+ip;

    if (ma != "")
        cmd += " netmask "+ma;

    if (ga != "")
        cmd += " gw "+ga;

    qDebug() << cmd;

    system(cmd.toLocal8Bit().constData());
}

void NetCfgDialog::netcfgReject()
{

}

void NetCfgDialog::update()
{
    QString ip, mask;
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    //获取所有网络接口的列表
    foreach(QNetworkInterface interface,list)
    {  //遍历每一个网络接口
        qDebug() << "Device: "<<interface.name();
        if (interface.name() == "lo")
            continue;
        //设备名
        qDebug() << "HardwareAddress: "<<interface.hardwareAddress();
        //硬件地址
        QList<QNetworkAddressEntry> entryList = interface.addressEntries();
     //获取IP地址条目列表，每个条目中包含一个IP地址，一个子网掩码和一个广播地址
        netname = interface.name();
        qDebug() << "netname:" << netname;
        foreach(QNetworkAddressEntry entry,entryList)
        {//遍历每一个IP地址条目
            qDebug()<<"IP Address: "<<entry.ip().toString();
            qDebug() << "get valid iface:" << interface.name();
            ip = entry.ip().toString();
            mask = entry.netmask().toString();
            break;
        }
    }

    qDebug() << "get ip:" << ip << "get mask:" << mask;

    ui->IPCfglineEdit->setText(ip);
    ui->MaskCfglineEdit->setText(mask);
}
