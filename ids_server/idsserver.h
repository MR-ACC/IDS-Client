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
#include "ids.h"
#include "../common/displaycfgdialog.h"
#include "../common/netcfgdialog.h"
#include "app_amp.h"

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
    IpcCfgAll                  mIpcCfgAll;
    IdsLayoutAll            mLayoutAll;
    DisplayModeInfo    mDispmodePreview;
    bool                          mDispmodePreviewFlag;

signals:
    void idsPlayerStart();
    void idsPlayerStop();

public slots:
    void idsPlayerStartSlot(void);
    void idsPlayerStopSlot(void);
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
    QWidget            *mWidgetList[IDS_LAYOUT_WIN_MAX_NUM];
    char                    mPlayerStatus[IDS_LAYOUT_WIN_MAX_NUM][256];

    QMutex              mMutex;
};

#endif // IDSSERVER_H
