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
    isUpload=false;
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

void TcpClient::setCurPath(const char *path)
{
    this->m_sCurPath=path;
}

QString TcpClient::getCurPath()
{
    return m_sCurPath;
}

void TcpClient::setSavePath(const QString path)
{
    this->m_savepath=path;
}

void TcpClient::showConnect(){
    // QMessageBox::information(this,"连接服务器","连接服务器成功");
}

void TcpClient::recvMsg()
{
    if(!isUpload){
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
                // QMessageBox::information(this,"login",LOGIN_0K);
                this->hide();
                this->m_sCurPath=QString("./%1").arg(m_cName);
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
        case ENUM_MSG_TYPE_DEL_DIR_RESPONSE:{
            QMessageBox::information(this,"删除文件夹",pdu->caData);
            // OpeWidget::getInstance().getBook()->flushDir();
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
        case ENUM_MSG_TYPE_CREATE_DIR_RESPONSE:{
            QMessageBox::information(this,"创建文件",pdu->caData);
            break;
        };
        case ENUM_MSG_TYPE_CREATE_FLUSH_RESPONSE:{
            qDebug()<<"ENUM_MSG_TYPE_CREATE_FLUSH_RESPONSE";
            char name[32]={'\0'};
            memcpy(name,pdu->caData,32);
            if(QString(name).length()!=0){
            QString root=TcpClient::getInstance().getCurPath();
            QString path=QString("%1/%2").arg(root).arg(name);
            TcpClient::getInstance().setCurPath(path.toStdString().c_str());
            qDebug()<<TcpClient::getInstance().getCurPath();
            };
            OpeWidget::getInstance().getBook()->updateFileList(pdu);
            break;
        }
        case ENUM_MSG_TYPE_RENAME_DIR_RESPONSE:{
            QMessageBox::information(this,"重命名文件夹",pdu->caData);
            OpeWidget::getInstance().getBook()->flushDir();
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_RESPONSE:{
             qDebug()<<"ENUM_MSG_TYPE_ENTER_DIR_RESPONSE";
            char state[32]={'\0'};
            memcpy(state,pdu->caData,32);
            qDebug()<<(QString(state).compare(DIR_EXIST));
            if(QString(state).compare(DIR_EXIST)){
                qDebug()<<(char*)(pdu->caMsg);
            }else{
                QMessageBox::information(this,"进入文件",pdu->caData);
            };
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_RESPONSE:{
            QMessageBox::information(this,"上传文件",pdu->caData);
            OpeWidget::getInstance().getBook()->flushDir();
            break;
        };
        case ENUM_MSG_TYPE_DEL_FILE_RESPONSE:{
            QMessageBox::information(this,"删除文件",pdu->caData);
            OpeWidget::getInstance().getBook()->flushDir();
            break;
        };
        case ENUM_MSG_TYPE_DOWNLOAD_RESPONSE:{
            char filename[32]={'\0'};
            qint64 filesize;
            sscanf(pdu->caData,"%s %lld",filename,&filesize);
            QString path=QString("%1/%2").arg(m_savepath).arg(filename);
            mfile.setFileName(path);
            if(mfile.open(QIODevice::WriteOnly)){
                isUpload=true;
                mTotal=filesize;
                m_iRecved=0;
            };
            break;
        };
        case ENUM_MSG_TYPE_SHARE_RESPONSE:{
            QMessageBox::information(this,"分享文件",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_RESEND_REQUEST:{
            char sendName[32]={"\0"};
            char *path=new char[pdu->uiMsgLen];
            memcpy(path,(char*)pdu->caMsg,pdu->uiMsgLen);
            memcpy(sendName,pdu->caData,32);
            qDebug()<<QString(path).left(pdu->uiMsgLen);
            QString pat=QString(path).left(pdu->uiMsgLen);
            int index =pat.lastIndexOf("/");
            QString fileName=pat.mid(index);
            QString info=QString("%1 share file ---%2").arg(sendName).arg(fileName);
            int ret=QMessageBox::question(this,"共享文件",info);
            if(ret == QMessageBox::Yes){
                PDU *p=mkPDU(pdu->uiMsgLen);
                memcpy((char*)p->caMsg,pdu->caMsg,pdu->uiMsgLen);
                p->uiMsgType=ENUM_MSG_TYPE_SHARE_RESEND_RESPONSE;
                QString name=TcpClient::getInstance().getName();
                memcpy(p->caData,name.toStdString().c_str(),name.size());
                TcpClient::getInstance().getSocket().write((char*)p,p->uiPDULen);
            }  
            break;
        }
        case ENUM_MSG_TYPE_SHARE_RESEND_RESPONSE:{
            OpeWidget::getInstance().getBook()->flushDir();
            break;

        }
        case ENUM_MSG_TYPE_MOVE_RESPONSE:{
           QMessageBox::information(this,"移动文件",pdu->caData);
            break;
        }
        };
        free(pdu);
        pdu=NULL;
    }else{
        QByteArray buff=m_socket.readAll();
        mfile.write(buff);
        m_iRecved+=buff.size();
        if(m_iRecved ==mTotal){
            isUpload=false;
            mfile.close();
           QMessageBox::information(this,"下载文件","下载成功");
        }else if(m_iRecved >mTotal){
            isUpload=false;
            mfile.close();
            QMessageBox::warning(this,"下载文件","下载出错");
        };

    }
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

