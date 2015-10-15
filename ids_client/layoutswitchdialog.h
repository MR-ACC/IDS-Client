#ifndef LAYOUTSWITCHDIALOG_H
#define LAYOUTSWITCHDIALOG_H

#include <QDialog>
#include "app_amp.h"
#include "../common/idsutil.h"
namespace Ui {
class layoutSwitchDialog;
}

class layoutSwitchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit layoutSwitchDialog(QWidget *parent = 0);
    ~layoutSwitchDialog();
    int idsUpdate(gpointer endpoint);
    gpointer                    mIdsEndpoint;
    IdsLayoutAll                mlayout;
    int                         mMsgRet;
private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::layoutSwitchDialog *ui;
};

#endif // LAYOUTSWITCHDIALOG_H
