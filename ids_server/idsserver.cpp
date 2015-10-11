#include "idsserver.h"
#include "ui_idsserver.h"
#include "../common/displaycfgdialog.h"
#include "../common/netcfgdialog.h"
#include "../common/chncfgdialog.h"
#include "../common/layoutcfgdialog.h"

void PlayThread::run()
{
    qDebug() << __func__ << __LINE__ << "mPlayerid" << mPlayerid;
    emit ((idsServer *)mPriv)->idsPlayerStartOne(mPlayerid);
}

static int refresh_event_handle(IdsEvent *pev, gpointer priv)
{
    emit ((idsServer *)priv)->idsPlayerRestart();
    return EHR_ACCEPT;
}

static int disp_mode_cfg_event_handle(IdsEvent *pev, gpointer priv)
{
    if (pev->info.ptr == NULL)
    {
        ((idsServer *)priv)->mDispmodePreviewFlag = FALSE;
        emit ((idsServer *)priv)->idsPlayerShow();
    }
    else
    {
        ((idsServer *)priv)->mDispmodePreviewFlag = TRUE;
        ((idsServer *)priv)->mDispmodePreview = *((DisplayModeInfo *)pev->info.ptr);
        emit ((idsServer *)priv)->idsPlayerHide();
        ((idsServer *)priv)->update();
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

static void layout_set_cb(gpointer buf, gint buf_size, gpointer priv)
{
    Q_ASSERT(buf_size == sizeof(IdsResponse));
    IdsResponse *pres = (IdsResponse *)buf;
    Q_ASSERT(pres->magic == IDS_RESPONSE_MAGIC);
    ((idsServer *)priv)->mMsgRet = pres->ret;
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

    mChnCfg = new QAction(tr("通道配置"), this);
    mChnCfg->setIcon(QIcon(":/image/ipc.ico"));
    connect(mChnCfg, SIGNAL(triggered()), this, SLOT(chnCfgSlot()));

    mLayoutCfg = new QAction(tr("布局配置"), this);
    mLayoutCfg->setIcon(QIcon(":/image/layout.ico"));
    connect(mLayoutCfg, SIGNAL(triggered()), this, SLOT(layoutCfgSlot()));

    mDispmodeCfg = new QAction(tr("显示模式"), this);
    mDispmodeCfg->setIcon(QIcon(":/image/dispmode.ico"));
    connect(mDispmodeCfg, SIGNAL(triggered()), this, SLOT(dispmodeCfgSlot()));

    mNetCfg = new QAction(tr("网络配置"), this);
    mNetCfg->setIcon(QIcon(":/image/network.ico"));
    connect(mNetCfg, SIGNAL(triggered()), this, SLOT(netCfgSlot()));

    mAbout = new QAction(tr("关于"), this);
    mAbout->setIcon(QIcon(":/image/about.ico"));
    connect(mAbout, SIGNAL(triggered()), this, SLOT(aboutSlot()));

    mExit = new QAction(tr("关机"), this);
    mExit->setIcon(QIcon(":/image/exit.ico"));
    connect(mExit, SIGNAL(triggered()), this, SLOT(exitSlot()));

    int i;
    mWinNum = 0;
    for (i=0; i<IDS_LAYOUT_WIN_MAX_NUM; i++)
        mWidgetList[i] = NULL;

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

    connect(this, SIGNAL(idsPlayerRestart()), this, SLOT(idsPlayerRestartSlot()));
    connect(this, SIGNAL(idsPlayerStartOne(int)), this, SLOT(idsPlayerStartOneSlot(int)));
    connect(this, SIGNAL(idsPlayerHide()), this, SLOT(idsPlayerHideSlot()));
    connect(this, SIGNAL(idsPlayerShow()), this, SLOT(idsPlayerShowSlot()));

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
    register_event_listener("ids-server", IDS_EVENT_AMP_REFRESH,
                                     NULL, refresh_event_handle, this);

    QTimer::singleShot(2000, this, SLOT(idsPlayerRestartSlot()));
}

idsServer::~idsServer()
{
    int i;
    for (i=0; i<mSceneNum; i++)
    {
        mSceneSwitchMenu->removeAction(mSceneList[i]);
        mSceneGroup->removeAction(mSceneList[i]);
        delete mSceneList[i];
    }
    delete mSceneGroup;
    delete mSceneSwitchMenu;
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
    idsPlayerStop();
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
        font.setPixelSize(30);
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
}

void idsServer::idsRefresh(void)
{
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_GET_LAYOUT, -1,
                           NULL, 0, layout_get_cb, (void*)this, 1);
    if (mMsgRet != MSG_EXECUTE_OK)
        qDebug() << QString().sprintf("ids cmd get layout error. code = %d.", mMsgRet);

    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_GET_IPC_CFG, -1,
                           NULL, 0, ipc_cfg_get_cb, (void*)this, 1);
    if (mMsgRet != MSG_EXECUTE_OK)
        qDebug() << QString().sprintf("ids get ipc cfg error. code = %d.", mMsgRet);

    int i;
    for (i=0; i<mSceneNum; i++)
    {
        mSceneSwitchMenu->removeAction(mSceneList[i]);
        mSceneGroup->removeAction(mSceneList[i]);
        delete mSceneList[i];
    }
    mSceneNum = mLayoutAll.num;
    for (i=0; i<mSceneNum; i++)
    {
        mSceneList[i] = new QAction(QString(mLayoutAll.layout[i].name), this);        //创建新的菜单项
        mSceneList[i]->setCheckable(true);                                                                     //属性是可选的
        mSceneList[i]->setWhatsThis(QString::number(i, 10));                                      //将该属性用来做场景ID
        connect(mSceneList[i], SIGNAL(triggered()), this, SLOT(sceneSwitchSlot()));   //该菜单项的连接信号和槽
        mSceneGroup->addAction(mSceneList[i]);                                                         //添加菜单项到组里
        mSceneSwitchMenu->addAction(mSceneList[i]);                                               //把action项放入子菜单中
    }
    if (mLayoutAll.id >= 0 && mLayoutAll.id < mSceneNum)
        mSceneList[mLayoutAll.id]->setChecked(true);
}

