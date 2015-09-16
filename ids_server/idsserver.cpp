#include "idsserver.h"
#include "ui_idsserver.h"

int idsServer::disp_mode_cfg_event_handle(IdsEvent *pev, gpointer priv)
{
    if (pev->info.ptr == NULL)
    {
        emit ((idsServer *)priv)->idsPlayerStart();
    }
    else
    {
        emit ((idsServer *)priv)->idsPlayerStop();
        ((idsServer *)priv)->mDispmodePreview = *((DisplayModeInfo *)pev->info.ptr);
//        ((idsServer *)priv)->idsPlayerStopSlot();
        ((idsServer *)priv)->repaint();
    }

    return EHR_ACCEPT;
}

void layout_get_cb(gpointer buf, gint buf_size, gpointer priv)
{
    IdsLayoutAll*layout_all = (IdsLayoutAll *)buf;
    if (sizeof(IdsLayoutAll) != buf_size)
    {
        Q_ASSERT (buf_size == sizeof(IdsResponse));
        qDebug() << QString().sprintf("layout_cfg_get_cb timeout error. ret = %d", ((IdsResponse*)buf)->ret);
    }
    else
        ((idsServer *)priv)->mLayoutAll = *layout_all;
}

void ipc_cfg_get_cb(gpointer buf, gint buf_size, gpointer priv)
{
    IpcCfgAll *ipc_cfg_all = (IpcCfgAll *)buf;
    if (sizeof(IpcCfgAll) != buf_size)
    {
        Q_ASSERT (buf_size == sizeof(IdsResponse));
        qDebug() << QString().sprintf("ipc_cfg__get_cb timeout error. ret = %d", ((IdsResponse*)buf)->ret);
    }
    else
        ((idsServer *)priv)->mIpcCfgAll = *ipc_cfg_all;
}

idsServer::idsServer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::idsServer)
{
    ui->setupUi(this);

    this->setWindowState( Qt::WindowFullScreen );
    this->setAutoFillBackground(true);

    QPalette palette;
    palette.setColor(QPalette::Background, QColor(0,0,0));
    this->setPalette(palette);
    this->ui->gridLayout->setMargin(0);

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

    if (TRUE != ids_core_init())
        throw QString("init core library failed");
    if (TRUE != ids_modules_init())
        throw QString("init modules library failed");
    if (TRUE != ids_cmd_app_amp_init())
        throw QString("init app amp library failed");

    mIdsEndpoint = ids_create_local_endpoint();
    if (NULL == mIdsEndpoint)
        throw QString("ids_create_remote_endpoint failed");

    register_event_listener("ids-server", IDS_EVENT_DIS_CFG_MODE,
                                     NULL, disp_mode_cfg_event_handle, this);

    mDispmodePreviewFlag = FALSE;

    mSceneNum = 0;
    mSceneGroup = new QActionGroup(this);        //创建菜单项组，里面的菜单项为互斥
    for (i=0; i<IDS_LAYOUT_MAX_NUM; i++)
        mSceneList[i] = NULL;

    connect(this, SIGNAL(idsPlayerStart()), this, SLOT(idsPlayerStartSlot()));
    connect(this, SIGNAL(idsPlayerStop()), this, SLOT(idsPlayerStopSlot()));

    getSceneList();
    mSceneId = 0;                                                       //default play 0
    QTimer::singleShot(100, this, SLOT(idsPlayerStartSlot()));
}

idsServer::~idsServer()
{
    idsPlayerStopSlot();
    ids_destroy_local_endpoint(mIdsEndpoint);
    delete ui;
}

void idsServer::paintEvent(QPaintEvent*)
{
    if (TRUE == mDispmodePreviewFlag)
    {
        mDispmodePreviewFlag = FALSE;

        QPainter painter(this);
        QFont font = QApplication::font();
        font.setPixelSize(60);
        painter.setFont(font);
        painter.setPen(Qt::white);
        QRect rect;
        QString text;
        int i;

        for (i=0; i<mDispmodePreview.monitor_nums; i++)
        {
            rect = QRect(mDispmodePreview.monitor_mode_infos[i].pos_x+10,
                         mDispmodePreview.monitor_mode_infos[i].pos_y+10,
                         mDispmodePreview.monitor_mode_infos[i].monitor_res_info.w-20,
                         mDispmodePreview.monitor_mode_infos[i].monitor_res_info.h-20);
            text = mDispmodePreview.monitor_mode_infos[i].name + QString().sprintf("\n(%d*%d)",
                        mDispmodePreview.monitor_mode_infos[i].monitor_res_info.w,
                        mDispmodePreview.monitor_mode_infos[i].monitor_res_info.h);

            painter.drawText(rect, Qt::AlignCenter, text);
            painter.drawRect(rect);
        }
    }
    else
    {
    }
}

void idsServer::getSceneList(void)
{
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_GET_LAYOUT, -1,
                           NULL, 0, layout_get_cb, (void*)this, 1);
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_GET_IPC_CFG, -1,
                           NULL, 0, ipc_cfg_get_cb, (void*)this, 1);
    gint i;

    if (mSceneNum > 0)
    {
        for (i=0; i<mSceneNum; i++)
        {
            mSceneGroup->removeAction(mSceneList[i]);
            delete mSceneList[i];
        }
        mSceneNum = 0;
    }
    mSceneNum = mLayoutAll.num;
    if (mSceneNum > 0)
    {
        for (i=0; i<mSceneNum; i++)
        {
            mSceneList[i] = new QAction(QString(mLayoutAll.layout[i].name), this);  //创建新的菜单项
            mSceneList[i]->setCheckable(true);                                      //属性是可选的
            mSceneList[i]->setWhatsThis(QString::number(i, 10));                    //将该属性用来做场景ID
            connect(mSceneList[i], SIGNAL(triggered()), this, SLOT(sceneSwitchSlot())); //该菜单项的连接信号和槽
            mSceneGroup->addAction(mSceneList[i]);                                  //添加菜单项到组里
            mSceneSwitchMenu->addAction(mSceneList[i]);                             //把action项放入子菜单中
        }
    }
}

