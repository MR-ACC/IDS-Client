#include "idsserver.h"
#include "ui_idsserver.h"
#include "../common/displaycfgdialog.h"
#include "../common/netcfgdialog.h"
#include "../common/chncfgdialog.h"
#include <QThread>

GLVideoWidget::GLVideoWidget(QWidget *parent) :
    GLWidget((GLWidget*)parent)
{
    setWindowFlags(Qt::FramelessWindowHint);

    QPalette palette;
    palette.setColor(QPalette::Background, QColor(0,50,0));
    this->setPalette(palette);
}

GLVideoWidget::~GLVideoWidget()
{

}

void GLVideoWidget::paintEvent(QPaintEvent* event)
{
    if (mText != "")
    {
//        QPainter painter(this);
//        QFont font = QApplication::font();
//        font.setPixelSize(20);
//        painter.setFont(font);

//        QRect rect;
//        rect = QRect(5, 5, width()-10, height()-10);
//        painter.setPen(QColor(80,80,80));
//        painter.drawRect(rect);
//        painter.setPen(QColor(180,180,180));
//        painter.drawText(rect, Qt::AlignCenter, mText);
    }
}

void PlayThread::run()
{
    emit ((idsServer *)mPriv)->idsPlayerStartOneSlot(mPlayerid);
    qDebug() << __func__ << __LINE__ << "mPlayerid" << mPlayerid;
}

static int disp_mode_cfg_event_handle(IdsEvent *pev, gpointer priv)
{
    if (pev->info.ptr == NULL)
    {
        ((idsServer *)priv)->mDispmodePreviewFlag = FALSE;
        emit ((idsServer *)priv)->idsPlayerStart();
    }
    else
    {
        ((idsServer *)priv)->mDispmodePreviewFlag = TRUE;
        ((idsServer *)priv)->mDispmodePreview = *((DisplayModeInfo *)pev->info.ptr);
        emit ((idsServer *)priv)->idsPlayerStop();
    }
    return EHR_ACCEPT;
}

static void layout_get_cb(gpointer buf, gint buf_size, gpointer priv)
{
    IdsLayoutAll*layout_all = (IdsLayoutAll *)buf;
    if (sizeof(IdsLayoutAll) != buf_size)
    {
        Q_ASSERT (buf_size == sizeof(IdsResponse));
        ((idsServer *)priv)->mMsgRet = ((IdsResponse*)buf)->ret;
    }
    else
    {
        ((idsServer *)priv)->mLayoutAll = *layout_all;
        ((idsServer *)priv)->mMsgRet = MSG_EXECUTE_OK;
    }
}

static void ipc_cfg_get_cb(gpointer buf, gint buf_size, gpointer priv)
{
    IpcCfgAll *ipc_cfg_all = (IpcCfgAll *)buf;
    if (sizeof(IpcCfgAll) != buf_size)
    {
        Q_ASSERT (buf_size == sizeof(IdsResponse));
        ((idsServer *)priv)->mMsgRet = ((IdsResponse*)buf)->ret;
    }
    else
    {
        ((idsServer *)priv)->mIpcCfgAll = *ipc_cfg_all;
        ((idsServer *)priv)->mMsgRet = MSG_EXECUTE_OK;
    }
}

