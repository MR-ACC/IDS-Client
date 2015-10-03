#ifndef DISPLAYCFGDIALOG_H
#define DISPLAYCFGDIALOG_H

#include <QDialog>
#include <ids.h>

#define MAX_RES_NUMBERS 20

namespace Ui {
class displayCfgDialog;
}

class displayCfgDialog : public QDialog
{
    Q_OBJECT

public:
    explicit displayCfgDialog(QWidget *parent = 0);
    ~displayCfgDialog();

    int idsUpdate(gpointer);
    gpointer                    mIdsEndpoint;
    DisplayModeInfo     mDispMode;
    MonitorInfos            mMonitorInfos;
    int                              mMsgRet;

protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent *);

private slots:
    void accept();
    void reject();
    void modeResChanged();

private:

    int mModeCnt, mResCnt;
    MonitorResInfo mResAll[MAX_RES_NUMBERS];

    int mModeId, mResId;
    int mMonitorValid[MAX_MONITOR_NUMBERS];
    int mMonitorId[MAX_MONITOR_NUMBERS];

    Ui::displayCfgDialog *ui;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event);
};

#endif // DISPLAYCFGDIALOG_H
