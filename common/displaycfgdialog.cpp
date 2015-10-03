#include "displaycfgdialog.h"
#include "ui_displaycfgdialog.h"
#include <QtGui>
#include <QMessageBox>

#define MAX_MODE_NUMBERS 20
const int mode_v_cnt[MAX_MODE_NUMBERS] = {1, 1, 1, 1, 2, 2, 2, 0};
const int mode_h_cnt[MAX_MODE_NUMBERS] = {1, 2, 3, 4, 2, 3, 4, 0};

static void get_monitor_infos_cb(gpointer buf, gint buf_size, gpointer priv)
{
    if (sizeof(MonitorInfos) != buf_size)
    {
        Q_ASSERT (buf_size == sizeof(IdsResponse));
        ((displayCfgDialog *)priv)->mMsgRet = ((IdsResponse*)buf)->ret;
    }
    else
    {
        ((displayCfgDialog *)priv)->mMonitorInfos = *(MonitorInfos *)buf;
        ((displayCfgDialog *)priv)->mMsgRet = MSG_EXECUTE_OK;
    }
}

static void get_display_mode_cb(gpointer buf, gint buf_size, gpointer priv)
{
    if (sizeof(DisplayModeInfo) != buf_size)
    {
        Q_ASSERT (buf_size == sizeof(IdsResponse));
        ((displayCfgDialog *)priv)->mMsgRet = ((IdsResponse*)buf)->ret;
    }
    else
    {
        ((displayCfgDialog *)priv)->mDispMode = *(DisplayModeInfo *)buf;
        ((displayCfgDialog *)priv)->mMsgRet = MSG_EXECUTE_OK;
    }
}

static void set_preview_display_mode_cb(gpointer buf, gint buf_size, gpointer priv)
{
    Q_ASSERT(buf_size == sizeof(IdsResponse));
    IdsResponse *pres = (IdsResponse *)buf;
    Q_ASSERT(pres->magic == IDS_RESPONSE_MAGIC);
    ((displayCfgDialog *)priv)->mMsgRet = pres->ret;
}

displayCfgDialog::displayCfgDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::displayCfgDialog)
{
    ui->setupUi(this);
    connect(ui->listWidget_mode, SIGNAL(itemSelectionChanged()), this, SLOT(modeResChanged()));
    connect(ui->listWidget_res, SIGNAL(itemSelectionChanged()), this, SLOT(modeResChanged()));
}

int displayCfgDialog::idsUpdate(gpointer endpoint)
{
    //get now mDispMode
    mIdsEndpoint = endpoint;
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_GET_MONITOR_INFOS,
                           -1, NULL, 0, get_monitor_infos_cb, this, 3);
    if (mMsgRet != MSG_EXECUTE_OK)
        return 0;
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_GET_DISPLAY_MODE,
                           -1, NULL, 0, get_display_mode_cb, this, 3);
    if (mMsgRet != MSG_EXECUTE_OK)
        return 0;

    //set ids_server display mode preview mode.
    int start = 1;
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_PREVIEW_DISPLAY_MODE,
                           -1, &start, sizeof(start), set_preview_display_mode_cb, this, 3);
    if (mMsgRet != MSG_EXECUTE_OK)
        return 0;

    int i, j, k;

    for (i=0; i<MAX_MODE_NUMBERS; i++)
    {
        if (mode_v_cnt[i] <= 0 ||
            mode_v_cnt[i] * mode_h_cnt[i] > mMonitorInfos.monitor_nums)
            break;
    }
    mModeCnt = i;

    mResCnt = 0;
    for (i=0; i<mMonitorInfos.monitor_nums; i++)
    {
        for (j=0; j<MAX_MONITOR_RES_NUMBERS; j++)
        {
            if (mMonitorInfos.monitor_infos[i].monitor_res_infos[j].w <= 0 ||
                mMonitorInfos.monitor_infos[i].monitor_res_infos[j].h <= 0)
                break;
            for (k=0; k<mResCnt; k++)
            {
                if (mMonitorInfos.monitor_infos[i].monitor_res_infos[j].w == mResAll[k].w &&
                    mMonitorInfos.monitor_infos[i].monitor_res_infos[j].h == mResAll[k].h )
                        break;
            }
            if (k == mResCnt)
            {
                mResAll[k].w = mMonitorInfos.monitor_infos[i].monitor_res_infos[j].w;
                mResAll[k].h = mMonitorInfos.monitor_infos[i].monitor_res_infos[j].h;
                mResCnt++;
            }
        }
    }

    mModeId = -1;
    mResId = -1;

    ui->listWidget_mode->clear();
    for (i=0; i<mModeCnt; i++)
        ui->listWidget_mode->addItem(QString().sprintf("%d * %d", mode_v_cnt[i], mode_h_cnt[i]));

    ui->listWidget_res->clear();
    for (i=0; i<mResCnt; i++)
        ui->listWidget_res->addItem(QString().sprintf("%d * %d", mResAll[i].w, mResAll[i].h));

    ui->listWidget_monitor->clear();
    for (i=0; i<mMonitorInfos.monitor_nums; i++)
        ui->listWidget_monitor->addItem(QString(mMonitorInfos.monitor_infos[i].name));
    return 1;
}