idsServer::idsServer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::idsServer)
{
    ui->setupUi(this);
    ui->gridLayout->setMargin(0);

    QPalette palette;
    palette.setColor(QPalette::Background, QColor(0,0,0));
    this->setPalette(palette);
    this->setAutoFillBackground(true);

    setGeometry(0, 0, QApplication::desktop()->width(), QApplication::desktop()->height());
    setWindowFlags(Qt::FramelessWindowHint);
//    setAttribute(Qt::WA_TranslucentBackground,true);

//    this->setWindowState( Qt::WindowFullScreen );
//    this->setWindowOpacity(0.3);

    mChnCfg = new QAction(tr("通道配置 "), this);
    mChnCfg->setIcon(QIcon(":/image/ipc.ico"));
    connect(mChnCfg, SIGNAL(triggered()), this, SLOT(chnCfgSlot()));

    mLayoutCfg = new QAction(tr("layout配置 "), this);
    mLayoutCfg->setIcon(QIcon(":/image/layout.ico"));
    connect(mLayoutCfg, SIGNAL(triggered()), this, SLOT(layoutCfgSlot()));

    mDispmodeCfg = new QAction(tr("display mode配置 "), this);
    mDispmodeCfg->setIcon(QIcon(":/image/dispmode.ico"));
    connect(mDispmodeCfg, SIGNAL(triggered()), this, SLOT(dispmodeCfgSlot()));

    mNetCfg = new QAction(tr("网络配置 "), this);
    mNetCfg->setIcon(QIcon(":/image/network.ico"));
    connect(mNetCfg, SIGNAL(triggered()), this, SLOT(netCfgSlot()));

    mAbout = new QAction(tr("关    于"), this);
    mAbout->setIcon(QIcon(":/image/about.ico"));
    connect(mAbout, SIGNAL(triggered()), this, SLOT(aboutSlot()));

    mExit = new QAction(tr("关    机"), this);
    mExit->setIcon(QIcon(":/image/exit.ico"));
    connect(mExit, SIGNAL(triggered()), this, SLOT(exitSlot()));

    int i;
    mWinNum = 0;
    for (i=0; i<IDS_LAYOUT_WIN_MAX_NUM; i++)
    {
        mPlayerList[i] = NULL;
        mWidgetList[i] = NULL;
    }

    mSceneNum = 0;
    mSceneGroup = new QActionGroup(this);        //创建菜单项组，里面的菜单项为互斥
    for (i=0; i<IDS_LAYOUT_MAX_NUM; i++)
        mSceneList[i] = NULL;

    mMainMenu = new QMenu();  //创建主菜单
    mSceneSwitchMenu = mMainMenu->addMenu(tr("选择布局"));   //在主菜单中创建子菜单one pictures
    mSceneSwitchMenu->setIcon(QIcon(":/image/scene.ico"));
    mMainMenu->addSeparator();
    mMainMenu->addSeparator();
    mMainMenu->addAction(mChnCfg);               //把action项放入主菜单中
    mMainMenu->addSeparator();
    mMainMenu->addSeparator();
    mMainMenu->addAction(mLayoutCfg);               //把action项放入主菜单中
    mMainMenu->addSeparator();
    mMainMenu->addSeparator();
    mMainMenu->addAction(mDispmodeCfg);         //把action项放入主菜单中
    mMainMenu->addSeparator();
    mMainMenu->addSeparator();
    mMainMenu->addAction(mNetCfg);               //把action项放入主菜单中
    mMainMenu->addSeparator();
    mMainMenu->addSeparator();
    mMainMenu->addAction(mAbout);                   //把about项放入主菜单中
    mMainMenu->addSeparator();
    mMainMenu->addSeparator();
    mMainMenu->addAction(mExit);
    mMainMenu->addSeparator();
    mMainMenu->addSeparator();

    mDispmodePreviewFlag = FALSE;

    connect(this, SIGNAL(idsPlayerStart()), this, SLOT(idsPlayerStartSlot()));
    connect(this, SIGNAL(idsPlayerStop()), this, SLOT(idsPlayerStopSlot()));
    connect(this, SIGNAL(idsPlayerStartOne(int)), this, SLOT(idsPlayerStartOneSlot(int)));

    if (TRUE != ids_core_init())
        throw QString("ids init core library failed");
    if (TRUE != ids_modules_init())
        throw QString("ids init modules library failed");
    if (TRUE != ids_cmd_app_amp_init())
        throw QString("ids init app amp library failed");

    mIdsEndpoint = ids_create_local_endpoint();
    if (NULL == mIdsEndpoint)
        throw QString("ids_create_remote_endpoint failed");

    register_event_listener("ids-server", IDS_EVENT_DISP_MODE_CFG,
                                     NULL, disp_mode_cfg_event_handle, this);

    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_GET_LAYOUT, -1,
                           NULL, 0, layout_get_cb, (void*)this, 1);
    if (mMsgRet != MSG_EXECUTE_OK)
        QMessageBox::critical(this, "error", QString().sprintf("ids cmd get layout error. code = %d.", mMsgRet), QMessageBox::Yes, NULL);
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_GET_IPC_CFG, -1,
                           NULL, 0, ipc_cfg_get_cb, (void*)this, 1);
    if (mMsgRet != MSG_EXECUTE_OK)
        QMessageBox::critical(this, "error", QString().sprintf("ids get ipc cfg error. code = %d.", mMsgRet), QMessageBox::Yes, NULL);

    newSceneList();
    mSceneId = 0;                                                       //default play the first scene
    QTimer::singleShot(500, this, SLOT(idsPlayerStartSlot()));
}

