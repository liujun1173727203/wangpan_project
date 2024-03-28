#include "mytcpsocket.h"
#include "protocol.h"
#include<QDebug>
MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    connect(this,SIGNAL(readyRead()),this,SLOT(recvMsg()));
}

void MyTcpSocket::recvMsg()
{
    qDebug()<<this->bytesAvailable();
    uint uipduLen=0;
    this->read((char *)&uipduLen,sizeof(uint));
    uint uiMsgLen=uipduLen-sizeof(PDU);
    PDU *pdu=mkPDU(uiMsgLen);
    this->read((char*)pdu+sizeof(uint),uipduLen-sizeof(uint));
    qDebug()<<pdu->uiMsgType<<":"<<pdu->uiMsgLen<<":"<<(char*)(pdu->caMsg);
    free(pdu);
}
