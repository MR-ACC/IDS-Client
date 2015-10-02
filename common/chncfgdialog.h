#ifndef CHNCFGDIALOG_H
#define CHNCFGDIALOG_H

#include <QDialog>
#include "app_amp.h"

namespace Ui {
class ChnCfgDialog;
}

class ChnCfgDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChnCfgDialog(QWidget *parent = 0);
    ~ChnCfgDialog();
    int                          idsUpdate(gpointer);
    gpointer                   mIdsEndpoint;
    int                             mMsgRet;
    IpcCfgAll                  mIpcCfgAll;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::ChnCfgDialog *ui;
};

#endif // CHNCFGDIALOG_H
