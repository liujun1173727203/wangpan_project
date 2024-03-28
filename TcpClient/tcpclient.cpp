#include "tcpclient.h"
#include "protocol.h"
#include "./ui_tcpclient.h"
#include <QDebug>
#include<QHostAddress>
#include<QMessageBox>
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
}

TcpClient::~TcpClient()
{
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
void TcpClient::showConnect(){
    QMessageBox::information(this,"连接服务器","连接服务器成功");
}

void TcpClient::on_send_clicked()
{
    QString  strMsg =ui->lineEdit->text();
    //类型强转后，编译器无法确定输出何时结束，找不到结束符。会导致输出？？？？
    strMsg+='\0';
    if(strMsg.isEmpty()){
        return;
    }
    qDebug()<<strMsg.length()<<strMsg.size()<<sizeof(strMsg)<<(strMsg.toStdString().c_str());
    // PDU *pdu=mkPDU(sizeof(strMsg));
    PDU *pdu=mkPDU(strMsg.length());
    pdu->uiMsgType=1111;
    memcpy(pdu->caMsg,strMsg.toStdString().c_str(),strMsg.size());
    qDebug()<<"client"<<pdu->uiPDULen<<":"<<(char*)pdu->caMsg;
    m_socket.write( (char *)pdu,pdu->uiPDULen);
    free(pdu);
}

