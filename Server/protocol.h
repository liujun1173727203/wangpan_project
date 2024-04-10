#ifndef PROTOCOL_H
#define PROTOCOL_H
#include<stdlib.h>
#include<string.h>
typedef unsigned int uint;


#define SEND_APPLY "Send a request"
#define REGIST_0K "register ok"
#define REGIST_FAILED "regist failed: name existed"
#define LOGIN_0K "login success"
#define LOGIN_FAILED "login failed: name or pwd error"
#define NOT_EXIST "no exist person"
#define IS_OFFLINE "person is offline"
#define ALREADY_FRIEND "you are already friends"
#define REFUSE_ADD "refuse add friend"
#define ADD_SUCCESS "add friend success"
#define DEL_SUCCESS "delete success"
enum ENUM_MSG_TYPE{
    ENUM_MSG_TYPE_MIN=0,
    ENUM_MSG_TYPE_MAX=0X00ffffff,
    ENUM_MSG_TYPE_REGIST_REQUEST,
    ENUM_MSG_TYPE_REJIST_RESPONSE,
    ENUM_MSG_TYPE_LOGIN_REQUEST,
    ENUM_MSG_TYPE_LOGIN_RESPONSE,
    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,
    ENUM_MSG_TYPE_ALL_ONLINE_RESPONSE,
    ENUM_MSG_TYPE_SEARCH_REQUEST,
    ENUM_MSG_TYPE_SEARCH_RESPONSE,
    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST,
    ENUM_MSG_TYPE_ADD_FRIEND_RESPONSE,
    ENUM_MSG_TYPE_RESEND_FRIEND_REQUEST,
    ENUM_MSG_TYPE_RESEND_FRIEND_RESPONSE,
    ENUM_MSG_TYPE_RESEND_REFUSE_REQUEST,

    ENUM_MSG_TYPE_FLUSH_FRIEND_RESPONSE,
    ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST,

    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST,
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPONSE,

    ENUM_MSG_TYPE_SEND_MESSAGE_REQUEST,
    ENUM_MSG_TYPE_SEND_MESSAGE_RESPONSE,
};
//消息协议
struct PDU{
    uint uiPDULen;
    uint uiMsgType;
    char caData[64];
    uint uiMsgLen;
    int caMsg[];
};

PDU *mkPDU(uint MsgLen);
#endif // PROTOCOL_H
