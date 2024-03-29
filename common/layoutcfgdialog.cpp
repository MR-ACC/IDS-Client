#include "layoutcfgdialog.h"
#include "ui_layoutcfgdialog.h"
#include <QDesktopWidget>
#include <QPushButton>

static void layout_get_cb(gpointer buf, gint buf_size, gpointer priv)
{
    if (sizeof(IdsLayoutAll) != buf_size)
    {
        Q_ASSERT (buf_size == sizeof(IdsResponse));
        ((layoutCfgDialog *)priv)->mMsgRet = ((IdsResponse*)buf)->ret;
    }
    else
    {
        IdsLayoutAll *layout_all = (IdsLayoutAll *)buf;
        ((layoutCfgDialog *)priv)->mlayout = *layout_all;
        ((layoutCfgDialog *)priv)->mMsgRet = MSG_EXECUTE_OK;
    }
}

static void layout_set_cb(gpointer buf, gint buf_size, gpointer priv)
{
    Q_ASSERT(buf_size == sizeof(IdsResponse));
    IdsResponse *pres = (IdsResponse *)buf;
    Q_ASSERT(pres->magic == IDS_RESPONSE_MAGIC);
    ((layoutCfgDialog *)priv)->mMsgRet = pres->ret;
}

layoutCfgDialog::layoutCfgDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::layoutCfgDialog)
{
    //QApplication::setFont(QFont("Times New Roman",14));
    ui->setupUi(this);
    QDesktopWidget* desktop = QApplication::desktop();
    move((desktop->width() - this->width())/2, (desktop->height() - this->height())/2);
    this->ui->buttonBox->button(QDialogButtonBox::Ok)->setText("确定");
    this->ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("取消");

    isDrawing = false;
    isLayoutSwitch = false;
    isNewLayout = false;
    isClick = true;
    isChangeWinChannel = false;
    mWinCount = 0;
    mCurSelectedWin = -1;
    for(int i = 0; i < IDS_LAYOUT_WIN_H; i++)
        for(int j = 0; j < IDS_LAYOUT_WIN_W; j++)
        {
            mlayoutMatrix[i][j] = 0;
        }
    mMargin = this->height() - this->width()/IDS_LAYOUT_WIN_W*IDS_LAYOUT_WIN_H;
    pix = QPixmap(this->width(),this->height());
    pix.fill(Qt::white);

    this->ui->btnDoneNew->setEnabled(false);
    this->ui->btnCancelNew->setEnabled(false);
    this->ui->labelChannel->setEnabled(false);
    this->ui->comboBoxCameraType->setEnabled(false);
    this->ui->comboBoxChannel->setEnabled(false);

    connect(this, SIGNAL(sig(int)), this, SLOT(msgslot(int)));

    mBrushFull = new QBrush(QColor(80,80,80),Qt::SolidPattern);
    mBrushEmpty = new QBrush(Qt::gray,Qt::SolidPattern);
    mPenNotSelected = new QPen(QColor(150,150,150),5);
    mPenSelected = new QPen(Qt::white,5);
    mPenGrid =  new QPen(QColor(40,40,40),1);
    mPenGridCenter =  new QPen(QColor(60,60,60),2);

    mVidList[0] = "拼接摄像机";
    mVidList[1] = "联动摄像机";
    for(int i = 0; i < IPC_CFG_STITCH_CNT; i++)
    {
        mVidList[i + 2] = "高点摄像机" + QString::number(i + 1);
    }
    for(int i = 0; i < IPC_CFG_NORMAL_CNT; i++)
    {
        mVidList[i + IPC_CFG_STITCH_CNT + 2] = "低点摄像机" + QString::number(i + 1);
    }

    on_comboBoxCameraType_activated(this->ui->comboBoxCameraType->currentIndex());
}

layoutCfgDialog::~layoutCfgDialog()
{
    delete ui;
}

int layoutCfgDialog::calcTextSize(int w, int h)
{
   // qDebug()<<"w"<<w;
    if(w < 50)
        return 3;
    else if(w < 80)
        return 7;
    else if(w < 100)
        return 12;
    else if(w < 200)
        return 18;
    else if(w < 300)
        return 22;
    else
        return 24;
}

