#ifndef PROTOCOL_H
#define PROTOCOL_H
#include<stdlib.h>
#include<string.h>
typedef unsigned int uint;

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