displayCfgDialog::~displayCfgDialog()
{
    //cancle ids_server display mode preview mode.
    int end = 0;
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_PREVIEW_DISPLAY_MODE,
                           -1, &end, sizeof(end), set_preview_display_mode_cb, this, 3);
    if (mMsgRet != MSG_EXECUTE_OK)
    {
        QString text;
        text.sprintf("取消显示模式预览失败. 错误码: %d", mMsgRet);
        QMessageBox::critical(this, "提示", text, QMessageBox::Yes, NULL);
    }

    delete ui;
}

void displayCfgDialog::paintEvent(QPaintEvent*)
{
    int winW = this->ui->widget->width();
    int winH = this->ui->widget->height();
    int monitorW = 0;
    int monitorH = 0;
    int x, y;

    int i, j;
    for (i=0; i<mDispMode.monitor_nums; i++)
    {
        monitorW = MAX(monitorW, mDispMode.monitor_mode_infos[i].pos_x + mDispMode.monitor_mode_infos[i].monitor_res_info.w);
        monitorH = MAX(monitorH, mDispMode.monitor_mode_infos[i].pos_y + mDispMode.monitor_mode_infos[i].monitor_res_info.h);
    }
    double ratio = MIN((double)winW/monitorW, (double)winH/monitorH);
    x = (winW - ratio*monitorW) / 2;
    y = (winH - ratio*monitorH) / 2;
    x += this->ui->widget->pos().x();
    y += this->ui->widget->pos().y();

    QPainter painter(this);
    painter.setPen(Qt::black);

    QFont font;
    QRect rect;
    QString text;
    font = QApplication::font();
    font.setPixelSize(20);
    painter.setFont(font);

    painter.setPen(Qt::gray);
    rect = QRect(x-1, y-1, monitorW*ratio+2, monitorH*ratio+2);
    painter.drawRect(rect);
    painter.setPen(Qt::black);

    for (i=0; i<mDispMode.monitor_nums; i++)
    {
        rect = QRect(x+mDispMode.monitor_mode_infos[i].pos_x*ratio+1,
                     y+mDispMode.monitor_mode_infos[i].pos_y*ratio+1,
                     mDispMode.monitor_mode_infos[i].monitor_res_info.w*ratio-2,
                     mDispMode.monitor_mode_infos[i].monitor_res_info.h*ratio-2);
        text = mDispMode.monitor_mode_infos[i].name + QString().sprintf("\n(%d*%d)",
                    mDispMode.monitor_mode_infos[i].monitor_res_info.w,
                    mDispMode.monitor_mode_infos[i].monitor_res_info.h);

        if (mModeId == -1 || mResId == -1)
            painter.setPen(Qt::gray);
        else
        {
            for (j=0; j<mDispMode.monitor_nums; j++)
            {
                if (i != j && mMonitorId[i] == mMonitorId[j])
                {
                    painter.setPen(Qt::red);
                    break;
                }
            }
        }
        painter.drawText(rect, Qt::AlignCenter, text);
        painter.setPen(Qt::black);
        painter.drawRect(rect);
    }
}

void displayCfgDialog::mousePressEvent(QMouseEvent *e)
{
    if (mModeId == -1 || mResId == -1)
        return;

    int winW = this->ui->widget->width();
    int winH = this->ui->widget->height();
    int monitorW = 0;
    int monitorH = 0;
    int x, y;

    int i;
    for (i=0; i<mDispMode.monitor_nums; i++)
    {
        monitorW = MAX(monitorW, mDispMode.monitor_mode_infos[i].pos_x + mDispMode.monitor_mode_infos[i].monitor_res_info.w);
        monitorH = MAX(monitorH, mDispMode.monitor_mode_infos[i].pos_y + mDispMode.monitor_mode_infos[i].monitor_res_info.h);
    }
    double ratio = MIN((double)winW/monitorW, (double)winH/monitorH);
    x = (winW - ratio*monitorW) / 2;
    y = (winH - ratio*monitorH) / 2;
    x += this->ui->widget->pos().x();
    y += this->ui->widget->pos().y();

    x = (e->x()-x) / ratio;
    y = (e->y()-y) / ratio;

    int go = (e->button() == Qt::LeftButton) ? 1 : (mMonitorInfos.monitor_nums-1);

    for (i=0; i<mDispMode.monitor_nums; i++)
    {
        if (x >= mDispMode.monitor_mode_infos[i].pos_x &&
            y >= mDispMode.monitor_mode_infos[i].pos_y &&
            x <= mDispMode.monitor_mode_infos[i].pos_x + mDispMode.monitor_mode_infos[i].monitor_res_info.w &&
            y <= mDispMode.monitor_mode_infos[i].pos_y + mDispMode.monitor_mode_infos[i].monitor_res_info.h)
        {
            while (true)
            {
                mMonitorId[i] = (mMonitorId[i]+go) % mMonitorInfos.monitor_nums;
                if (mMonitorValid[mMonitorId[i]] == TRUE)
                    break;
            }
            strcpy(mDispMode.monitor_mode_infos[i].name, mMonitorInfos.monitor_infos[mMonitorId[i]].name);
            break;
        }
    }

    this->repaint();
}

