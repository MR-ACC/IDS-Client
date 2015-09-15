#include "idsclient.h"
#include "ui_idsclient.h"

idsclient::idsclient(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::idsclient)
{
    ui->setupUi(this);
}

idsclient::~idsclient()
{
    delete ui;
}
