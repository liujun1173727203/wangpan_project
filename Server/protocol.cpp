#include"protocol.h"

PDU *mkPDU(uint MsgLen){
    uint uiPDULen=sizeof(PDU)+MsgLen;
    PDU *pdu=(PDU*)malloc(uiPDULen);
    memset(pdu,0,uiPDULen);
    pdu->uiPDULen=uiPDULen;
    pdu->uiMsgLen=MsgLen;
    return pdu;
}
