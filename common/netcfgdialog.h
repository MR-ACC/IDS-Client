#ifndef NETCFGDIALOG_H
#define NETCFGDIALOG_H

#include <QDialog>

namespace Ui {
class NetCfgDialog;
}

class NetCfgDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NetCfgDialog(QWidget *parent = 0);
    ~NetCfgDialog();
    void update();
private:
    QString netname;
    Ui::NetCfgDialog *ui;

private slots:
    void netcfgAccept();
    void netcfgReject();
};

#endif // NETCFGDIALOG_H
