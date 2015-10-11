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
    FILE *fp = fopen(this->ui->lineEditFile->text().toLatin1().data(), "rb");
    if(fp)
    {
       fseek(fp, 0L, SEEK_END);
       int len = ftell(fp);
       fseek(fp, 0L, SEEK_SET);
       //qDebug()<<"len"<<len;
       IdsFileUpdate *ifu = (IdsFileUpdate *)malloc(sizeof(IdsFileUpdate)+len);
       ifu->len = len;
       strcpy(ifu->path, "stitch.cfg");
       fread(ifu->buf, 1, len, fp);
       fclose(fp);
       //qDebug()<<"ifu->buf"<<ifu->buf;

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
           text.sprintf("摄像机通道设置失败. 错误码: %d", mMsgRet);
           QMessageBox::information(this, "提示", text, QMessageBox::Yes, NULL);
       }
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
