#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QString>
#include <QTimer>
#include <QFileDialog>
#include <QCryptographicHash>

#define MAX_CAMERA_NUM 128

namespace Ui {
    class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0, QString ip = "192.168.1.2");
    ~Dialog();

private:
    Ui::Dialog *ui;
    int num;
    int ipnum;
    int status[MAX_CAMERA_NUM];
    bool interactiveFinish;
    int interactiveFinishNum;
    QString log[MAX_CAMERA_NUM];
    QTimer *timer;
    QString time_status;

private slots:
	void on_pushButtonSend_clicked();
	void on_pushButtonBrow_clicked();
    void updateStatusLabel(const QString& status);
    void send_slot(int i);
    void time_slot(int i);
    void Update_Slot();
};

#endif // DIALOG_H
