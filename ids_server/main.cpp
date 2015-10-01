#include "idsserver.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFont font;
    font.setPointSize(14);
    a.setFont(font);

    idsServer w;
    w.show();

    return a.exec();
}
