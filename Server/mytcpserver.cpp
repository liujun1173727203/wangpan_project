#include "mytcpserver.h"
#include<QDebug>
void MyTcpServer::deletesocket(MyTcpSocket *socket)
{
    QList<MyTcpSocket*>::Iterator iter=sockelist.begin();
    for(;iter !=sockelist.end();iter++){
        if(*iter ==socket){
            socket->deleteLater();
            *iter=NULL;
            sockelist.erase(iter);
            break;
        }
    }
    qDebug()<<"disconnect";
};

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
    connect(psocket,SIGNAL(offline(MyTcpSocket*)),this,SLOT(deletesocket(MyTcpSocket*)));

}

void MyTcpServer::resend(const char *name, PDU *pdu)
{
    for(int i=0;i<sockelist.size();i++){
        if(name ==sockelist.at(i)->getName()){
            qDebug()<<sockelist.at(i)->getName();
            qDebug()<< sockelist.at(i)->bytesToWrite();
            int ret =sockelist.at(i)->write((char *)pdu,pdu->uiPDULen);
            qDebug()<< sockelist.at(i)->bytesToWrite();
            qDebug()<<ret;
            if(ret <0){
                qDebug()<<"write error ";
            }
            break;
        }
    }
}