void layoutCfgDialog::paintEvent(QPaintEvent *event)
{

    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    int endX, endY;

    x = MAX(MIN(lastPoint.x(), endPoint.x()), 0);
    y = MAX(MIN(lastPoint.y(), endPoint.y()), 0);

    endX = MAX(lastPoint.x(), endPoint.x());
    if(endX > this->width())
        w = this->width() - x;
    else
        w = endX - x;

    endY = MAX(lastPoint.y(), endPoint.y());
    if(lastPoint.y() > this->height() - mMargin && endPoint.y() > this->height() - mMargin)
    {
        h = 0;
        w = 0;
    }
    else if(endY > this->height() - mMargin)
        h = this->height() - mMargin - y;
    else
        h = endY - y;

    QPainter painter(this);
    if(isDrawing)     //如果正在绘图
    {
        tempPix = pix;    //将以前pix中的内容复制到tempPix中，这样实现了交互绘图
        QPainter pp(&tempPix);
        pp.drawRect(x,y,w,h);
        painter.drawPixmap(0,0,tempPix);
    }//不能把这个if语句删掉，只写else语句，即把else改为if（！isDrawing），这样会造成正在绘图时画布消失，只有在绘制完成后画布才重新出现
    else
    {
        QPainter pp(&pix);

        pp.setBrush(*mBrushFull); //设置画刷形式
        int minWidth = this->width()/IDS_LAYOUT_WIN_W;
        /*if(mWinCount >= IDS_LAYOUT_WIN_MAX_NUM)
        {
            qDebug()<<"sig";
            emit sig(0);
        }
        else */if(w != 0 && h != 0 &&  (y <= this->height() - mMargin) && mWinCount < IDS_LAYOUT_WIN_MAX_NUM && !isLayoutSwitch && !isNewLayout && !isChangeWinChannel && isClick)
        {
            //qDebug()<<x<<y<<w<<h<<minWidth;
            int drawX = (x/minWidth);
            int drawY = (y/minWidth);
            int drawW = ((x+w-1)/minWidth + 1) - drawX;
            int drawH = ((y+h-1)/minWidth + 1) - drawY;
            pp.setPen(*mPenNotSelected); //设置画笔形式
            //qDebug()<<drawX<<drawY<<drawW<<drawH<<minWidth;
            int coverFlag = 0;
            for(int i = drawY; i < (drawY+drawH); i++)
            {
                for(int j = drawX; j < (drawX+drawW); j++)
                {
                    if(mlayoutMatrix[i][j] == 1)
                    {
                        qDebug()<<"cover";
                        coverFlag = 1;
                        break;
                    }
                }
                if(coverFlag)
                    break;
            }
            if(coverFlag == 0)
            {
                for(int i = drawY; i < (drawY+drawH); i++)
                {
                    for(int j = drawX; j < (drawX+drawW); j++)
                    {

                            mlayoutMatrix[i][j] = 1;
                    }

                }
                mlayout.layout[mlayout.num].win[mWinCount].x = drawX;
                mlayout.layout[mlayout.num].win[mWinCount].y = drawY;
                mlayout.layout[mlayout.num].win[mWinCount].w = drawW;
                mlayout.layout[mlayout.num].win[mWinCount].h = drawH;
                if(this->ui->comboBoxCameraType->currentIndex() == 0)
                    mlayout.layout[mlayout.num].win[mWinCount].vid = 0;
                else if(this->ui->comboBoxCameraType->currentIndex() == 1)
                    mlayout.layout[mlayout.num].win[mWinCount].vid = 2 + this->ui->comboBoxChannel->currentIndex();
                else
                    mlayout.layout[mlayout.num].win[mWinCount].vid = 2 + IPC_CFG_STITCH_CNT + this->ui->comboBoxChannel->currentIndex();

                QRect rect(drawX*minWidth + 2,drawY*minWidth + 2,drawW*minWidth - 4,drawH*minWidth - 4);
                pp.setPen(*mPenSelected); //设置画笔形式
                pp.drawRect(rect);
                pp.setFont(QFont("Times New Roman",calcTextSize(drawW*minWidth - 4, drawH*minWidth - 4),QFont::Bold));
                pp.drawText(rect, Qt::AlignCenter, mVidList[mlayout.layout[mlayout.num].win[mWinCount].vid]);

                mCurSelectedWin = mWinCount;
                //qDebug()<<"mlayout.num: "<<mlayout.num<<"  mWinCount: "<<mWinCount<<"  vid: "<<mlayout.layout[mlayout.num].win[mWinCount].vid;

                for(int i = 0; i < mWinCount; i++)
                {
                    drawX = mlayout.layout[mlayout.num].win[i].x;
                    drawY = mlayout.layout[mlayout.num].win[i].y;
                    drawW = mlayout.layout[mlayout.num].win[i].w;
                    drawH = mlayout.layout[mlayout.num].win[i].h;

                    QRect rect(drawX*minWidth + 1,drawY*minWidth + 1,drawW*minWidth - 2,drawH*minWidth - 2);
                    pp.setPen(*mPenNotSelected); //设置画笔形式
                    pp.drawRect(rect);
                    pp.setFont(QFont("Times New Roman",calcTextSize(drawW*minWidth - 2, drawH*minWidth - 2),QFont::Bold));
                    pp.drawText(rect, Qt::AlignCenter, mVidList[mlayout.layout[mlayout.num].win[i].vid]);
                }
                mWinCount++;
            }
            isClick = false;
        }
        else if(isLayoutSwitch)
        {
            qDebug()<<"isLayoutSwitch"<<mCurSelectedLayout;
            for(int i = 0; i < IDS_LAYOUT_WIN_MAX_NUM && mlayout.layout[mCurSelectedLayout].win[i].w != 0; i++)
            {
                int drawX = mlayout.layout[mCurSelectedLayout].win[i].x;
                int drawY = mlayout.layout[mCurSelectedLayout].win[i].y;
                int drawW = mlayout.layout[mCurSelectedLayout].win[i].w;
                int drawH = mlayout.layout[mCurSelectedLayout].win[i].h;

                QRect rect(drawX*minWidth + 1,drawY*minWidth + 1,drawW*minWidth - 2,drawH*minWidth - 2);
                pp.setPen(*mPenNotSelected); //设置画笔形式
                pp.drawRect(rect);
                pp.setFont(QFont("Times New Roman",calcTextSize(drawW*minWidth - 2, drawH*minWidth - 2),QFont::Bold));
                pp.drawText(rect, Qt::AlignCenter, mVidList[mlayout.layout[mCurSelectedLayout].win[i].vid]);
            }
            isLayoutSwitch = false;
        }
        else if(isNewLayout)
        {
            qDebug()<<"isNewLayout";
            QRect rect(0,0,this->width(),this->height() - mMargin);
            pp.setBrush(*mBrushEmpty); //设置画刷形式
            pp.setPen(*mPenSelected); //设置画笔形式
            pp.drawRect(rect);
            isNewLayout = false;
        }
        else if(w == 0 && h == 0 && (x != 0 || y != 0) && (y <= this->height() - mMargin) && isClick)
        {
            qDebug()<<"click";
            for(int i = 0; i < IDS_LAYOUT_WIN_MAX_NUM && mlayout.layout[mCurSelectedLayout].win[i].w != 0; i++)
            {
                int drawX = mlayout.layout[mCurSelectedLayout].win[i].x;
                int drawY = mlayout.layout[mCurSelectedLayout].win[i].y;
                int drawW = mlayout.layout[mCurSelectedLayout].win[i].w;
                int drawH = mlayout.layout[mCurSelectedLayout].win[i].h;
                //qDebug()<<endPoint.x()<<endPoint.y()<<drawX<<drawY<<drawW<<drawH;
                if(endPoint.x()*1.0/minWidth > drawX && \
                        endPoint.y()*1.0/minWidth > drawY &&\
                        endPoint.x()*1.0/minWidth < drawX + drawW && \
                        endPoint.y()*1.0/minWidth < drawY + drawH)
                {
                    //qDebug()<<"in";
                    QRect rect(drawX*minWidth + 2,drawY*minWidth + 2,drawW*minWidth - 4,drawH*minWidth - 4);
                    pp.setPen(*mPenSelected); //设置画笔形式
                    pp.drawRect(rect);
                    pp.setFont(QFont("Times New Roman",calcTextSize(drawW*minWidth - 4, drawH*minWidth - 4),QFont::Bold));
                    pp.drawText(rect, Qt::AlignCenter, mVidList[mlayout.layout[mCurSelectedLayout].win[i].vid]);

                    mCurSelectedWin = i;
                }
                else
                {
                    //qDebug()<<"out";
                    QRect rect(drawX*minWidth + 1,drawY*minWidth + 1,drawW*minWidth - 2,drawH*minWidth - 2);
                    pp.setPen(*mPenNotSelected); //设置画笔形式
                    pp.drawRect(rect);
                    pp.setFont(QFont("Times New Roman",calcTextSize(drawW*minWidth - 2, drawH*minWidth - 2),QFont::Bold));
                    pp.drawText(rect, Qt::AlignCenter, mVidList[mlayout.layout[mCurSelectedLayout].win[i].vid]);

                }
            }
            isClick = false;
        }
        else if(isChangeWinChannel)
        {
            qDebug()<<"isChangeWinChannel: "<<mCurSelectedWin;
            for(int i = 0; i < IDS_LAYOUT_WIN_MAX_NUM && mlayout.layout[mCurSelectedLayout].win[i].w != 0; i++)
            {

                int drawX = mlayout.layout[mCurSelectedLayout].win[i].x;
                int drawY = mlayout.layout[mCurSelectedLayout].win[i].y;
                int drawW = mlayout.layout[mCurSelectedLayout].win[i].w;
                int drawH = mlayout.layout[mCurSelectedLayout].win[i].h;
                //qDebug()<<endPoint.x()<<endPoint.y()<<drawX<<drawY<<drawW<<drawH;
                if(i == mCurSelectedWin)
                {
                    //qDebug()<<"in";
                    QRect rect(drawX*minWidth + 2,drawY*minWidth + 2,drawW*minWidth - 4,drawH*minWidth - 4);
                    pp.setPen(*mPenSelected); //设置画笔形式
                    pp.drawRect(rect);
                    pp.setFont(QFont("Times New Roman",calcTextSize(drawW*minWidth - 4, drawH*minWidth - 4),QFont::Bold));
                    pp.drawText(rect, Qt::AlignCenter, mVidList[mlayout.layout[mCurSelectedLayout].win[i].vid]);
                }
                else
                {
                    //qDebug()<<"out";
                    QRect rect(drawX*minWidth + 1,drawY*minWidth + 1,drawW*minWidth - 2,drawH*minWidth - 2);
                    pp.setPen(*mPenNotSelected); //设置画笔形式
                    pp.drawRect(rect);
                    pp.setFont(QFont("Times New Roman",calcTextSize(drawW*minWidth - 2, drawH*minWidth - 2),QFont::Bold));
                    pp.drawText(rect, Qt::AlignCenter, mVidList[mlayout.layout[mCurSelectedLayout].win[i].vid]);
                }
            }
            isChangeWinChannel = false;
        }
        drawGrid(&pp);
        painter.drawPixmap(0,0,pix);
    }
}
void layoutCfgDialog::drawGrid(QPainter *painter)
{

    QRect rect;
    rect=QRect(0,0,width(),height()-mMargin);
    for(int i=0;i<=IDS_LAYOUT_WIN_W;i++)
    {
        int x=rect.left()+(i*(rect.width()-1)/IDS_LAYOUT_WIN_W);
        if(i%6 == 0)
            painter->setPen(*mPenGridCenter); //设置画笔形式
        else
            painter->setPen(*mPenGrid); //设置画笔形式
        painter->drawLine(x,rect.top(),x,rect.bottom());
    }
    for(int j=0;j<=IDS_LAYOUT_WIN_H;j++)
    {
        int y=rect.bottom()-(j*(rect.height()-1)/IDS_LAYOUT_WIN_H);
        if(j%4 == 0)
            painter->setPen(*mPenGridCenter); //设置画笔形式
        else
            painter->setPen(*mPenGrid); //设置画笔形式
        painter->drawLine(rect.left(),y,rect.right(),y);
    }
}


