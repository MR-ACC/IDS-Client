#include "idsserver.h"
#include "ui_idsserver.h"

void ids_io_fin_cb(gpointer priv)
{
    ((idsServer*)priv)->mIdsEndpoint = ids_create_remote_endpoint("192.168.1.56", 1702, ids_io_fin_cb, priv, NULL);
    if (NULL == ((idsServer*)priv)->mIdsEndpoint)
        throw QString("ids_io_fin_cb. ids_create_remote_endpoint failed");
}

int dis_cfg_mode_event_handle(IdsEvent *pev, gpointer priv)
{
    Q_ASSERT(pev->info.ptr != NULL);

    ((idsServer *)priv)->mDispModePreviewFlag = TRUE;
    ((idsServer *)priv)->mDispModePreview = *((DisplayModeInfo *)pev->info.ptr);
    ((idsServer *)priv)->repaint();
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

    mChnConfig = new QAction(tr("通道配置 "), this);
    mChnConfig->setIcon(QIcon(":/image/config.ico"));
    connect(mChnConfig, SIGNAL(triggered()), this, SLOT(chnConfigSlot()));

    mNetConfig = new QAction(tr("网络配置 "), this);
    mNetConfig->setIcon(QIcon(":/image/network.ico"));
    connect(mNetConfig, SIGNAL(triggered()), this, SLOT(netConfigSlot()));

    mAbout = new QAction(tr("  关    于 "), this);
    mAbout->setIcon(QIcon(":/image/about.ico"));
    connect(mAbout, SIGNAL(triggered()), this, SLOT(aboutSlot()));

    mExit = new QAction(tr("  关    机 "), this);
    mExit->setIcon(QIcon(":/image/exit.ico"));
    connect(mExit, SIGNAL(triggered()), this, SLOT(exitSlot()));

    int i;
    mSceneNum = 0;
    mSceneGroup = new QActionGroup(this);        //创建菜单项组，里面的菜单项为互斥
    for (i=0; i<IDS_LAYOUT_MAX_NUM; i++)
        mSceneList[i] = NULL;

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

    mMainMenu->addAction(mChnConfig);               //把action项放入主菜单中
    mMainMenu->addSeparator();
    mMainMenu->addSeparator();
    mMainMenu->addAction(mNetConfig);               //把action项放入主菜单中
    mMainMenu->addSeparator();
    mMainMenu->addSeparator();
    mMainMenu->addAction(mAbout);                   //把about项放入主菜单中
    mMainMenu->addSeparator();
    mMainMenu->addSeparator();

    mMainMenu->addAction(mExit);
    mMainMenu->addSeparator();
    mMainMenu->addSeparator();

    mDispModePreviewFlag = FALSE;

    gint ret;
    if (TRUE != ids_core_init())
        throw QString("init core library failed");
    if (TRUE != ids_modules_init())
        throw QString("init modules library failed");
    if (TRUE != ids_cmd_app_amp_init())
        throw QString("init app amp library failed");

//    mIdsEndpoint = ids_create_remote_endpoint("192.168.1.56", 1702, ids_io_fin_cb, this, NULL);
    mIdsEndpoint = ids_create_local_endpoint();
    if (NULL == mIdsEndpoint)
        throw QString("ids_create_remote_endpoint failed");

    cfg_id = register_event_listener("ids-server", IDS_EVENT_DIS_CFG_MODE,
                                     NULL, dis_cfg_mode_event_handle, this);
}

idsServer::~idsServer()
{
    ids_destroy_local_endpoint(mIdsEndpoint);

    delete ui;
}

void idsServer::paintEvent(QPaintEvent*)
{
    if (TRUE == mDispModePreviewFlag)
    {
        mDispModePreviewFlag = FALSE;

        QPainter painter(this);
        QFont font = QApplication::font();
        font.setPixelSize(20);
        painter.setFont(font);
        painter.setPen(Qt::white);
        QRect rect;
        QString text;
        int i;

        for (i=0; i<mDispModePreview.monitor_nums; i++)
        {
            rect = QRect(mDispModePreview.monitor_mode_infos[i].pos_x+1,
                         mDispModePreview.monitor_mode_infos[i].pos_y+1,
                         mDispModePreview.monitor_mode_infos[i].monitor_res_info.w-2,
                         mDispModePreview.monitor_mode_infos[i].monitor_res_info.h-2);
            text = mDispModePreview.monitor_mode_infos[i].name + QString().sprintf("\n(%d*%d)",
                        mDispModePreview.monitor_mode_infos[i].monitor_res_info.w,
                        mDispModePreview.monitor_mode_infos[i].monitor_res_info.h);

            painter.drawText(rect, Qt::AlignCenter, text);
            painter.drawRect(rect);
        }
    }
}

void layout_get_callback(gpointer buf, gint buf_size, gpointer priv)
{
    IdsLayoutAll*layout_all = (IdsLayoutAll *)buf;
    if (sizeof(IdsLayoutAll) != buf_size)
    {
        if (buf_size == sizeof(IdsResponse))
            qDebug() << "layout_get_callback timeout(if ret == 1-). ret = " << ((IdsResponse*)buf)->ret;
    }
    else
        ((idsServer *)priv)->mLayoutAll = *layout_all;
}
void ipc_cfg_get_callback(gpointer buf, gint buf_size, gpointer priv)
{
    IpcCfgAll *ipc_cfg_all = (IpcCfgAll *)buf;
    if (sizeof(IpcCfgAll) != buf_size)
    {
        if (buf_size == sizeof(IdsResponse))
            qDebug() << "ipc_cfg_get_callback timeout(if ret == -1). ret = " << ((IdsResponse*)buf)->ret;
    }
    else
        ((idsServer *)priv)->mIpcCfgAll = *ipc_cfg_all;
}