void idsServer::idsPlayerStartSlot(void)
{
    if (mSceneId < 0 || mSceneId >= mSceneNum)
        return;

    int winW = this->width();
    int winH = this->height();
    int i, j, ret;

    for (i=0; i<IDS_LAYOUT_WIN_MAX_NUM; i++)
        if (mLayoutAll.layout[mSceneId].win[i].w <= 0)
            break;
    mWinNum = i;

    WindowInfo winfo[IPC_CFG_STITCH_CNT];

    for (i=0; i<mWinNum; i++)
    {
        mPlayerList[i] = NULL;
        mWidgetList[i] = new QWidget(this);
        mWidgetList[i]->setAutoFillBackground(true);
        mWidgetList[i]->setGeometry(winW * mLayoutAll.layout[mSceneId].win[i].x / IDS_LAYOUT_WIN_W
                                  , winH * mLayoutAll.layout[mSceneId].win[i].y / IDS_LAYOUT_WIN_H
                                  , winW * mLayoutAll.layout[mSceneId].win[i].w / IDS_LAYOUT_WIN_W
                                  , winH * mLayoutAll.layout[mSceneId].win[i].h / IDS_LAYOUT_WIN_H);
        mWidgetList[i]->show();

        // vid--------0:stitch.    1:linkage.     2~5:g0~g3.      6~69:d0~d63
        if (mLayoutAll.layout[mSceneId].win[i].vid == 0) // stitch
        {
            for (j=0; j<IPC_CFG_STITCH_CNT; j++)
            {
                if (mIpcCfgAll.ipc_sti[j].url[0] == 0)
                    break;
                winfo[j].media_url = mIpcCfgAll.ipc_sti[j].url;
                winfo[j].win_w = this->mWidgetList[i]->width();
                winfo[j].win_h = this->mWidgetList[i]->height();
                winfo[j].win_id = GUINT_TO_POINTER(mWidgetList[i]->winId());
                winfo[j].flags = IDS_USE_THE_SAME_WINDOW;
            }
            if (j > 0)
            {
                ret = ids_play_stream(&winfo[0], j, IDS_TYPE(IDS_TYPE_STITCH), NULL, this, &mPlayerList[i]);
                if (ret == 0)
                   qDebug() << "ids_play_stream failed. stitch. vid: " << mLayoutAll.layout[mSceneId].win[i].vid;
            }
        }
        else if (mLayoutAll.layout[mSceneId].win[i].vid == 1)
        {
        }
        else if (mLayoutAll.layout[mSceneId].win[i].vid < 2+IPC_CFG_STITCH_CNT)
        {
            j = mLayoutAll.layout[mSceneId].win[i].vid-2;
            if (mIpcCfgAll.ipc_sti[j].url[0])
            {
                winfo[0].media_url = mIpcCfgAll.ipc_sti[j].url;
                winfo[0].win_w = this->mWidgetList[i]->width();
                winfo[0].win_h = this->mWidgetList[i]->height();
                winfo[0].win_id = GUINT_TO_POINTER(mWidgetList[i]->winId());
                winfo[0].flags = 0;
                ret = ids_play_stream(&winfo[0], 1, 0, NULL, this, &mPlayerList[i]);
                if (ret == 0)
                   qDebug() << "ids_play_stream failed. vid: " << mLayoutAll.layout[mSceneId].win[i].vid;
            }
        }
        else if (mLayoutAll.layout[mSceneId].win[i].vid < 2+IPC_CFG_STITCH_CNT+IPC_CFG_NORMAL_CNT)
        {
            j = mLayoutAll.layout[mSceneId].win[i].vid-2-IPC_CFG_STITCH_CNT;
            if (mIpcCfgAll.ipc[j].url[0])
            {
                winfo[0].media_url = mIpcCfgAll.ipc[j].url;
                winfo[0].win_w = this->mWidgetList[i]->width();
                winfo[0].win_h = this->mWidgetList[i]->height();
                winfo[0].win_id = GUINT_TO_POINTER(mWidgetList[i]->winId());
                winfo[0].flags = 0;
                ret = ids_play_stream(&winfo[0], 1, 0, NULL, this, &mPlayerList[i]);
                if (ret == 0)
                   qDebug() << "ids_play_stream failed. vid: " << mLayoutAll.layout[mSceneId].win[i].vid;
            }
        }
    }
}

void idsServer::idsPlayerStopSlot(void)
{
    int i;

    for (i=0; i<mWinNum; i++)
    {
        if (mPlayerList[i] != NULL)
            ids_stop_stream(mPlayerList[i]);
        if (mWidgetList[i] != NULL)
        {
            mWidgetList[i]->close();
            delete mWidgetList[i];
        }
    }
    mWinNum = 0;
}

void idsServer::sceneSwitchSlot(void)
{
    idsPlayerStopSlot();
    repaint();
    mSceneId = mSceneGroup->checkedAction()->whatsThis().toInt();
    idsPlayerStartSlot();
}

void idsServer::chnCfgSlot(void)
{
}

void idsServer::layoutCfgSlot(void)
{
    idsPlayerStartSlot();
}

void idsServer::dispmodeCfgSlot(void)
{
    displayCfgDialog dispCfg;
    dispCfg.mIdsServerWin = this;
    dispCfg.getInfo(mIdsEndpoint);
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
    getSceneList();
    mMainMenu->exec(e->globalPos());                    //选择菜单弹出的位置
}