void layoutCfgDialog::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons()&Qt::LeftButton) //鼠标左键按下的同时移动鼠标
    {
        endPoint = event->pos();
        update();
    }
}
void layoutCfgDialog::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton) //鼠标左键按下
    {
        lastPoint = event->pos();
        isDrawing = true;   //正在绘图
        isClick = true;
    }
}

void layoutCfgDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) //鼠标左键释放
    {
        endPoint = event->pos();
        isDrawing = false;    //结束绘图
        //if(mWinCount >= IDS_LAYOUT_WIN_MAX_NUM)
        //
        update();
    }
}

void layoutCfgDialog::on_btnNewLayout_clicked()
{
    if(mlayout.num == IDS_LAYOUT_MAX_NUM)
    {
        QMessageBox::information(this, tr("提示"), tr("布局数达到最大"));
        return;
    }
    if(this->ui->lineEditLayoutName->text() == "")
    {
        QMessageBox::information(this, tr("提示"), tr("布局名不能为空"));
        return;
    }
    for(int i = 0; i < IDS_LAYOUT_WIN_MAX_NUM; i++)
        mlayout.layout[mlayout.num].win[i].w = 0;

    memset(mlayout.layout[mlayout.num].name, '\0', sizeof(mlayout.layout[mlayout.num].name));

    std::string str = this->ui->lineEditLayoutName->text().toStdString();
    const char* inbuf = str.c_str();
    base64_encode((guchar *)inbuf, strlen(inbuf), (guchar *)mlayout.layout[mlayout.num].name, sizeof(mlayout.layout[mlayout.num].name));

    qDebug()<<mlayout.layout[mlayout.num].name;
    //strcpy(mlayout.layout[mlayout.num].name, this->ui->lineEditLayoutName->text().toLatin1().data());
    this->ui->comboBoxLayoutList->addItem(this->ui->lineEditLayoutName->text());
    this->ui->comboBoxLayoutList->setCurrentText(this->ui->lineEditLayoutName->text());

    isNewLayout = true;
    this->ui->btnNewLayout->setEnabled(false);
    this->ui->btnDoneNew->setEnabled(true);
    this->ui->btnCancelNew->setEnabled(true);
    this->ui->comboBoxLayoutList->setEnabled(false);
    this->ui->btnDel->setEnabled(false);
    this->ui->btnModify->setEnabled(false);
    this->ui->lineEditLayoutName->setEnabled(false);
    this->ui->buttonBox->setEnabled(false);
    this->ui->labelChannel->setEnabled(true);
    this->ui->comboBoxCameraType->setEnabled(true);
    this->ui->comboBoxChannel->setEnabled(true);

    for(int i = 0; i < IDS_LAYOUT_WIN_H; i++)
        for(int j = 0; j < IDS_LAYOUT_WIN_W; j++)
        {
            mlayoutMatrix[i][j] = 0;
        }
    isDrawing = false;
    isLayoutSwitch = false;
    isClick = false;
    mWinCount = 0;
    mCurSelectedWin = -1;

    update();
}

