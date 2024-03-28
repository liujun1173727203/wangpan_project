#include "tcpserver.h"
#include "./ui_tcpserver.h"
#include "mytcpserver.h"
#include <QFile>
#include<QDebug>
TcpServer::TcpServer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);
    loadConfig();
    MyTcpServer::Getinstance().listen(QHostAddress(this->m_strIP),this->m_usPort);
}

TcpServer::~TcpServer()
{
    delete ui;
}
void TcpServer::loadConfig()
{
    QFile file(":/client.config");
    if(file.open(QIODevice::ReadOnly)){
        QByteArray baData=file.readAll();
        QString strData=baData.toStdString().c_str();
        QStringList ip_port=strData.split("\r\n");
        this->m_strIP=ip_port[0];
        this->m_usPort=ip_port[1].toUShort();

    }

}
