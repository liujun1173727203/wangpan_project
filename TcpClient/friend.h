#ifndef FRIEND_H
#define FRIEND_H

#include"online.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QWidget>
#include<QTextEdit>
#include<QListWidget>
#include<QLineEdit>
#include<QPushButton>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include"p2pchat.h"
class Friend:public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);
    void setUsrOnline(PDU *pdu);
    static QMap<QString,P2pchat *> &getChatList();
    static QMap<QString, P2pchat*> chatlist;
public slots:
     void showOnline();
     void searchOnlinefriend();
     void flushfriend();
     void showfriend(PDU *pdu);
     void delfriend();
     void chatFriend();
private:
    QTextEdit *m_pShowMsgTe;
    QListWidget *m_pFriendListwidget;
    QLineEdit *m_pInputMsgLe;

    QPushButton *m_pDelFriendPB;
    QPushButton *m_pFlushFriendPB;
    QPushButton *m_pShowOnlineUsrPB;
    QPushButton *m_PSearchUsrPB;
    QPushButton *m_pMsgSendPB;
    QPushButton *m_pPrivateChatPB;
    Online *online;

};

#endif // FRIEND_H