void layoutCfgDialog::on_btnDoneNew_clicked()
{
    for(int i = 0; i < IDS_LAYOUT_WIN_H; i++)
        for(int j = 0; j < IDS_LAYOUT_WIN_W; j++)
        {
            if(mlayoutMatrix[i][j] == 0)
            {
                QMessageBox::information(this, tr("提示"), tr("窗口未覆盖所有的区域"));
                return;
            }
        }

    int stitchNum = 0;
    int linkNum = 0;
    for(int i = 0; i < mWinCount; i++)
    {
        if(mlayout.layout[mlayout.num].win[i].vid == 0)
            stitchNum++;
        if(mlayout.layout[mlayout.num].win[i].vid == 1)
            linkNum++;
    }
    if(stitchNum > 1)
    {
        QMessageBox::information(this, tr("提示"), tr("拼接窗口不能多于1"));
        return;
    }
    else if(linkNum > 1)
    {
        QMessageBox::information(this, tr("提示"), tr("联动窗口不能多于1"));
        return;
    }
    //qDebug()<<"on_btnDoneNew_clicked   mlayout.num: "<<mlayout.num<<"mWinCount"<<mWinCount;

    for(int i = mWinCount; i < IDS_LAYOUT_WIN_MAX_NUM; i++)
        mlayout.layout[mlayout.num].win[i].w = 0;

    this->ui->btnNewLayout->setEnabled(true);
    this->ui->comboBoxLayoutList->setEnabled(true);
    this->ui->btnDel->setEnabled(true);
    this->ui->btnModify->setEnabled(true);
    this->ui->btnDoneNew->setEnabled(false);
    this->ui->btnCancelNew->setEnabled(false);
    this->ui->lineEditLayoutName->setEnabled(true);
    this->ui->buttonBox->setEnabled(true);
    this->ui->labelChannel->setEnabled(false);
    this->ui->comboBoxCameraType->setEnabled(false);
    this->ui->comboBoxChannel->setEnabled(false);
    //mCurSelectedWin = -1;
    mlayout.num++;
}

