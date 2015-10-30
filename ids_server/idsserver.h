#ifndef IDSSERVER_H
#define IDSSERVER_H

#include <QtGui>
#include <QWidget>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QDebug>
#include <QProcess>
#include <QNetworkInterface>
#include <QFrame>
#include <QColorDialog>
#include <QDesktopWidget>
#include <QThread>
#include <QTimer>
#include "videowidget.h"
#include "../common/idsutil.h"

namespace Ui {
class idsServer;
}

class idsServer : public QWidget
{
    Q_OBJECT

public:
    explicit idsServer(QWidget *parent = 0);
    void contextMenuEvent(QContextMenuEvent *);
    ~idsServer();

    gpointer                   mIdsEndpoint;
    int                             mMsgRet;
    IpcCfgAll                  mIpcCfgAll;
    IdsLayoutAll            mLayoutAll;
    IdsServerInfo          mServerInfo;
    DisplayModeInfo    mDispmodePreview;
    bool                          mDispmodePreviewFlag;

signals:
    void idsPlayerRestart();
    void idsPlayerHide();
    void idsPlayerShow();

private slots:
    void idsPlayerRestartSlot(void);
    void idsPlayerHideSlot(void);
    void idsPlayerShowSlot(void);
    void cursorHideSlot(void);
    void idsReconnectSlot(void);
    void sceneSwitchSlot(void);
    void chnCfgSlot(void);
    void layoutCfgSlot(void);
    void dispmodeCfgSlot(void);
    void netCfgSlot(void);
    void aboutSlot(void);
    void rebootSlot(void);
    void shutdownSlot(void);

protected:
    void paintEvent(QPaintEvent*);
    void closeEvent(QCloseEvent *);

private:
    void idsRefresh(void);
    void idsPlayerStart(void);
    void idsPlayerStop(void);
    void idsSceneListRelease(void);
    void idsSceneListCreate(void);
    Ui::idsServer       *ui;
    QMenu               *mMainMenu;                      //主菜单
    QMenu               *mSceneSwitchMenu;               //二级菜单
    QAction             *mReconnect;
    QAction             *mChnCfg;
    QAction             *mLayoutCfg;
    QAction             *mDispmodeCfg;
    QAction             *mNetCfg;
    QAction             *mAbout;
    QAction             *mReboot;
    QAction             *mShutdown;

    QActionGroup        *mSceneGroup;                    //用来实现子菜单选项互斥
    QAction             *mSceneList[IDS_LAYOUT_MAX_NUM];
    int                 mSceneNum;

    int                 mWinNum;
    VideoWidget         *mWidgetList[IDS_LAYOUT_WIN_MAX_NUM];

    QMutex              mMutex;

    QTimer              mCursorTimer;
    gint                mCursorStatus;
};

#endif // IDSSERVER_H
