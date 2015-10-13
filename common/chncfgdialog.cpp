#include "chncfgdialog.h"
#include "ui_chncfgdialog.h"
#include <QMessageBox>
#include <QDesktopWidget>
#include <QPushButton>

static void ipc_cfg_get_cb(gpointer buf, gint buf_size, gpointer priv)
{
    if (sizeof(IpcCfgAll) != buf_size)
    {
        Q_ASSERT (buf_size == sizeof(IdsResponse));
        ((ChnCfgDialog *)priv)->mMsgRet = ((IdsResponse*)buf)->ret;
    }
    else
    {
        IpcCfgAll *ipc_cfg_all = (IpcCfgAll *)buf;
        ((ChnCfgDialog *)priv)->mIpcCfgAll = *ipc_cfg_all;
        ((ChnCfgDialog *)priv)->mMsgRet = MSG_EXECUTE_OK;
    }
}

static void ipc_cfg_set_cb(gpointer buf, gint buf_size, gpointer priv)
{
    Q_ASSERT(buf_size == sizeof(IdsResponse));
    IdsResponse *pres = (IdsResponse *)buf;
    Q_ASSERT(pres->magic == IDS_RESPONSE_MAGIC);
    ((ChnCfgDialog *)priv)->mMsgRet = pres->ret;
}

ChnCfgDialog::ChnCfgDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChnCfgDialog)
{
    ui->setupUi(this);

    this->ui->buttonBox->button(QDialogButtonBox::Ok)->setText("确定");
    this->ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("取消");

    this->ui->tableWidget->setRowCount(IPC_CFG_MAX_CNT);
    this->ui->tableWidget->setColumnWidth(0, 400);
    int i;
    for (i=0; i<IPC_CFG_STITCH_CNT; i++)
        this->ui->tableWidget->setVerticalHeaderItem(i,new QTableWidgetItem(QString().sprintf("高点摄像机%d",i+1)));
    for (i=0; i<IPC_CFG_NORMAL_CNT; i++)
        this->ui->tableWidget->setVerticalHeaderItem(IPC_CFG_STITCH_CNT+i,new QTableWidgetItem(QString().sprintf("低点摄像机%d",i+1)));

    QDesktopWidget* desktop = QApplication::desktop();
    move((desktop->width() - this->width())/2, (desktop->height() - this->height())/2);

}

ChnCfgDialog::~ChnCfgDialog()
{
    delete ui;
}

int ChnCfgDialog::idsUpdate(gpointer endpoint)
{
    mIdsEndpoint = endpoint;
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_GET_IPC_CFG, -1,
                           NULL, 0, ipc_cfg_get_cb, (void*)this, 3);
    if (mMsgRet != MSG_EXECUTE_OK)
        return 0;
    int i;
    for (i=0; i<IPC_CFG_STITCH_CNT; i++)
        this->ui->tableWidget->setItem(i, 0, new QTableWidgetItem(this->mIpcCfgAll.ipc_sti[i].url));
    for (i=0; i<IPC_CFG_NORMAL_CNT; i++)
        this->ui->tableWidget->setItem(IPC_CFG_STITCH_CNT+i, 0, new QTableWidgetItem(this->mIpcCfgAll.ipc[i].url));
    return 1;
}

void ChnCfgDialog::on_buttonBox_accepted()
{
    int i;
    for (i=0; i<IPC_CFG_STITCH_CNT+IPC_CFG_NORMAL_CNT; i++)
    {
        char *url = this->ui->tableWidget->item(i,0)->text().toLatin1().data();
        {
            if (i < IPC_CFG_STITCH_CNT)
                memcpy(mIpcCfgAll.ipc_sti[i].url, url, MIN(strlen(url), 255));
            else
                memcpy(mIpcCfgAll.ipc[i-IPC_CFG_STITCH_CNT].url, url, MIN(strlen(url), 255));
        }
    }
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_SET_IPC_CFG, -1,
                           &mIpcCfgAll, sizeof(mIpcCfgAll), ipc_cfg_set_cb, (void*)this, 1);
    if (mMsgRet == MSG_EXECUTE_OK)
        this->close();
    else
    {
        QString text;
        text.sprintf("摄像机通道设置失败. 错误码: %d", mMsgRet);
        QMessageBox::information(this, "提示", text, QMessageBox::Yes, NULL);
    }
}
