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
#include "videowidget.h"
#include "app_amp.h"

class PlayThread : public QThread
{
     Q_OBJECT
protected:
     void run();
public:
     void *mPriv;
     int mPlayerid;
};

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
    DisplayModeInfo    mDispmodePreview;
    bool                          mDispmodePreviewFlag;

signals:
    void idsPlayerStartOne(int);
    void idsPlayerStart();
    void idsPlayerStop();
    void idsPlayerHide();
    void idsPlayerShow();

public slots:
    void idsPlayerStartOneSlot(int);
    void idsPlayerStartSlot(void);
    void idsPlayerStopSlot(void);
    void idsPlayerHideSlot(void);
    void idsPlayerShowSlot(void);
    void sceneSwitchSlot(void);
    void chnCfgSlot(void);
    void layoutCfgSlot(void);
    void dispmodeCfgSlot(void);
    void netCfgSlot(void);
    void aboutSlot(void);
    void exitSlot(void);

protected:
    void paintEvent(QPaintEvent*);
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent *);

private:
    void newSceneList(void);
    void deleteSceneList(void);

    Ui::idsServer     *ui;
    QMenu              *mMainMenu;                          //主菜单
    QMenu              *mSceneSwitchMenu;             //二级菜单
    QAction             *mChnCfg;
    QAction             *mLayoutCfg;
    QAction             *mDispmodeCfg;
    QAction             *mNetCfg;
    QAction             *mAbout;
    QAction             *mExit;

    QActionGroup  *mSceneGroup;                    //用来实现子菜单选项互斥
    QAction             *mSceneList[IDS_LAYOUT_MAX_NUM];
    int                       mSceneNum;
    int                       mSceneId;

    int                       mWinNum;
    int                       mWinIdStitch, mWinIdLink;
    IdsPlayer           *mPlayerList[IDS_LAYOUT_WIN_MAX_NUM];
    VideoWidget        *mWidgetList[IDS_LAYOUT_WIN_MAX_NUM];
    PlayThread        mPlayThread[IDS_LAYOUT_WIN_MAX_NUM];
    QMutex              mPlayMutex[IDS_LAYOUT_WIN_MAX_NUM];

    QMutex              mMutex;
};

#endif // IDSSERVER_H