idsServer::~idsServer()
{
    delete mSceneGroup;
//    delete mSceneSwitchMenu;
    delete mMainMenu;
    delete mExit;
    delete mAbout;
    delete mNetCfg;
    delete mDispmodeCfg;
    delete mLayoutCfg;
    delete mChnCfg;
    delete ui;
}

void idsServer::closeEvent(QCloseEvent *)
{
    ids_destroy_local_endpoint(mIdsEndpoint);
    idsPlayerStopSlot();
    deleteSceneList();
}

void idsServer::showEvent(QShowEvent *)
{
}

void idsServer::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QFont font = QApplication::font();
    QRect rect;
    QString text;
    int i;

    if (TRUE == mDispmodePreviewFlag)
    {
        painter.setPen(Qt::white);
        font.setPixelSize(60);
        painter.setFont(font);

        for (i=0; i<mDispmodePreview.monitor_nums; i++)
        {
            rect = QRect(mDispmodePreview.monitor_mode_infos[i].pos_x+100,
                         mDispmodePreview.monitor_mode_infos[i].pos_y+100,
                         mDispmodePreview.monitor_mode_infos[i].monitor_res_info.w-200,
                         mDispmodePreview.monitor_mode_infos[i].monitor_res_info.h-200);
            text = mDispmodePreview.monitor_mode_infos[i].name + QString().sprintf("\n(%d*%d)",
                        mDispmodePreview.monitor_mode_infos[i].monitor_res_info.w,
                        mDispmodePreview.monitor_mode_infos[i].monitor_res_info.h);

            painter.drawText(rect, Qt::AlignCenter, text);
            painter.drawRect(rect);
        }
    }
    else
    {
        painter.setPen(Qt::red);
        font.setPixelSize(20);
        painter.setFont(font);
        painter.drawText(QRect(200, 10, 400, 400), Qt::AlignCenter, "Main window is transparent. you can't see me.");

//        QBitmap bitMap(width(),height()); // A bit map has the same size with current widget
//        QPainter p(&bitMap);
//        p.setPen(QColor(255,255,255)); // Any color that is not QRgb(0,0,0) is right
//        p.drawRect(0,0,width(),height());
//        // Now begin to draw the place where we want to show it
//        p.setPen(QColor(0,0,0));
//        p.drawText(QRect(100, 100, 100, 100), Qt::AlignCenter, "test");
//        p.fillRect(0,0,width()/2,height()/2, Qt::SolidPattern);
//        p.fillRect(width()/2,height()/2, width()/2,height()/2, Qt::SolidPattern);
//        setMask(bitMap);

//        painter.setCompositionMode( QPainter::CompositionMode_Clear);
//        painter.setPen(QColor(255,0,0));
//        painter.fillRect( 600, 200, 200, 200, Qt::SolidPattern );
//        painter.fillRect( 400, 200, 400, 400, QColor(0,255,0,255));
    }
}

void idsServer::newSceneList(void)
{
    mSceneNum = mLayoutAll.num;
    int i;
    for (i=0; i<mSceneNum; i++)
    {
        mSceneList[i] = new QAction(QString(mLayoutAll.layout[i].name), this);        //创建新的菜单项
        mSceneList[i]->setCheckable(true);                                                                     //属性是可选的
        mSceneList[i]->setWhatsThis(QString::number(i, 10));                                      //将该属性用来做场景ID
        connect(mSceneList[i], SIGNAL(triggered()), this, SLOT(sceneSwitchSlot()));   //该菜单项的连接信号和槽
        mSceneGroup->addAction(mSceneList[i]);                                                         //添加菜单项到组里
        mSceneSwitchMenu->addAction(mSceneList[i]);                                               //把action项放入子菜单中
    }
}
void idsServer::deleteSceneList(void)
{
    int i;
    for (i=0; i<mSceneNum; i++)
    {
        mSceneSwitchMenu->removeAction(mSceneList[i]);
        mSceneGroup->removeAction(mSceneList[i]);
        delete mSceneList[i];
    }
    mSceneNum = 0;
}