void idsServer::idsPlayerStart(void)
{
    if (mLayoutAll.id < 0 || mLayoutAll.id >= mSceneNum)
        return;

    mMutex.lock();

    int winW = this->width();
    int winH = this->height();
    int i;

    for (i=0; i<IDS_LAYOUT_WIN_MAX_NUM; i++)
        if (mLayoutAll.layout[mLayoutAll.id].win[i].w <= 0)
            break;
    mWinNum = i;

    for (i=0; i<mWinNum; i++)
    {
        mWidgetList[i] = new VideoWidget(this);
        mWidgetList[i]->setGeometry(winW * mLayoutAll.layout[mLayoutAll.id].win[i].x / IDS_LAYOUT_WIN_W,
                                    winH * mLayoutAll.layout[mLayoutAll.id].win[i].y / IDS_LAYOUT_WIN_H,
                                    winW * mLayoutAll.layout[mLayoutAll.id].win[i].w / IDS_LAYOUT_WIN_W,
                                    winH * mLayoutAll.layout[mLayoutAll.id].win[i].h / IDS_LAYOUT_WIN_H);
        mWidgetList[i]->show();

        mPlayThread[i].mPriv = (void *)this;
        mPlayThread[i].mPlayerid = i;
        mPlayThread[i].start();
    }
    mMutex.unlock();
}

void idsServer::idsPlayerStop(void)
{
    mMutex.lock();
    int i;
    for (i=0; i<mWinNum; i++)
    {
        mPlayMutex[i].lock();

        Q_ASSERT(mWidgetList[i] != NULL);
        mWidgetList[i]->stopPlay();
        mWidgetList[i]->close();
        delete mWidgetList[i];

        mPlayMutex[i].unlock();
    }
    mWinNum = 0;
    mMutex.unlock();
}

void idsServer::idsPlayerRestartSlot(void)
{
    idsRefresh();
    idsPlayerStop();
    idsPlayerStart();
}