void layoutCfgDialog::on_btnCancelNew_clicked()
{
    this->ui->btnNewLayout->setEnabled(true);
    this->ui->comboBoxLayoutList->setEnabled(true);
    this->ui->btnDel->setEnabled(true);
    this->ui->btnModify->setEnabled(true);
    this->ui->btnDoneNew->setEnabled(false);
    this->ui->btnCancelNew->setEnabled(false);
    this->ui->lineEditLayoutName->setEnabled(true);
    this->ui->buttonBox->setEnabled(true);
    this->ui->labelChannel->setEnabled(false);
    this->ui->comboBoxCameraType->setEnabled(false);
    this->ui->comboBoxChannel->setEnabled(false);

    for(int i = 0; i < IDS_LAYOUT_WIN_MAX_NUM; i++)
        mlayout.layout[mlayout.num].win[i].w = 0;

    this->ui->comboBoxLayoutList->removeItem(this->ui->comboBoxLayoutList->currentIndex());
    //mCurSelectedWin = -1;
    update();
}

void layoutCfgDialog::on_comboBoxLayoutList_currentIndexChanged(int index)
{
    //qDebug()<<"on_comboBoxLayoutList_currentIndexChanged"<<index;
    if(index < 0)
    {
        mCurSelectedLayout = 0;
        mCurSelectedWin = -1;
        isNewLayout = true;
        this->ui->btnModify->setEnabled(false);
        this->ui->btnDel->setEnabled(false);
        this->ui->comboBoxLayoutList->setEnabled(false);
    }
    else
    {
        mCurSelectedLayout = index;
        //qDebug()<<"mCurSelectedLayout: "<<mCurSelectedLayout;
        isLayoutSwitch = true;
        mCurSelectedWin = -1;
        this->ui->btnModify->setEnabled(true);
        this->ui->btnDel->setEnabled(true);
        this->ui->comboBoxLayoutList->setEnabled(true);
    }
    for(int i = 0; i < IDS_LAYOUT_WIN_H; i++)
        for(int j = 0; j < IDS_LAYOUT_WIN_W; j++)
        {
            mlayoutMatrix[i][j] = 1;
        }
    update();
}