void idsServer::idsPlayerStartSlot(void)
{
    mMutex.lock();
    if (mSceneId < 0 || mSceneId >= mSceneNum)
        return;

    int winW = this->width();
    int winH = this->height();
    int i;

    for (i=0; i<IDS_LAYOUT_WIN_MAX_NUM; i++)
        if (mLayoutAll.layout[mSceneId].win[i].w <= 0)
            break;
    mWinNum = i;

    mWinIdStitch = -1;
    mWinIdLink = -1;

    for (i=0; i<mWinNum; i++)
   {
        mPlayerList[i] = NULL;
        mWidgetList[i] = new GLVideoWidget(this);
        mWidgetList[i]->setGeometry(
//                    winW * mLayoutAll.layout[mSceneId].win[i].x / IDS_LAYOUT_WIN_W + this->pos().x(),
//                    winH * mLayoutAll.layout[mSceneId].win[i].y / IDS_LAYOUT_WIN_H + this->pos().y(),
                    winW * mLayoutAll.layout[mSceneId].win[i].x / IDS_LAYOUT_WIN_W,
                    winH * mLayoutAll.layout[mSceneId].win[i].y / IDS_LAYOUT_WIN_H,
                                  winW * mLayoutAll.layout[mSceneId].win[i].w / IDS_LAYOUT_WIN_W,
                                  winH * mLayoutAll.layout[mSceneId].win[i].h / IDS_LAYOUT_WIN_H);
        mWidgetList[i]->mText = QString("connecting...");
//        mWidgetList[i]->repaint();
        mWidgetList[i]->show();

//        idsPlayerStartOneSlot(i);
        mPlayThread[i].mPriv = (void *)this;
        mPlayThread[i].mPlayerid = i;
        mPlayThread[i].start();
    }
    mMutex.unlock();
}

void idsServer::idsPlayerStartOneSlot(int i)
{
    qDebug()<<__func__<<__LINE__<<"play: "<<i;

    WindowInfo winfo[IPC_CFG_STITCH_CNT];
    int j, vid, ret;
    vid = mLayoutAll.layout[mSceneId].win[i].vid;
    if (vid == 0) // stitch
    {
        mWinIdStitch = i;
        for (j=0; j<IPC_CFG_STITCH_CNT; j++)
        {
            if (0 == mIpcCfgAll.ipc_sti[j].url[0])
                break;
            winfo[j].media_url = mIpcCfgAll.ipc_sti[j].url;
            winfo[j].win_w = this->mWidgetList[i]->width();
            winfo[j].win_h = this->mWidgetList[i]->height();
            winfo[j].win_id = GUINT_TO_POINTER(mWidgetList[i]->winId());
            winfo[j].flags = IDS_USE_THE_SAME_WINDOW | IDS_ENABLE_ALGO_GL_HWACCEL;
            winfo[j].draw = glwidget_render_frame_cb;
            winfo[j].priv = this->mWidgetList[i];
        }
        if (j == 0)
            mWidgetList[i]->mText = QString("url is null.");
        else
        {
            ret = ids_play_stream(&winfo[0], j, IDS_TYPE(IDS_TYPE_STITCH) | IDS_TYPE(IDS_TYPE_MOUSE_LINKAGE), NULL, this, &mPlayerList[i]);
            if (0 != ret)
                mWidgetList[i]->mText = QString("");
            else
            {
                mWidgetList[i]->mText = QString("connect failed.");
                for (j=0; j<IPC_CFG_STITCH_CNT; j++)
                {
                    if (mIpcCfgAll.ipc_sti[j].url[0] == 0)
                        break;
                    mWidgetList[i]->mText += QString("\n");
                    mWidgetList[i]->mText += QString(mIpcCfgAll.ipc_sti[j].url);
                }
            }
        }
    } //if (vid == 0) // stitch
    else if (vid == 1) //link
    {
        mWinIdLink = i;
    } //else if (vid == 1) //link
    else
    {
        Q_ASSERT(vid < 2+IPC_CFG_STITCH_CNT+IPC_CFG_NORMAL_CNT);
        if (vid < 2+IPC_CFG_STITCH_CNT)
            winfo[0].media_url = mIpcCfgAll.ipc_sti[vid-2].url;
        else
            winfo[0].media_url = mIpcCfgAll.ipc[vid-2-IPC_CFG_STITCH_CNT].url;
        if (0 == winfo[0].media_url[0])
            ((GLVideoWidget *)mWidgetList[i])->mText = QString("url is null.");
        else
        {
            winfo[0].win_w = this->mWidgetList[i]->width();
            winfo[0].win_h = this->mWidgetList[i]->height();
            winfo[0].win_id = GUINT_TO_POINTER(mWidgetList[i]->winId());
            winfo[0].flags = IDS_ENABLE_ALGO_GL_HWACCEL;
            winfo[0].draw = glwidget_render_frame_cb;
            winfo[0].priv = this->mWidgetList[i];
            ret = ids_play_stream(&winfo[0], 1, 0, NULL, this, &mPlayerList[i]);
            if (0 != ret)
                mWidgetList[i]->mText = QString("");
            else
            {
                mWidgetList[i]->mText = QString("connect failed.\n");
                mWidgetList[i]->mText += QString(winfo[0].media_url);
            }
        }
    } //else
    mWidgetList[i]->repaint();
}

