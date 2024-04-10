#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H
#include"protocol.h"
#include <QTcpServer>
#include "mytcpsocket.h"
#include<QList>
//要使用槽 必须包括Q_OBJECT宏和继承QOBJECT基类，QTcpServer 继承了QOBJect基类
class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public slots:
           void deletesocket(MyTcpSocket *socket);
public:
    MyTcpServer();
    static MyTcpServer &Getinstance();
    void incomingConnection(qintptr socketDesc);
    void resend(const char* name,PDU *pdu);
private:
    QList<MyTcpSocket*> sockelist;
};

#endif // MYTCPSERVER_H
