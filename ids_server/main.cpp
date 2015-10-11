#include "idsserver.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFont font;
    font.setPointSize(14);
    a.setFont(font);

    ids_set_cmdline(argc, argv);

    idsServer w;
    w.show();

    return a.exec();
}
