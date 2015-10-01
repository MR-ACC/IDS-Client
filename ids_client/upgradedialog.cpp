#include "upgradedialog.h"
#include "ui_dialog.h"
#include "upgradethread.h"

Dialog::Dialog(QWidget *parent, QString ip) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    ui->setupUi(this);
    #if 1
    QRegExp rx("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    QRegExpValidator v(rx,0);
    ui->lineEditTag->setValidator(&v);
    ui->lineEditTag->setInputMask("000.000.000.000");
	#endif
    this->ui->lineEditTag->setText(ip);
    timer=new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(Update_Slot()));
    num=150;
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButtonBrow_clicked()
{
    QString filter; 
    filter = ".fw";
    QString filename;
    if((filename = QFileDialog::getOpenFileName(this, tr("选择文件"), tr("."), tr("固件 (*.*)"))) != NULL )
    {
        ui->lineEditFile->setText(filename);
    }
}

void Dialog::on_pushButtonSend_clicked()
{
    if(ui->lineEditTag->text().isEmpty() || ui->lineEditFile->text().isEmpty())
    {
        ui->labelStatus->setText(tr(" IP地址、固件名不能为空！"));
        return;
    }

    RWThread* rw_thread[MAX_CAMERA_NUM];
    int i = 0;
    int upgradetype = 0;
    ui->upgrade->setEnabled(false);

    upgradetype = 0;
    num = 5;


    ui->labelStatus->setText(tr(" 连接中，请稍等..."));
    i = 0;
    status[i] = 0;
    rw_thread[i] = new RWThread();
    connect(rw_thread[i],SIGNAL(timesig(int)),this,SLOT(time_slot(int)));
    connect(rw_thread[i], SIGNAL(sig(int)), this, SLOT(send_slot(int)));
    connect(rw_thread[i], SIGNAL(message(QString)), this, SLOT(updateStatusLabel(QString)));
    rw_thread[i]->sendFile(i, ui->lineEditFile->text(), ui->lineEditTag->text(), "md5", upgradetype);
    i++;

    ipnum = i;
    interactiveFinish = false;
    interactiveFinishNum = 0;
    timer->start(1000);
}


void Dialog::updateStatusLabel(const QString &status)
{
    QString buf = status;
    ui->labelStatus->setText(buf.remove("\r\n"));
}

void Dialog::Update_Slot()
{
    if(interactiveFinish == false)
    {
        interactiveFinishNum = 0;
        int i;
        for(i = 0; i < ipnum; i++)
        {
            if(status[i] == 0)
                break;
            if(status[i] == 1)
                interactiveFinishNum++;
        }
        if(i == ipnum)
        {
            interactiveFinish = true;
            if(interactiveFinishNum == 0)
            {
                timer->stop();
                ui->upgrade->setEnabled(true);
                return;
            }
        }
    }
    if(interactiveFinish == true)
    {
        time_status=QString(" 服务器正在升级,请勿断电......剩余约%1秒").arg(num);
        ui->labelStatus->setText(time_status);
        if(num==0)
        {
            timer->stop();
            ui->labelStatus->setText(tr(" 升级成功！"));
            num=150;
            ui->upgrade->setEnabled(true);
        }
        num--;
    }
}

void Dialog::time_slot(int i)
{
    status[i] = 1;
}


void Dialog::send_slot(int i)
{
    status[i] = -1;
}
