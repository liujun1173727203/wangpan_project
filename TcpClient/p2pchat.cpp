#include "p2pchat.h"
#include "ui_p2pchat.h"
#include "QMessageBox"
#include "tcpclient.h"
#include<QDateTime>
P2pchat::P2pchat(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::P2pchat)
{
    ui->setupUi(this);
    this->myName=TcpClient::getInstance().getName();
}

P2pchat::~P2pchat()
{
    delete ui;
}


void P2pchat::setChatName(QString name)
{
    this->chatName=name;
}

void P2pchat::setmyName(QString name)
{
    this->myName=name;
}

void P2pchat::recvChatMsg(const char *msg)
{

    ui->msg_show->addItem(this->chatName+" "+msg);
}

void P2pchat::on_sendmsg_clicked()
{
    QDateTime dateTime= QDateTime::currentDateTime();
    QString str = dateTime .toString("MM-dd hh:mm");
    QString msg=ui->msg_input->text();
    if(msg.length() ==0){
        QMessageBox::warning(this, "发送消息", "消息不能为空");
    };
    msg =str+" : "+msg;
    PDU *pdu=mkPDU(sizeof(msg));
    qDebug()<<"ENUM_MSG_TYPE_SEND_MESSAGE_REQUEST";
    pdu->uiMsgType=ENUM_MSG_TYPE_SEND_MESSAGE_REQUEST;
    memcpy(pdu->caData,this->myName.toStdString().c_str(),32);
    memcpy(pdu->caData+32,this->chatName.toStdString().c_str(),32);
    memcpy(pdu->caMsg,msg.toStdString().c_str(),msg.length());
    TcpClient::getInstance().getSocket().write((char*)pdu,pdu->uiPDULen);

    QString send_msg_show =this->myName+" "+msg;
    ui->msg_show->addItem(send_msg_show);
    ui->msg_input->clear();
};