void idsServer::idsPlayerStopSlot(void)
{
    mMutex.lock();
    int i;

    for (i=0; i<mWinNum; i++)
    {
        if (mPlayerList[i] != NULL)
        {
            ids_stop_stream(mPlayerList[i]);
            mPlayerList[i] = NULL;
        }
        if (mWidgetList[i] != NULL)
        {
            mWidgetList[i]->close();
            delete mWidgetList[i];
            mWidgetList[i] = NULL;
        }
    }
    mWinNum = 0;
    mMutex.unlock();
}

void idsServer::sceneSwitchSlot(void)
{
    if (mSceneId != mSceneGroup->checkedAction()->whatsThis().toInt())
    {
        mSceneId = mSceneGroup->checkedAction()->whatsThis().toInt();
        idsPlayerStopSlot();
        idsPlayerStartSlot();
    }
}

void idsServer::chnCfgSlot(void)
{
    ChnCfgDialog chnCfg;
    chnCfg.setGeometry(200, 200, 640, 480);

    if (chnCfg.update(mIdsEndpoint))
        chnCfg.exec();
    else
        qDebug() << QString().sprintf("ids get ipc cfg error. code = %d.", chnCfg.mMsgRet);
}

void idsServer::layoutCfgSlot(void)
{
}

void idsServer::dispmodeCfgSlot(void)
{
    displayCfgDialog dispCfg;
    dispCfg.update(mIdsEndpoint);
    dispCfg.setGeometry(200, 200, 600, 400);
    dispCfg.exec();
}

void idsServer::netCfgSlot(void)
{
    NetCfgDialog netCfg;
    netCfg.update(mIdsEndpoint);
    netCfg.setGeometry(200, 200, 200, 200);
    netCfg.exec();
}

void idsServer::aboutSlot(void)
{
    QMessageBox::about(this, tr("关于"), tr("ids_server"));
}

void idsServer::exitSlot(void)
{
    idsPlayerStopSlot();
    QMessageBox *msgBox = new QMessageBox(QMessageBox::Warning
                                          , tr("警告")
                                          , tr("是否确认关机?")
                                          , QMessageBox::Yes | QMessageBox::No
                                          , NULL);
    msgBox->setWindowFlags(Qt::FramelessWindowHint);
    if (msgBox->exec() == QMessageBox::Yes)
        QProcess::execute("shutdown -h now");

    delete msgBox;
}

void idsServer::contextMenuEvent(QContextMenuEvent *e)
{
    if (true == mMutex.tryLock())
    {
        ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_GET_LAYOUT, -1,
                               NULL, 0, layout_get_cb, (void*)this, 1);
        if (mMsgRet != MSG_EXECUTE_OK)
            qDebug() << QString().sprintf("ids cmd get layout error. code = %d.", mMsgRet);
        ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_GET_IPC_CFG, -1,
                               NULL, 0, ipc_cfg_get_cb, (void*)this, 1);
        if (mMsgRet != MSG_EXECUTE_OK)
            qDebug() << QString().sprintf("ids cmd get ipc cfg error. code = %d.", mMsgRet);
        deleteSceneList();
        newSceneList();
        mMutex.unlock();
    }
    mMainMenu->exec(e->globalPos());                    //选择菜单弹出的位置
}
