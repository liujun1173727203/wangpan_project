#include <QApplication>
#include"tcpclient.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TcpClient::getInstance().show();
    // qDebug()<<QString("./aa/bb/c").mid(8);
    return a.exec();
}
