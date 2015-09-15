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
    bool                mDispModePreviewFlag;
    DisplayModeInfo     mDispModePreview;

public slots:
    void sceneSwitchSlot(void);
    void chnConfigSlot(void);
    void netConfigSlot(void);
    void aboutSlot(void);
    void exitSlot(void);

protected:
    void paintEvent(QPaintEvent*);

private:
    void getSceneList(void);
    void sceneStopPlay(void);
    int cfg_id;

    Ui::idsServer *ui;
    QMenu               *mMainMenu;                      /*popupMenu*/  //主菜单
    QMenu               *mSceneSwitchMenu;               /*switchScene*/  // 二级菜单
    QAction             *mChnConfig;                     /*config*/
    QAction             *mNetConfig;                     /*net*/
    QAction             *mAbout;                         /*about*/
    QAction             *mExit;

    int                 mSceneNum;
    QActionGroup        *mSceneGroup;                    //用来实现子菜单选项互斥
    QAction             *mSceneList[IDS_LAYOUT_MAX_NUM];

    int                 mWinNum;
    IdsPlayer           *mPlayerList[IDS_LAYOUT_WIN_MAX_NUM];
    QWidget             *mWidgetList[IDS_LAYOUT_WIN_MAX_NUM];
};

#endif // IDSSERVER_H