void layoutCfgDialog::on_comboBoxCameraType_activated(int index)
{
    this->ui->comboBoxChannel->clear();

    if(index == 0)
        this->ui->comboBoxChannel->addItem(mVidList[0]);
    else if(index == 1)
    {
        for(int i = 0; i < IPC_CFG_STITCH_CNT; i++)
        {
            this->ui->comboBoxChannel->addItem(mVidList[i + 2]);
        }
    }
    else if(index == 2)
    {
        for(int i = 0; i < IPC_CFG_NORMAL_CNT; i++)
        {
            this->ui->comboBoxChannel->addItem(mVidList[i + IPC_CFG_STITCH_CNT + 2]);
        }
    }
    on_comboBoxChannel_activated(this->ui->comboBoxChannel->currentIndex());
}

void layoutCfgDialog::on_btnDel_clicked()
{
    QMessageBox::StandardButton rb = QMessageBox::question(NULL, tr("提示"), tr("确认删除此布局吗？"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if(rb == QMessageBox::Yes && mlayout.num > 0)
    {
        mlayout.num--;
        if(this->ui->comboBoxLayoutList->currentIndex() == mlayout.id && this->ui->comboBoxLayoutList->count() != 1)
            mlayout.id = 0;
        else if(this->ui->comboBoxLayoutList->currentIndex() < mlayout.id)
            mlayout.id -= 1;
        else if(this->ui->comboBoxLayoutList->count() == 1)
            mlayout.id = -1;

        //qDebug()<<"mlayout.num"<<mlayout.num<<this->ui->comboBoxLayoutList->currentIndex();
        for(int i = this->ui->comboBoxLayoutList->currentIndex(); i < mlayout.num; i++)
        {
            memcpy(&(mlayout.layout[i]), &(mlayout.layout[i+1]), sizeof(IdsLayout));
        }
        for(int i = 0; i < IDS_LAYOUT_WIN_MAX_NUM; i++)
            mlayout.layout[mlayout.num].win[i].w = 0;
        this->ui->comboBoxLayoutList->removeItem(this->ui->comboBoxLayoutList->currentIndex());
        mCurSelectedWin = -1;
    }
}

void layoutCfgDialog::on_comboBoxChannel_activated(int index)
{
    //qDebug()<<"mCurSelectedWin: "<<mCurSelectedWin<<"  mCurSelectedLayout: "<<mCurSelectedLayout;
    if(mCurSelectedWin != -1)
    {
        if(this->ui->comboBoxCameraType->currentIndex() == 0)
            mlayout.layout[mCurSelectedLayout].win[mCurSelectedWin].vid = 0;
        else if(this->ui->comboBoxCameraType->currentIndex() == 1)
            mlayout.layout[mCurSelectedLayout].win[mCurSelectedWin].vid = 2 + index;
        else
            mlayout.layout[mCurSelectedLayout].win[mCurSelectedWin].vid = 2 + IPC_CFG_STITCH_CNT + index;

        isChangeWinChannel = true;
        update();
    }
}

void layoutCfgDialog::msgslot(int i)
{
    //qDebug()<<"msgslot"<<i;
    if(i == 0)
    {
        QMessageBox::information(this, tr("提示"), tr("单个布局最多允许8个播放窗口"));
    }
}

int layoutCfgDialog::idsUpdate(gpointer endpoint)
{
    mIdsEndpoint = endpoint;
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_GET_LAYOUT, -1,
                           NULL, 0, layout_get_cb, (void*)this, 3);
    if (mMsgRet != MSG_EXECUTE_OK)
        return 0;
    //qDebug()<<"mlayout.num"<<mlayout.num;
    if(mlayout.num == 0)
    {
        for(int i = 0; i < IDS_LAYOUT_WIN_H; i++)
            for(int j = 0; j < IDS_LAYOUT_WIN_W; j++)
            {
                mlayoutMatrix[i][j] = 1;
            }
    }
    for(int i = 0; i < mlayout.num; i++)
    {
        char base64DecodeBuf[128];
        memset(base64DecodeBuf, '\0', sizeof(base64DecodeBuf));
        base64_decode((guchar *)mlayout.layout[i].name, strlen(mlayout.layout[i].name), (guchar *)base64DecodeBuf, sizeof(base64DecodeBuf));
        QString name = QString::fromUtf8(base64DecodeBuf, strlen(base64DecodeBuf));
        //qDebug()<<"base64_decode"<<name<<mlayout.layout[i].name;
        this->ui->comboBoxLayoutList->addItem(name);
    }
    if(mlayout.id >= 0)
        this->ui->comboBoxLayoutList->setCurrentIndex(mlayout.id);
    return 1;
}

void layoutCfgDialog::on_buttonBox_accepted()
{
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_SET_LAYOUT, -1,
                           &mlayout, sizeof(IdsLayoutAll), layout_set_cb, (void*)this, 3);
    if (mMsgRet == MSG_EXECUTE_OK)
        this->close();
    else
    {
        QString text;
        text.sprintf("布局设置失败. 错误码: %d", mMsgRet);
        QMessageBox::information(this, tr("提示"), text);
    }
}