void displayCfgDialog::accept()
{
    if (mModeId == -1 || mResId == -1)
    {
        this->close();
        return;
    }

    int i, j;
    for (i=0; i<mDispMode.monitor_nums; i++)
    {
        for (j=0; j<i; j++)
        {
            if (mMonitorId[i] == mMonitorId[j])
            {
                QMessageBox::critical(this, "提示", "显示器冲突", QMessageBox::Yes, NULL);
                return;
            }
        }
    }

    QMessageBox message(QMessageBox::NoIcon, "显示模式设置",
                        "更改显示模式后,程序将重启,确认更改吗?", QMessageBox::Yes | QMessageBox::No, NULL);
    if(message.exec() == QMessageBox::Yes)
    {
        ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_SET_DISPLAY_MODE,
                               -1, &mDispMode, sizeof(mDispMode), set_preview_display_mode_cb, this, 3);

        if (mMsgRet == MSG_EXECUTE_OK)
            this->close();
        else
        {
            QString text;
            text.sprintf("显示模式设置失败. 错误码: %d", mMsgRet);
            QMessageBox::information(this, "提示", text, QMessageBox::Yes, NULL);
        }
    }
}

void displayCfgDialog::reject()
{
    this->close();
}

void displayCfgDialog::closeEvent(QCloseEvent *event)
{

}

void displayCfgDialog::modeResChanged()
{
    mModeId = -1;
    mResId = -1;
    QList<QListWidgetItem*> items;
    QString text;
    int i, j;

    items = ui->listWidget_mode->selectedItems();
    if (items.size() == 1)
        mModeId = ui->listWidget_mode->row(items[0]);

    items = ui->listWidget_res->selectedItems();
    if (items.size() == 1)
        mResId = ui->listWidget_res->row(items[0]);

    text.sprintf("mModeId:%d. mResId:%d", mModeId, mResId);

    if (mModeId == -1 || mResId == -1)
    {
        ui->listWidget_monitor->clear();
        this->ui->label_monitors->setText("所有显示器");
        for (i=0; i<mMonitorInfos.monitor_nums; i++)
            ui->listWidget_monitor->addItem(QString(mMonitorInfos.monitor_infos[i].name));
        return;
    }

    for (i=0; i<mMonitorInfos.monitor_nums; i++)
    {
        mMonitorValid[i] = FALSE;
        for (j=0; j<MAX_MONITOR_RES_NUMBERS; j++)
        {
            if (mMonitorInfos.monitor_infos[i].monitor_res_infos[j].w <= 0 ||
                mMonitorInfos.monitor_infos[i].monitor_res_infos[j].h <= 0)
                break;
            if (mMonitorInfos.monitor_infos[i].monitor_res_infos[j].w == mResAll[mResId].w &&
                mMonitorInfos.monitor_infos[i].monitor_res_infos[j].h == mResAll[mResId].h )
            {
                mMonitorValid[i] = TRUE;
                break;
            }
        }
    }
    ui->listWidget_monitor->clear();
    this->ui->label_monitors->setText("支持的显示器");
    for (i=0; i<mMonitorInfos.monitor_nums; i++)
        if (mMonitorValid[i] == TRUE)
            ui->listWidget_monitor->addItem(QString(mMonitorInfos.monitor_infos[i].name));

    mDispMode.monitor_nums = mode_v_cnt[mModeId] * mode_h_cnt[mModeId];
    MonitorResInfo res = {mResAll[mResId].w, mResAll[mResId].h, 60, 0};

    j = 0;
    for (i=0; i<mDispMode.monitor_nums; i++)
    {
        while (j < mMonitorInfos.monitor_nums && mMonitorValid[j] == FALSE)
            j++;

        if (j < mMonitorInfos.monitor_nums)
            mMonitorId[i] = j++;
        else
            mMonitorId[i] = mMonitorId[0];
        mDispMode.monitor_mode_infos[i].monitor_res_info = res;
        mDispMode.monitor_mode_infos[i].pos_x = (mResAll[mResId].w) * (i % mode_h_cnt[mModeId]);
        mDispMode.monitor_mode_infos[i].pos_y = (mResAll[mResId].h) * (i / mode_h_cnt[mModeId]);
        strcpy(mDispMode.monitor_mode_infos[i].name, mMonitorInfos.monitor_infos[mMonitorId[i]].name);
    }
    this->repaint();
}