void idsServer::getSceneList(void)
{
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_GET_LAYOUT, -1,
                           NULL, 0, layout_get_callback, (void*)this, 1);
    ids_net_write_msg_sync(mIdsEndpoint, IDS_CMD_GET_IPC_CFG, -1,
                           NULL, 0, ipc_cfg_get_callback, (void*)this, 1);
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
    //Scene1->setChecked(true);          //设置默认的菜单组的菜单项状态，firstChannel被选中
}

void idsServer::sceneStopPlay(void)
{
    int i;

    for (i=0; i<mWinNum; i++)
    {
        if (mPlayerList[i] != NULL)
            ids_stop_stream(mPlayerList[i]);
        if (mWidgetList[i] != NULL)
            delete mWidgetList[i];
    }
    mWinNum = 0;
}

void idsServer::sceneSwitchSlot(void)
{
    sceneStopPlay();

    int id = mSceneGroup->checkedAction()->whatsThis().toInt();
    int winW = this->width();
    int winH = this->height();
    int i, j, ret;

    if (id == 1) //debug use
        return;

    for (i=0; i<IDS_LAYOUT_WIN_MAX_NUM; i++)
        if (mLayoutAll.layout[id].win[i].w <= 0)
            break;
    mWinNum = i;

    WindowInfo winfo[IPC_CFG_STITCH_CNT];

    for (i=0; i<mWinNum; i++)
    {
        mPlayerList[i] = NULL;
        mWidgetList[i] = new QWidget(this);
        mWidgetList[i]->setAutoFillBackground(true);
        mWidgetList[i]->setGeometry(winW * mLayoutAll.layout[id].win[i].x / IDS_LAYOUT_WIN_W
                                  , winH * mLayoutAll.layout[id].win[i].y / IDS_LAYOUT_WIN_H
                                  , winW * mLayoutAll.layout[id].win[i].w / IDS_LAYOUT_WIN_W
                                  , winH * mLayoutAll.layout[id].win[i].h / IDS_LAYOUT_WIN_H);
        mWidgetList[i]->show();

        // vid--------0:stitch.    1:linkage.     2~5:g0~g3.      6~69:d0~d63
        if (mLayoutAll.layout[id].win[i].vid == 0) // stitch
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
                   qDebug() << "ids_play_stream failed. stitch. vid: " << mLayoutAll.layout[id].win[i].vid;
            }
        }
        else if (mLayoutAll.layout[id].win[i].vid == 1)
        {
        }
        else if (mLayoutAll.layout[id].win[i].vid < 2+IPC_CFG_STITCH_CNT)
        {
            j = mLayoutAll.layout[id].win[i].vid-2;
            if (mIpcCfgAll.ipc_sti[j].url[0])
            {
                winfo[0].media_url = mIpcCfgAll.ipc_sti[j].url;
                winfo[0].win_w = this->mWidgetList[i]->width();
                winfo[0].win_h = this->mWidgetList[i]->height();
                winfo[0].win_id = GUINT_TO_POINTER(mWidgetList[i]->winId());
                winfo[0].flags = 0;
                ret = ids_play_stream(&winfo[0], 1, 0, NULL, this, &mPlayerList[i]);
                if (ret == 0)
                   qDebug() << "ids_play_stream failed. vid: " << mLayoutAll.layout[id].win[i].vid;
            }
        }
        else if (mLayoutAll.layout[id].win[i].vid < 2+IPC_CFG_STITCH_CNT+IPC_CFG_NORMAL_CNT)
        {
            j = mLayoutAll.layout[id].win[i].vid-2-IPC_CFG_STITCH_CNT;
            if (mIpcCfgAll.ipc[j].url[0])
            {
                winfo[0].media_url = mIpcCfgAll.ipc[j].url;
                winfo[0].win_w = this->mWidgetList[i]->width();
                winfo[0].win_h = this->mWidgetList[i]->height();
                winfo[0].win_id = GUINT_TO_POINTER(mWidgetList[i]->winId());
                winfo[0].flags = 0;
                ret = ids_play_stream(&winfo[0], 1, 0, NULL, this, &mPlayerList[i]);
                if (ret == 0)
                   qDebug() << "ids_play_stream failed. vid: " << mLayoutAll.layout[id].win[i].vid;
            }
        }
    }
}

void idsServer::chnConfigSlot(void)
{
    displayCfgDialog dispCfg;
    dispCfg.mIdsServerWin = this;
    dispCfg.getInfo(mIdsEndpoint);
    dispCfg.setGeometry(0, 0, 600, 400);
    dispCfg.show();
    dispCfg.exec();
}

void idsServer::netConfigSlot(void)
{
    NetCfgDialog netCfg;
    netCfg.update();
    netCfg.exec();
}

void idsServer::aboutSlot(void)
{
    QMessageBox::about(this, tr("关于"), tr("中心校准客户端V1.0   "));
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
        QProcess::execute("shutdown -h now");

    delete msgBox;
}

void idsServer::contextMenuEvent(QContextMenuEvent *e)
{
    static int flag = 0;
    if (flag == 1)
        return;

    flag = 1;
    this->getSceneList();
    mMainMenu->exec(e->globalPos());                    //选择菜单弹出的位置
    flag = 0;
}