void layoutCfgDialog::on_buttonBox_rejected()
{
    this->close();
}

void layoutCfgDialog::on_btnModify_clicked(bool checked)
{
    if(!checked)
    {
        int stitchNum = 0;
        for(int i = 0; i < IDS_LAYOUT_WIN_MAX_NUM && mlayout.layout[mCurSelectedLayout].win[i].w != 0; i++)
        {
            if(mlayout.layout[mCurSelectedLayout].win[i].vid == 0)
                stitchNum++;
        }
        if(stitchNum > 1)
        {
            QMessageBox::information(this, tr("提示"), tr("拼接窗口不能多于1"));
            this->ui->btnModify->setChecked(true);
            return;
        }
    }
    this->ui->labelChannel->setEnabled(checked);
    this->ui->comboBoxCameraType->setEnabled(checked);
    this->ui->comboBoxChannel->setEnabled(checked);
    this->ui->buttonBox->setEnabled(!checked);
    this->ui->btnNewLayout->setEnabled(!checked);
    this->ui->comboBoxLayoutList->setEnabled(!checked);
    this->ui->btnDel->setEnabled(!checked);
    this->ui->lineEditLayoutName->setEnabled(!checked);
    if(checked)
        this->ui->btnModify->setText("完成");
    else
        this->ui->btnModify->setText("修改");

}

