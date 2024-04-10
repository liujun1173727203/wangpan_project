#include "friend.h"
#include<QInputDialog>
#include<QMessageBox>
Friend ::Friend(QWidget *parent)
    : QWidget{parent}
{
    m_pShowMsgTe= new QTextEdit();
    m_pFriendListwidget= new QListWidget();
    m_pInputMsgLe=new QLineEdit();

    m_pDelFriendPB =new QPushButton("删除好友");
    m_pFlushFriendPB =new QPushButton("刷新好友列表");
    m_pShowOnlineUsrPB=new QPushButton("显示在线用户");
    m_PSearchUsrPB =new QPushButton("查找用户");
    m_pMsgSendPB =new QPushButton("发送");
    m_pPrivateChatPB =new QPushButton("私聊");
    QVBoxLayout  *pLeftPBVBL =new QVBoxLayout;
    pLeftPBVBL->addWidget(m_pDelFriendPB);
    pLeftPBVBL->addWidget(m_pFlushFriendPB);
    pLeftPBVBL->addWidget(m_PSearchUsrPB);
    pLeftPBVBL->addWidget(m_pShowOnlineUsrPB);
    pLeftPBVBL->addWidget(m_pMsgSendPB);
    pLeftPBVBL->addWidget(m_pPrivateChatPB);

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pShowMsgTe);
    pTopHBL->addWidget(m_pFriendListwidget);
    pTopHBL->addLayout(pLeftPBVBL);

    QHBoxLayout *pMsgHBL =new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLe);
    pMsgHBL->addWidget(m_pMsgSendPB);

    online=new Online;
    QVBoxLayout *pMain=new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(online);
    online->hide();
    setLayout(pMain);
    connect(m_pShowOnlineUsrPB,SIGNAL(clicked(bool)),this,SLOT(showOnline()));
    connect(m_PSearchUsrPB,SIGNAL(clicked(bool)),this,SLOT(searchOnlinefriend()));
    connect(m_pFlushFriendPB,SIGNAL(clicked(bool)),this,SLOT(flushfriend()));
    connect(m_pDelFriendPB,SIGNAL(clicked(bool)),this,SLOT(delfriend()));
        connect(m_pPrivateChatPB,SIGNAL(clicked(bool)),this,SLOT(chatFriend()));
}

void Friend::setUsrOnline(PDU *pdu)
{
    online->showUsrOnline(pdu);
}



void Friend::showOnline()
{
    if(online->isHidden()){
        PDU *pdu=mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::getInstance().getSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
        online->show();
    }else{
        online->hide();
    }
}


void Friend::searchOnlinefriend()
{
    QString name=QInputDialog::getText(this,"Search","用户名：");
    if(name.length() == 0)return;
    PDU *pdu=mkPDU(0);
    pdu->uiMsgType=ENUM_MSG_TYPE_SEARCH_REQUEST;
    memcpy((char*)pdu->caData,name.toStdString().c_str(),32);
    TcpClient::getInstance().getSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void Friend::flushfriend()
{
    QString name=TcpClient::getInstance().getName();
    PDU *pdu=mkPDU(0);
    pdu->uiMsgType=ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    memcpy(pdu->caData,name.toStdString().c_str(),name.length());
    TcpClient::getInstance().getSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void Friend::showfriend(PDU *pdu)
{
    this->m_pFriendListwidget->clear();
    char tmp[32];
    int msgl =pdu->uiMsgLen/32;
    for(int i=0;i<msgl;i++){
        memcpy(tmp,(char*)pdu->caMsg+i*32,32);
        this->m_pFriendListwidget->addItem(tmp);
    }
}

void Friend::delfriend()
{
    QString q=this->m_pFriendListwidget->currentItem()->text();
    qDebug()<<q;
    if(0 == q.length())return;
    PDU *pdu=mkPDU(0);
    pdu->uiMsgType=ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
    memcpy(pdu->caData,q.toStdString().c_str(),q.length());
    QString my_name=TcpClient::getInstance().getName();
    memcpy(pdu->caData+32,my_name.toStdString().c_str(),my_name.length());
    TcpClient::getInstance().getSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
    this->flushfriend();
}

QMap<QString, P2pchat*> Friend::chatlist;
QMap<QString,P2pchat *> &Friend::getChatList()
{
    return chatlist;
}
void Friend::chatFriend()
{
    QString name=this->m_pFriendListwidget->currentItem()->text();
    if(name.length() ==0){
        QMessageBox::information(NULL,"私聊","未选择私聊对象");
        return;
    }
     if(!chatlist.contains(QString(name))){
        P2pchat *p2pchat =new P2pchat;
        p2pchat->setWindowTitle(name);
        QString myname =TcpClient::getInstance().getName();
        p2pchat->setmyName(myname);
        p2pchat->setChatName(name);
        Friend::chatlist.insert(name,p2pchat);
     };
     chatlist.value(name)->show();

}
