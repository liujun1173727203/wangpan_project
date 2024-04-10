#include "tcpclient.h"
#include "protocol.h"
#include "./ui_tcpclient.h"
#include <QDebug>
#include<QHostAddress>
#include<QMessageBox>
#include "opewidget.h"
TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    loadConfig();
    //关联信号处理函数
    //发送方，发送什么信号,谁接受，谁处理
    connect(&m_socket,SIGNAL(connected()),this,SLOT(showConnect()));
    m_socket.connectToHost(QHostAddress(m_strIP),m_usPort);
    connect(&m_socket,SIGNAL(readyRead()),this,SLOT(recvMsg()));
}

TcpClient::~TcpClient()
{
     m_socket.disconnectFromHost();
    delete ui;
}

void TcpClient::loadConfig()
{
    QFile file(":/client.config");
    if(file.open(QIODevice::ReadOnly)){
        QByteArray baData=file.readAll();
        QString strData=baData.toStdString().c_str();
        QStringList ip_port=strData.split("\r\n");
        this->m_strIP=ip_port[0];
        this->m_usPort=ip_port[1].toUShort();
        // qDebug()<<this->m_strIP<<this->m_usPort;
    }

}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getSocket()
{
    return m_socket;
}

QString TcpClient::getName()
{
    return m_cName;
}

void TcpClient::showConnect(){
    // QMessageBox::information(this,"连接服务器","连接服务器成功");
}

void TcpClient::recvMsg()
{
    qDebug()<<"recv PDU";
    uint uipduLen=0;
    m_socket.read((char *)&uipduLen,sizeof(uint));
    uint uiMsgLen=uipduLen-sizeof(PDU);
    PDU *pdu=mkPDU(uiMsgLen);
    m_socket.read((char*)pdu+sizeof(uint),uipduLen-sizeof(uint));
    switch (pdu->uiMsgType){
    case ENUM_MSG_TYPE_REJIST_RESPONSE:
    {
        if(0==strcmp(pdu->caData,REGIST_0K)){
            QMessageBox::information(this,"regist","regist success");
        }else if(0==strcmp(pdu->caData,REGIST_FAILED)){
            QMessageBox::information(this,"regist",REGIST_FAILED);
        }
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_RESPONSE:
    {
        if(0==strcmp(pdu->caData,LOGIN_0K)){
            QMessageBox::information(this,"login",LOGIN_0K);
            this->hide();
            OpeWidget::getInstance().show();
        }else if(0==strcmp(pdu->caData,LOGIN_FAILED)){
            QMessageBox::information(this,"login",LOGIN_FAILED);
        }
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_RESPONSE:{
        OpeWidget::getInstance().getFriend()->setUsrOnline(pdu);
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_RESPONSE:{
        QMessageBox::information(this,"Search",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_RESPONSE:
    {
        QMessageBox::information(this,"添加好友",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_RESEND_FRIEND_REQUEST:{
        int ret =QMessageBox::information(NULL,"添加好友",pdu->caData,QMessageBox::Yes,QMessageBox::No);
        PDU *p=mkPDU(0);
        if(ret == QMessageBox::Yes){
            pdu->uiMsgType=ENUM_MSG_TYPE_RESEND_FRIEND_RESPONSE;
            m_socket.write((char*)pdu,pdu->uiPDULen);
        }else{
            pdu->uiMsgType=ENUM_MSG_TYPE_RESEND_REFUSE_REQUEST;
            m_socket.write((char*)pdu,pdu->uiPDULen);
        }
        m_socket.write((char*) p,p->uiPDULen);
        free(p);
        p=NULL;
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPONSE:{
        OpeWidget::getInstance().getFriend()->showfriend(pdu);
        break;
    };
    case ENUM_MSG_TYPE_DELETE_FRIEND_RESPONSE:{
        QMessageBox::information(this,"删除好友",pdu->caData);
        break;
    };
    case ENUM_MSG_TYPE_SEND_MESSAGE_RESPONSE:{
        qDebug()<<"ENUM_MSG_TYPE_SEND_MESSAGE_RESPONSE";
        char name[32]={'\0'};   //sender
        char fname[32]={'\0'};  //self
        memcpy(name,pdu->caData,32);
        memcpy(fname,pdu->caData+32,32);
        QMap<QString,P2pchat *> *chatlist=&Friend::chatlist;
        qDebug()<<&Friend::getChatList();
        QMap<QString,P2pchat *>::Iterator itor;
        for(itor=chatlist->begin();itor !=chatlist->end();itor++){
            qDebug()<<itor.key();
        }
        if(!chatlist->contains(QString(name))){
            P2pchat *p=new P2pchat;
            p->setChatName(name);
            p->setmyName(fname);
            p->setWindowTitle(name);
            chatlist->insert(name,p);
        };

        chatlist->value(name)->recvChatMsg((char*)pdu->caMsg);
        if(chatlist->value(name)->isHidden()){
            chatlist->value(name)->show();
        };
        break;
    };
    };
    free(pdu);
    pdu=NULL;
}

void TcpClient::on_login_clicked()
{
    QString strName =ui->name_edit->text();
    m_cName=strName;
    QString strpwd =ui->pwd_edit->text();
    if(strpwd.isEmpty()||strName.isEmpty()){
        QMessageBox::critical(this,"login","login fail:pwd or username is empty");
    }
    PDU *pdu=mkPDU(0);
    pdu->uiMsgType=ENUM_MSG_TYPE_LOGIN_REQUEST;
    memcpy(pdu->caData,strName.toStdString().c_str(),32);
    memcpy(pdu->caData+32,strpwd.toStdString().c_str(),32);
    m_socket.write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}


void TcpClient::on_regist_clicked()
{
    QString strName =ui->name_edit->text();
    QString strpwd =ui->pwd_edit->text();
    if(strpwd.length()<=0){
        QMessageBox::critical(this,"regist","regist fail:pwd is empty");
    }
    PDU *pdu=mkPDU(0);
    pdu->uiMsgType=ENUM_MSG_TYPE_REGIST_REQUEST;
    memcpy(pdu->caData,strName.toStdString().c_str(),32);
    memcpy(pdu->caData+32,strpwd.toStdString().c_str(),32);
    m_socket.write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}


void TcpClient::on_loginout_clicked()
{

}

