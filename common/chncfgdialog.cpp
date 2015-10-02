#include "chncfgdialog.h"
#include "ui_chncfgdialog.h"
#include <QMessageBox>

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

    this->ui->tableWidget->setRowCount(IPC_CFG_MAX_CNT);
    this->ui->tableWidget->setColumnWidth(0, 400);
    int i;
    for (i=0; i<IPC_CFG_STITCH_CNT; i++)
        this->ui->tableWidget->setVerticalHeaderItem(i,new QTableWidgetItem(QString().sprintf("高点摄像机%d",i+1)));
    for (i=0; i<IPC_CFG_NORMAL_CNT; i++)
        this->ui->tableWidget->setVerticalHeaderItem(IPC_CFG_STITCH_CNT+i,new QTableWidgetItem(QString().sprintf("低点摄像机%d",i+1)));
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
    for (i=0; i<IPC_CFG_STITCH_CNT; i++)
    {
        char *url = this->ui->tableWidget->item(i,0)->text().toLatin1().data();
        if (url[0] == 0 || url[0] == ' ')
            strcpy(this->mIpcCfgAll.ipc_sti[i].url, "");
        else
            strcpy(this->mIpcCfgAll.ipc_sti[i].url, url);
    }
    for (i=0; i<IPC_CFG_NORMAL_CNT; i++)
    {
        char *url = this->ui->tableWidget->item(i+IPC_CFG_STITCH_CNT,0)->text().toLatin1().data();
        if (url[0] == 0 || url[0] == ' ')
            strcpy(this->mIpcCfgAll.ipc[i].url, "");
        else
            strcpy(this->mIpcCfgAll.ipc[i].url, url);
    }
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_SET_IPC_CFG, -1,
                           &mIpcCfgAll, sizeof(mIpcCfgAll), ipc_cfg_set_cb, (void*)this, 1);
}
