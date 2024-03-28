#include "mytcpserver.h"
#include<QDebug>
MyTcpServer::MyTcpServer() {}

MyTcpServer &MyTcpServer::Getinstance(){
    static MyTcpServer instance;
    return instance;
}
void MyTcpServer::incomingConnection(qintptr socketdesc){
    qDebug()<<"new client connected";
    MyTcpSocket *psocket=new MyTcpSocket();
    psocket->setSocketDescriptor(socketdesc);
    sockelist.append(psocket);
}
