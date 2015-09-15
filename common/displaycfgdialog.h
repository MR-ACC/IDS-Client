#ifndef DISPLAYCFGDIALOG_H
#define DISPLAYCFGDIALOG_H

#include <QDialog>
#include <ids.h>

#define MAX_MODE_NUMBERS 20
#define MAX_RES_NUMBERS 20
static int mode_v_cnt[MAX_MODE_NUMBERS] = {1, 1, 1, 1, 2, 2, 2, 0};
static int mode_h_cnt[MAX_MODE_NUMBERS] = {1, 2, 3, 4, 2, 3, 4, 0};

namespace Ui {
class displayCfgDialog;
}

class displayCfgDialog : public QDialog
{
    Q_OBJECT

public:
    explicit displayCfgDialog(QWidget *parent = 0);
    ~displayCfgDialog();

    void getInfo(gpointer);
    gpointer            mIdsEndpoint;
    QWidget            *mIdsServerWin;
    DisplayModeInfo     mDispMode;
    MonitorInfos        mMonitorInfos;
    int                 mDispModeRet;

protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent *);

private slots:
    void accept();
    void reject();
    void modeResChanged();

    void on_pushButton_preview_clicked();

private:

    int mModeCnt, mResCnt;
    MonitorResInfo mResAll[MAX_RES_NUMBERS];

    int mModeId, mResId;
    int mMonitorValid[MAX_MONITOR_NUMBERS];
    int mMonitorId[MAX_MONITOR_NUMBERS];

    Ui::displayCfgDialog *ui;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *);
};

#endif // DISPLAYCFGDIALOG_H
