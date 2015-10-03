#ifndef STITCHDIALOG_H
#define STITCHDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include "app_amp.h"
namespace Ui {
class stitchDialog;
}

class stitchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit stitchDialog(QWidget *parent = 0);
    ~stitchDialog();
    int idsUpdate(gpointer);
    gpointer                    mIdsEndpoint;
    int                         mMsgRet;


private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_pushButtonBrow_clicked();

private:
    Ui::stitchDialog *ui;
};

#endif // STITCHDIALOG_H
