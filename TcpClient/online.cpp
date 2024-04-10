#include "online.h"
#include "ui_online.h"
#include "tcpclient.h"
Online::Online(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Online)
{
    ui->setupUi(this);
}

void Online::showUsrOnline(PDU *pdu)
{
    ui->listWidget->clear();
    char tmp[32];
    int msgl =pdu->uiMsgLen/32;
    for(int i=0;i<msgl;i++){
        memcpy(tmp,(char*)pdu->caMsg+i*32,32);
        ui->listWidget->addItem(tmp);
    }
}
Online::~Online()
{
    delete ui;
}

void Online::on_addfriend_clicked()
{
    QString toAddFriend=ui->listWidget->currentItem()->text();
    QString name = TcpClient::getInstance().getName();
    PDU *pdu=mkPDU(0);
    pdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    memcpy(pdu->caData,name.toStdString().c_str(),32);
    memcpy(pdu->caData+32,toAddFriend.toStdString().c_str(),32);
    TcpClient::getInstance().getSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

