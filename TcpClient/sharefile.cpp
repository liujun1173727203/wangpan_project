#include "sharefile.h"
#include <QCheckBox>
#include "tcpclient.h"
#include "protocol.h""
#include"opewidget.h"
sharefile::sharefile(QWidget *parent)
    : QWidget{parent}
{

    mSelectAllPB= new QPushButton("全选");
    mCancelAllPB= new QPushButton("取消选择");
    mOkPB = new QPushButton("确定");
    mCancelPB= new QPushButton("取消");

    mSA=new QScrollArea;
    mFriendW =new QWidget;
    mFriendWVBL=new QVBoxLayout(mFriendW);
    mButtonGroup =new QButtonGroup(mFriendW);
    mButtonGroup->setExclusive(false);

    QHBoxLayout *pTopHBL =new QHBoxLayout;
    pTopHBL->addWidget(mSelectAllPB);
    pTopHBL->addWidget(mCancelAllPB);
    pTopHBL->addStretch();

    QHBoxLayout *pDownHBL =new QHBoxLayout;
    pDownHBL->addWidget(mOkPB);
    pDownHBL->addWidget(mCancelPB);

    QVBoxLayout *pmain = new QVBoxLayout;
    pmain->addLayout(pTopHBL);
    pmain->addWidget(mSA);
    pmain->addLayout(pDownHBL);
    setLayout(pmain);
    connect(mCancelAllPB,SIGNAL(clicked(bool)),this,SLOT(cancelSelect()));
    connect(mSelectAllPB,SIGNAL(clicked(bool)),this,SLOT(SelectAll()));
    connect(mOkPB,SIGNAL(clicked(bool)),this,SLOT(OKPB()));
    connect(mCancelPB,SIGNAL(clicked(bool)),this,SLOT(Cancel()));
}

sharefile &sharefile::getShareFile()
{
    static sharefile file;
    return file;
}

void sharefile::updatefriend(QListWidget *friendlist)
{

    if(NULL ==friendlist){
        return;
    }
    QAbstractButton* tmp=NULL;
    QList<QAbstractButton*> prefriend=this->mButtonGroup->buttons();
    for(int i=0;i<prefriend.size();i++){
        mFriendWVBL->removeWidget(prefriend[i]);
        mButtonGroup->removeButton(prefriend[i]);
        tmp =prefriend[i];
        prefriend.removeOne(tmp);
        delete tmp;
        tmp= NULL;
    }
    QCheckBox *pcb=NULL;
    for(int i=0;i<friendlist->count();i++){
        pcb=new QCheckBox(friendlist->item(i)->text());
        mFriendWVBL->addWidget(pcb);
        mButtonGroup->addButton(pcb);
    }
    mSA->setWidget(mFriendW);
}

void sharefile::cancelSelect()
{
    QList<QAbstractButton*> list = mButtonGroup->buttons();
    for(int i=0;i<list.size();i++){
        if(list[i]->isChecked()){
            list[i]->setChecked(false);
        };
    };
}
void sharefile::SelectAll()
{
    QList<QAbstractButton*> list = mButtonGroup->buttons();
    for(int i=0;i<list.size();i++){
        if(!list[i]->isChecked()){
            list[i]->setChecked(true);
        };
    };
}

void sharefile::OKPB()
{
    qDebug()<<"okpb";
    QString shareName=TcpClient::getInstance().getName();
    QString fileName=OpeWidget::getInstance().getBook()->getFileName();
    QString path=TcpClient::getInstance().getCurPath();

    path =path+"/"+fileName;
    qDebug()<<path;
    QList<QAbstractButton*> list = mButtonGroup->buttons();
    int num=0;

    for(int i=0;i<list.size();i++){
        if(list[i]->isChecked()){
            num++;
        };
    };
    PDU *pdu=mkPDU(32*num+path.size());
    pdu->uiMsgType=ENUM_MSG_TYPE_SHARE_REQUEST;
    sprintf(pdu->caData,"%s %d",shareName.toStdString().c_str(),num);
    int j=0;
    for(int i=0;i<list.size();i++){
        if(list[i]->isChecked()){
            num++;
            memcpy((char*)pdu->caMsg+32*j,list[i]->text().toStdString().c_str(),32);
            j++;
        };
    };
    memcpy((char*)pdu->caMsg+32*j,path.toStdString().c_str(),path.size());
    TcpClient::getInstance().getSocket().write((char*)pdu,pdu->uiPDULen);
}

void sharefile::Cancel()
{
    this->hide();
}