void idsServer::idsPlayerStartOneSlot(int playerId)
{
    mPlayMutex[playerId].lock();

    int vid;
    gchar *urls[IPC_CFG_STITCH_CNT] = {0};
    vid = mLayoutAll.layout[mLayoutAll.id].win[playerId].vid;
    if (vid == 0) // stitch
    {
        int i;
        for (i=0; i<IPC_CFG_STITCH_CNT; i++)
        {
            if (0 == mIpcCfgAll.ipc_sti[i].url[0])
                break;
            urls[i] = mIpcCfgAll.ipc_sti[i].url;
        }
        mWidgetList[playerId]->startPlay(urls, i);
    } //if (vid == 0) // stitch
    else
    {
        Q_ASSERT(vid < 2+IPC_CFG_STITCH_CNT+IPC_CFG_NORMAL_CNT);
        if (vid < 2+IPC_CFG_STITCH_CNT)
            urls[0] = mIpcCfgAll.ipc_sti[vid-2].url;
        else
            urls[0] = mIpcCfgAll.ipc[vid-2-IPC_CFG_STITCH_CNT].url;
        mWidgetList[playerId]->startPlay(urls[0]);
    } //else
    mPlayMutex[playerId].unlock();
}

void idsServer::idsPlayerHideSlot(void)
{
    mMutex.lock();
    int i;
    for (i=0; i<mWinNum; i++)
    {
        mPlayMutex[i].lock();
        mWidgetList[i]->hide();
        mPlayMutex[i].unlock();
    }
    mMutex.unlock();
}

void idsServer::idsPlayerShowSlot(void)
{
    mMutex.lock();
    int i;
    for (i=0; i<mWinNum; i++)
    {
        mPlayMutex[i].lock();
        mWidgetList[i]->show();
        mPlayMutex[i].unlock();
    }
    mMutex.unlock();
}

void idsServer::sceneSwitchSlot(void)
{
    if (mLayoutAll.id != mSceneGroup->checkedAction()->whatsThis().toInt())
    {
        mLayoutAll.id = mSceneGroup->checkedAction()->whatsThis().toInt();
        ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_SET_LAYOUT, -1,
                               &mLayoutAll, sizeof(IdsLayoutAll), layout_set_cb, (void*)this, 1);
        if (mMsgRet != MSG_EXECUTE_OK)
            qDebug() << QString().sprintf("ids cmd set layout cfg error. code = %d.", mMsgRet);
    }
}

void idsServer::chnCfgSlot(void)
{
    ChnCfgDialog chnCfg;
//    chnCfg.setGeometry(200, 200, chnCfg.width(), chnCfg.height());
    if (chnCfg.idsUpdate(mIdsEndpoint))
    {
        chnCfg.exec();
    }
}

void idsServer::layoutCfgSlot(void)
{
    layoutCfgDialog layoutDialog;
//    layoutDialog.setGeometry(200, 200, layoutDialog.width(), layoutDialog.height());
    if (layoutDialog.idsUpdate(mIdsEndpoint))
        layoutDialog.exec();
}

void idsServer::dispmodeCfgSlot(void)
{
    displayCfgDialog dispCfg;
    dispCfg.setGeometry(0, 0, dispCfg.width(), dispCfg.height());
    if (dispCfg.idsUpdate(mIdsEndpoint))
        dispCfg.exec();
}

void idsServer::netCfgSlot(void)
{
    NetCfgDialog netCfg;
//    netCfg.setGeometry(200, 200, netCfg.width(), netCfg.height());
    if (netCfg.idsUpdate(mIdsEndpoint))
        netCfg.exec();
}

void idsServer::aboutSlot(void)
{
    QMessageBox::about(this, tr("关于"), tr("视频拼接服务器"));
}

void idsServer::exitSlot(void)
{
    QMessageBox *msgBox = new QMessageBox(QMessageBox::Warning
                                          , tr("警告")
                                          , tr("是否确认关机?")
                                          , QMessageBox::Yes | QMessageBox::No
                                          , NULL);
    msgBox->setWindowFlags(Qt::FramelessWindowHint);
    if (msgBox->exec() == QMessageBox::Yes)
    {
        idsPlayerStop();
        QProcess::execute("shutdown -h now");
    }

    delete msgBox;
}

void idsServer::contextMenuEvent(QContextMenuEvent *e)
{
    if (true == mMutex.tryLock())
    {
        idsRefresh();
        mMutex.unlock();
    }
    mMainMenu->exec(e->globalPos());                    //选择菜单弹出的位置
}
