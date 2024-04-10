#include "mytcpsocket.h"
#include "protocol.h"
#include "mytcpserver.h"
#include<QDebug>
#include"dao.h"
MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    connect(this,SIGNAL(readyRead()),this,SLOT(recvMsg()));
    connect(this,SIGNAL(disconnected()),this,SLOT(clientOffline()));
}

QString MyTcpSocket::getName()
{
    return m_name;
}

void MyTcpSocket::recvMsg()
{
    uint uipduLen=0;
    this->read((char *)&uipduLen,sizeof(uint));
    uint uiMsgLen=uipduLen-sizeof(PDU);
    PDU *pdu=mkPDU(uiMsgLen);
    this->read((char*)pdu+sizeof(uint),uipduLen-sizeof(uint));
    switch (pdu->uiMsgType){
    case ENUM_MSG_TYPE_REGIST_REQUEST:
    {
        char sName[32]={'\0'};
        char sPwd[32]={'\0'};
        memcpy(sName,pdu->caData,32);
        memcpy(sPwd,pdu->caData+32,32);
        qDebug()<<"name"<<":"<<sName<<"pwd"<<":"<<sPwd<<":";
        bool ret=DAO::getInstance().handleRegist(sName,sPwd);
        PDU *pde_rep=mkPDU(0);
        pde_rep->uiMsgType=ENUM_MSG_TYPE_REJIST_RESPONSE;
        if(ret){
            strcpy_s(pde_rep->caData,REGIST_0K);
        }else
        {
            strcpy_s(pde_rep->caData,REGIST_FAILED);
        };
        this->write((char*)pde_rep,pde_rep->uiPDULen);
        free(pde_rep);
        pde_rep=NULL;
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_REQUEST:
    {
        char sName[32]={'\0'};
        char sPwd[32]={'\0'};
        memcpy(sName,pdu->caData,32);
        memcpy(sPwd,pdu->caData+32,32);
        qDebug()<<"name"<<":"<<sName<<"pwd"<<":"<<sPwd<<":";
        bool ret=DAO::getInstance().handleLogin(sName,sPwd);
        PDU *pde_rep=mkPDU(0);
        pde_rep->uiMsgType=ENUM_MSG_TYPE_LOGIN_RESPONSE;

        if(ret){
            strcpy_s(pde_rep->caData,LOGIN_0K);
            m_name=sName;
        }else
        {
            strcpy_s(pde_rep->caData,LOGIN_FAILED);
        };
        this->write((char*)pde_rep,pde_rep->uiPDULen);
        free(pde_rep);
        pde_rep=NULL;
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
    {
        QStringList lists=DAO::getInstance().handleAllOnline();
        uint MsgLen=lists.size()*32;
        PDU *p=mkPDU(MsgLen);
        p->uiMsgType=ENUM_MSG_TYPE_ALL_ONLINE_RESPONSE;
        for(int i=0;i<lists.size();i++){
            memcpy((char *)p->caMsg+i*32,lists.at(i).toStdString().c_str(), lists.at(i).size());
        };
        write((char*)p,p->uiPDULen);
        free(p);
        p=NULL;
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_REQUEST:{
        char name[32]={'\0'};
        memcpy(name,pdu->caData,32);
        int isOnline=DAO::getInstance().handleSearch(name);
        PDU *p=mkPDU(0);
        p->uiMsgType=ENUM_MSG_TYPE_SEARCH_RESPONSE;
        if(-1 ==isOnline){
            strcpy(p->caData,"no such people");
        }else if(1==isOnline){
            strcpy(p->caData,"online");
        }else{
            strcpy(p->caData,"offline");
        }
        write((char*)p,p->uiPDULen);
        free(p);
        p=NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
    {   char name[32];
        char fname[32];
        memcpy(name,pdu->caData,32);
        memcpy(fname,pdu->caData+32,32);
        int ret=DAO::getInstance().handleAddFriend(name,fname);
        PDU *p=mkPDU(0);
        p->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPONSE;
        if(ret == -2){
            strcpy(p->caData,NOT_EXIST);
        }else if(ret == -3){
            strcpy(p->caData,IS_OFFLINE);
        }else if( ret ==0){
            strcpy(p->caData,ALREADY_FRIEND);
        }else if(ret ==-1){
            pdu->uiMsgType=ENUM_MSG_TYPE_RESEND_FRIEND_REQUEST;
            MyTcpServer::Getinstance().resend(fname,pdu);
            strcpy(p->caData,SEND_APPLY);
        }
        this->write((char*)p,p->uiPDULen);
        free(p);
        p=NULL;
        break;
    }
    case ENUM_MSG_TYPE_RESEND_FRIEND_RESPONSE:{
        char name[32]={'\0'};
        char fname[32]={'\0'};
        memcpy(name,pdu->caData,32);
        memcpy(fname,pdu->caData+32,32);
        DAO::getInstance().handleaddfriend(name,fname);
        PDU *p=mkPDU(0);
        p->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPONSE;
        strcpy(p->caData,ADD_SUCCESS);
        MyTcpServer::Getinstance().resend(name,p);
        break;
    };
    case ENUM_MSG_TYPE_RESEND_REFUSE_REQUEST:{
        char name[32]={'\0'};
        char fname[32]={'\0'};
        memcpy(name,pdu->caData,32);
        memcpy(fname,pdu->caData+32,32);
        DAO::getInstance().handleaddfriend(name,fname);
        PDU *p=mkPDU(0);
        p->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPONSE;
        strcpy(p->caData,REFUSE_ADD);
        MyTcpServer::Getinstance().resend(name,p);
        break;
    };
    case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:{
        char name[32]={'\0'};
        memcpy(name,pdu->caData,32);
        QStringList friendlist=DAO::getInstance().flushfriend(name);
        PDU *p=mkPDU(friendlist.size()*32);
        p->uiMsgType=ENUM_MSG_TYPE_FLUSH_FRIEND_RESPONSE;
        for(int i=0;i<friendlist.size();i++){
            memcpy((char*)(p->caMsg)+32*i,friendlist.at(i).toStdString().c_str(),friendlist.at(i).size());
        };
        this->write((char*)p,p->uiPDULen);
        free(p);
        p=NULL;
        break;
    };
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:{
        char name[32]={'\0'};
        char myname[32]={'\0'};
        memcpy(name,pdu->caData,32);
        memcpy(myname,pdu->caData+32,32);
        bool ret = DAO::getInstance().delFriend(name,myname);
        PDU *p=mkPDU(0);
        p->uiMsgType=ENUM_MSG_TYPE_DELETE_FRIEND_RESPONSE;
        strcpy(p->caData,DEL_SUCCESS);
        this->write((char*)p,p->uiPDULen);
        free(p);
        p=NULL;
        break;
    };

    case ENUM_MSG_TYPE_SEND_MESSAGE_REQUEST:{
        qDebug()<<"SEND_MESSAGE_REQUEST";
        char name[32]={'\0'};
        char fname[32]={'\0'};
        memcpy(name,pdu->caData,32);
        memcpy(fname,pdu->caData+32,32);
        pdu->uiMsgType=ENUM_MSG_TYPE_SEND_MESSAGE_RESPONSE;
        MyTcpServer::Getinstance().resend(fname,pdu);
        break;
    };
    };
    free(pdu);
    pdu=NULL;

};


void MyTcpSocket::clientOffline()
{
    DAO::getInstance().handleLoginout(m_name.toStdString().c_str());
    emit offline(this);

}
