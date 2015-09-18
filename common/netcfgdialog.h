#ifndef NETCFGDIALOG_H
#define NETCFGDIALOG_H

#include <QDialog>
#include "ids.h"

namespace Ui {
class NetCfgDialog;
}

class NetCfgDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NetCfgDialog(QWidget *parent = 0);
    ~NetCfgDialog();
    int update(gpointer  endpoint);
    NetInfo mNetInfo;
private:
    Ui::NetCfgDialog *ui;
    gpointer mIdsEndpoint;
private slots:
    void netcfgAccept();
    void netcfgReject();
};

#endif // NETCFGDIALOG_H
