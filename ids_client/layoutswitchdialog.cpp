#include "layoutswitchdialog.h"
#include "ui_layoutswitchdialog.h"
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>
layoutSwitchDialog::layoutSwitchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::layoutSwitchDialog)
{
    ui->setupUi(this);
    this->ui->buttonBox->button(QDialogButtonBox::Ok)->setText("确定");
    this->ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("取消");
}

layoutSwitchDialog::~layoutSwitchDialog()
{
    delete ui;
}

static void layout_get_cb(gpointer buf, gint buf_size, gpointer priv)
{
    if (sizeof(IdsLayoutAll) != buf_size)
    {
        Q_ASSERT (buf_size == sizeof(IdsResponse));
        ((layoutSwitchDialog *)priv)->mMsgRet = ((IdsResponse*)buf)->ret;
    }
    else
    {
        IdsLayoutAll *layout_all = (IdsLayoutAll *)buf;
        ((layoutSwitchDialog *)priv)->mlayout = *layout_all;
        ((layoutSwitchDialog *)priv)->mMsgRet = MSG_EXECUTE_OK;
    }
}

static void layout_set_cb(gpointer buf, gint buf_size, gpointer priv)
{
    Q_ASSERT(buf_size == sizeof(IdsResponse));
    IdsResponse *pres = (IdsResponse *)buf;
    Q_ASSERT(pres->magic == IDS_RESPONSE_MAGIC);
    ((layoutSwitchDialog *)priv)->mMsgRet = pres->ret;
}

int layoutSwitchDialog::idsUpdate(gpointer endpoint)
{
    mIdsEndpoint = endpoint;
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_GET_LAYOUT, -1,
                           NULL, 0, layout_get_cb, (void*)this, 3);
    if (mMsgRet != MSG_EXECUTE_OK)
        return 0;

    for(int i = 0; i < mlayout.num; i++)
        this->ui->comboBox->addItem(QString(mlayout.layout[i].name));
    if(mlayout.id >= 0)
        this->ui->comboBox->setCurrentIndex(mlayout.id);
    return 1;
}

void layoutSwitchDialog::on_buttonBox_accepted()
{
    qDebug()<<ui->comboBox->count() ;
    if(ui->comboBox->count() == 0)
        mlayout.id = -1;
    else
        mlayout.id = ui->comboBox->currentIndex();
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_SET_LAYOUT, -1,
                           &mlayout, sizeof(IdsLayoutAll), layout_set_cb, (void*)this, 3);
    if (mMsgRet == MSG_EXECUTE_OK)
        this->close();
    else
    {
        QString text;
        text.sprintf("布局设置失败. 错误码: %d", mMsgRet);
        QMessageBox::information(this, "提示", text, QMessageBox::Yes, NULL);
    }
}

void layoutSwitchDialog::on_buttonBox_rejected()
{
    this->close();
}
