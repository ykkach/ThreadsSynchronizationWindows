#include "threadssync.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ThreadsSync w;
    w.show();
    return a.exec();
}
