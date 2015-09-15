#ifndef IDSCLIENT_H
#define IDSCLIENT_H

#include <QMainWindow>

namespace Ui {
class idsclient;
}

class idsclient : public QMainWindow
{
    Q_OBJECT

public:
    explicit idsclient(QWidget *parent = 0);
    ~idsclient();

private:
    Ui::idsclient *ui;
};

#endif // IDSCLIENT_H
