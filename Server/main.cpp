#include "tcpserver.h"

#include <QApplication>
#include <QDir>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TcpServer w;
    w.show();
    // QDir dir;
    // int ret =dir.mkdir("./xiaoer");
    // qDebug()<<dir.currentPath();
    return a.exec();
}
