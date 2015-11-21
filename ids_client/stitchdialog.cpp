#include "stitchdialog.h"
#include "ui_stitchdialog.h"

stitchDialog::stitchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::stitchDialog)
{
    ui->setupUi(this);
    this->ui->buttonBox->button(QDialogButtonBox::Ok)->setText("确定");
    this->ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("取消");

}

stitchDialog::~stitchDialog()
{
    delete ui;
}

static void stitch_set_cb(gpointer buf, gint buf_size, gpointer priv)
{
    Q_ASSERT(buf_size == sizeof(IdsResponse));
    IdsResponse *pres = (IdsResponse *)buf;
    Q_ASSERT(pres->magic == IDS_RESPONSE_MAGIC);
    ((stitchDialog *)priv)->mMsgRet = pres->ret;
}
void stitchDialog::on_buttonBox_accepted()
{
    QFile *file = new QFile(this->ui->lineEditFile->text());
    if(file->open(QIODevice::ReadOnly))
    {

        int len = file->size();
        qDebug()<<"len"<<len;
        IdsFileUpdate *ifu = (IdsFileUpdate *)malloc(sizeof(IdsFileUpdate)+len);
        ifu->len = len;
        strcpy(ifu->path, "stitch.cfg");
        file->read(ifu->buf, len);
        file->close();
        ids_net_write_msg_sync(mIdsEndpoint
                , IDS_CMD_FILE_UPDATE
                , -1
                , ifu
                , sizeof(IdsFileUpdate)+len
                , stitch_set_cb
                , (void*)this
                , 3);
        free(ifu);
        if (mMsgRet == MSG_EXECUTE_OK)
            this->close();
        else
        {
            QString text;
            text.sprintf("拼接配置设置失败. 错误码: %d", mMsgRet);
            QMessageBox::information(this, "提示", text, QMessageBox::Yes, NULL);
        }
        return;
    }
}

void stitchDialog::on_buttonBox_rejected()
{
    this->close();
}

int stitchDialog::idsUpdate(gpointer endpoint)
{
    mIdsEndpoint = endpoint;
    return 1;
}

void stitchDialog::on_pushButtonBrow_clicked()
{
    QString filename;
    if((filename = QFileDialog::getOpenFileName(this, tr("选择文件"), tr("."), tr("*.cfg"))) != NULL )
    {
        ui->lineEditFile->setText(filename);
    }
}
