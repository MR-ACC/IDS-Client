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

    gpointer            mIdsEndpoint;
    IdsLayoutAll        mLayoutAll;
    IpcCfgAll           mIpcCfgAll;
    DisplayModeInfo     mDispmodePreview;

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

private:
    static int disp_mode_cfg_event_handle(IdsEvent *pev, gpointer priv);
    void getSceneList(void);
    int cfg_id;

    Ui::idsServer *ui;
    QMenu              *mMainMenu;                      /*popupMenu*/  //主菜单
    QMenu              *mSceneSwitchMenu;               /*switchScene*/  // 二级菜单
    QAction             *mChnCfg;                                  /*Cfg*/
    QAction             *mLayoutCfg;                             /*Cfg*/
    QAction             *mDispmodeCfg;                       /*Cfg*/
    QAction             *mNetCfg;                                  /*net*/
    QAction             *mAbout;                                        /*about*/
    QAction             *mExit;

    int                      mSceneId;
    int                      mSceneNum;
    QActionGroup  *mSceneGroup;                    //用来实现子菜单选项互斥
    QAction             *mSceneList[IDS_LAYOUT_MAX_NUM];

    int                      mWinNum;
    IdsPlayer           *mPlayerList[IDS_LAYOUT_WIN_MAX_NUM];
    QWidget            *mWidgetList[IDS_LAYOUT_WIN_MAX_NUM];

    bool                    mDispmodePreviewFlag;
};

#endif // IDSSERVER_H
