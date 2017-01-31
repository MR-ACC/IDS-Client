#include "idsclient.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    idsclient w;
    w.setWindowFlags(w.windowFlags()& ~Qt::WindowMaximizeButtonHint);
    w.setWindowFlags(w.windowFlags() & Qt::WindowMaximized);
    //w.show();
    //w.showMaximized();
    w.showFullScreen();

    return a.exec();
}
