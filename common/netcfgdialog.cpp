#include "netcfgdialog.h"
#include "ui_netcfgdialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QPushButton>

NetCfgDialog::NetCfgDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetCfgDialog)
{
    ui->setupUi(this);
    QDesktopWidget* desktop = QApplication::desktop();
    move((desktop->width() - this->width())/2, (desktop->height() - this->height())/2);
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    this->ui->NetCfgbuttonBox->button(QDialogButtonBox::Ok)->setText("确定");
    this->ui->NetCfgbuttonBox->button(QDialogButtonBox::Cancel)->setText("取消");

    connect(ui->NetCfgbuttonBox, SIGNAL(accepted()), this, SLOT(netcfgAccept()));
    connect(ui->NetCfgbuttonBox, SIGNAL(rejected()), this, SLOT(netcfgReject()));
}

NetCfgDialog::~NetCfgDialog()
{
    delete ui;
}

void msg_callback_dummy(gpointer buf, gint buf_size, gpointer priv)
{
    if (buf_size == sizeof(IdsResponse)) {
        IdsResponse *pres = (IdsResponse *)buf;
        if (pres->magic == IDS_RESPONSE_MAGIC) {
            return ;
        }
    }
}

void NetCfgDialog::netcfgAccept()
{
    QString ip = ui->IPCfglineEdit->text();
    QString ma = ui->MaskCfglineEdit->text();
    QString ga = ui->GateCfglineEdit->text();

    if (mNetInfo.flag & IFACE_OK) {
        mNetInfo.flag = IFACE_OK;
        if (ip != "") {
            strcpy(mNetInfo.ip, (const char*) ip.toLocal8Bit());
            mNetInfo.flag |= IP_OK;
        }

        if (ma != "") {
            strcpy(mNetInfo.mask, (const char*) ma.toLocal8Bit());
            mNetInfo.flag |= MASK_OK;
        }
qDebug() << mNetInfo.gw;

        if (ga != "" ) {
            strcpy(mNetInfo.gw, (const char*) ga.toLocal8Bit());
            mNetInfo.flag |= GW_OK;
        }

        if (mNetInfo.flag != IFACE_OK) {
            ids_net_write_msg_sync(mIdsEndpoint
                        , IDS_CMD_SET_NETWORK_INFO
                        , -1
                        , &mNetInfo
                        , sizeof(mNetInfo)
                        , msg_callback_dummy
                        , NULL
                        , 3);
            return ;
        }
    }
}

void NetCfgDialog::netcfgReject()
{

}

void get_netinfo_cb(gpointer buf, gint buf_size, gpointer priv)
{
    NetInfo *pninfo = (NetInfo *)buf;
    NetCfgDialog *pcfgDialog = (NetCfgDialog *)priv;

    if (buf_size == sizeof(IdsResponse)) {
        IdsResponse *pres = (IdsResponse *)buf;
        if (pres->magic == IDS_RESPONSE_MAGIC) {
            pcfgDialog->mNetInfo.flag = 0;
            return ;
        }
    }

    pcfgDialog->mNetInfo = *pninfo;
}

int NetCfgDialog::idsUpdate(gpointer  endpoint)
{
    ids_net_write_msg_sync(endpoint
                    , IDS_CMD_GET_NETWORK_INFO
                    , -1
                    , NULL
                    , 0
                    , get_netinfo_cb
                    , this
                    , 3);

    if (!mNetInfo.flag)
    {
        ui->IPCfglineEdit->setText("");
        ui->MaskCfglineEdit->setText("");
        ui->GateCfglineEdit->setText("");
        return 0;
    }
    else
    {
        if (mNetInfo.flag &IP_OK )
            ui->IPCfglineEdit->setText(mNetInfo.ip);

        if (mNetInfo.flag &MASK_OK)
            ui->MaskCfglineEdit->setText(mNetInfo.mask);

        if (mNetInfo.flag &GW_OK)
            ui->GateCfglineEdit->setText(mNetInfo.gw);

        mIdsEndpoint = endpoint;
        return 1;
    }
}
