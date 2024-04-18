#include "mytcpsocket.h"
#include "protocol.h"
#include "mytcpserver.h"
#include<QDebug>
#include"dao.h"
#include<QDir>
MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    m_timer=new QTimer;
    connect(m_timer,SIGNAL(timeout()),this, SLOT(DownloadFile()));
    connect(this,SIGNAL(readyRead()),this,SLOT(recvMsg()));
    connect(this,SIGNAL(disconnected()),this,SLOT(clientOffline()));
    isUpload=false;
}

QString MyTcpSocket::getName()
{
    return m_name;
}

void MyTcpSocket::saveDir(const QString strSrcDir, const QString strDestDir)
{
    QDir dir;
    dir.mkpath(strDestDir);
    dir.setPath(strSrcDir);
    QFileInfoList fileInfoList=dir.entryInfoList();
    QString srcTmp;
    QString destTmp;for(int i=0;i<fileInfoList.size(); i++)
        if(fileInfoList[i].isFile()){
            srcTmp =strSrcDir+'/'+fileInfoList[i].fileName();
            destTmp=strDestDir+'/'+fileInfoList[i].fileName();
            QFile::copy(srcTmp,destTmp);
        }else if(fileInfoList[i].isDir()){
            if(QString(".")==fileInfoList[i].fileName()
                ||QString("..")==fileInfoList[i].fileName()){
                continue;
            };
            srcTmp=strSrcDir+'/'+fileInfoList[i].fileName();
            destTmp=strDestDir+'/'+fileInfoList[i].fileName();
            saveDir(srcTmp,destTmp);
        }
}



void MyTcpSocket::recvMsg()
{
    if(!isUpload){
        uint uipduLen=0;
        this->read((char *)&uipduLen,sizeof(uint));
        uint uiMsgLen=uipduLen-sizeof(PDU);
        PDU *pdu=mkPDU(uiMsgLen);
        this->read((char*)pdu+sizeof(uint),uipduLen-sizeof(uint));
        switch (pdu->uiMsgType){
        case ENUM_MSG_TYPE_REGIST_REQUEST:
        {
            char sName[32]={'\0'};
            char sPwd[32]={'\0'};
            memcpy(sName,pdu->caData,32);
            memcpy(sPwd,pdu->caData+32,32);
            qDebug()<<"name"<<":"<<sName<<"pwd"<<":"<<sPwd<<":";
            bool ret=DAO::getInstance().handleRegist(sName,sPwd);
            PDU *pde_rep=mkPDU(0);
            pde_rep->uiMsgType=ENUM_MSG_TYPE_REJIST_RESPONSE;
            if(ret){
                strcpy_s(pde_rep->caData,REGIST_0K);
                QDir dir;
                dir.mkdir(QString("./%1").arg(sName));
            }else
            {
                strcpy_s(pde_rep->caData,REGIST_FAILED);
            };
            this->write((char*)pde_rep,pde_rep->uiPDULen);
            free(pde_rep);
            pde_rep=NULL;
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_REQUEST:
        {
            char sName[32]={'\0'};
            char sPwd[32]={'\0'};
            memcpy(sName,pdu->caData,32);
            memcpy(sPwd,pdu->caData+32,32);
            bool ret=DAO::getInstance().handleLogin(sName,sPwd);
            PDU *pde_rep=mkPDU(0);
            pde_rep->uiMsgType=ENUM_MSG_TYPE_LOGIN_RESPONSE;
            QDir dir(QString("./%1").arg(sName));
            if(!dir.exists()){
                dir.mkdir(QString("./%1").arg(sName));
            }
            if(ret){
                strcpy_s(pde_rep->caData,LOGIN_0K);
                m_name=sName;
            }else
            {
                strcpy_s(pde_rep->caData,LOGIN_FAILED);
            };
            this->write((char*)pde_rep,pde_rep->uiPDULen);
            free(pde_rep);
            pde_rep=NULL;
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
        {
            QStringList lists=DAO::getInstance().handleAllOnline();
            uint MsgLen=lists.size()*32;
            PDU *p=mkPDU(MsgLen);
            p->uiMsgType=ENUM_MSG_TYPE_ALL_ONLINE_RESPONSE;
            for(int i=0;i<lists.size();i++){
                memcpy((char *)p->caMsg+i*32,lists.at(i).toStdString().c_str(), lists.at(i).size());
            };
            write((char*)p,p->uiPDULen);
            free(p);
            p=NULL;
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_REQUEST:{
            char name[32]={'\0'};
            memcpy(name,pdu->caData,32);
            int isOnline=DAO::getInstance().handleSearch(name);
            PDU *p=mkPDU(0);
            p->uiMsgType=ENUM_MSG_TYPE_SEARCH_RESPONSE;
            if(-1 ==isOnline){
                strcpy(p->caData,"no such people");
            }else if(1==isOnline){
                strcpy(p->caData,"online");
            }else{
                strcpy(p->caData,"offline");
            }
            write((char*)p,p->uiPDULen);
            free(p);
            p=NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {   char name[32];
            char fname[32];
            memcpy(name,pdu->caData,32);
            memcpy(fname,pdu->caData+32,32);
            int ret=DAO::getInstance().handleAddFriend(name,fname);
            PDU *p=mkPDU(0);
            p->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPONSE;
            if(ret == -2){
                strcpy(p->caData,NOT_EXIST);
            }else if(ret == -3){
                strcpy(p->caData,IS_OFFLINE);
            }else if( ret ==0){
                strcpy(p->caData,ALREADY_FRIEND);
            }else if(ret ==-1){
                pdu->uiMsgType=ENUM_MSG_TYPE_RESEND_FRIEND_REQUEST;
                MyTcpServer::Getinstance().resend(fname,pdu);
                strcpy(p->caData,SEND_APPLY);
            }
            this->write((char*)p,p->uiPDULen);
            free(p);
            p=NULL;
            break;
        }
        case ENUM_MSG_TYPE_RESEND_FRIEND_RESPONSE:{
            char name[32]={'\0'};
            char fname[32]={'\0'};
            memcpy(name,pdu->caData,32);
            memcpy(fname,pdu->caData+32,32);
            DAO::getInstance().handleaddfriend(name,fname);
            PDU *p=mkPDU(0);
            p->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPONSE;
            strcpy(p->caData,ADD_SUCCESS);
            MyTcpServer::Getinstance().resend(name,p);
            break;
        };
        case ENUM_MSG_TYPE_RESEND_REFUSE_REQUEST:{
            char name[32]={'\0'};
            char fname[32]={'\0'};
            memcpy(name,pdu->caData,32);
            memcpy(fname,pdu->caData+32,32);
            DAO::getInstance().handleaddfriend(name,fname);
            PDU *p=mkPDU(0);
            p->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPONSE;
            strcpy(p->caData,REFUSE_ADD);
            MyTcpServer::Getinstance().resend(name,p);
            break;
        };
        case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:{
            char name[32]={'\0'};
            memcpy(name,pdu->caData,32);
            QStringList friendlist=DAO::getInstance().flushfriend(name);
            PDU *p=mkPDU(friendlist.size()*32);
            p->uiMsgType=ENUM_MSG_TYPE_FLUSH_FRIEND_RESPONSE;
            for(int i=0;i<friendlist.size();i++){
                memcpy((char*)(p->caMsg)+32*i,friendlist.at(i).toStdString().c_str(),friendlist.at(i).size());
            };
            this->write((char*)p,p->uiPDULen);
            free(p);
            p=NULL;
            break;
        };
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:{
            char name[32]={'\0'};
            char myname[32]={'\0'};
            memcpy(name,pdu->caData,32);
            memcpy(myname,pdu->caData+32,32);
            bool ret = DAO::getInstance().delFriend(name,myname);
            PDU *p=mkPDU(0);
            p->uiMsgType=ENUM_MSG_TYPE_DELETE_FRIEND_RESPONSE;
            strcpy(p->caData,DEL_SUCCESS);
            this->write((char*)p,p->uiPDULen);
            free(p);
            p=NULL;
            break;
        };
        case ENUM_MSG_TYPE_SEND_MESSAGE_REQUEST:{
            char name[32]={'\0'};
            char fname[32]={'\0'};
            memcpy(name,pdu->caData,32);
            memcpy(fname,pdu->caData+32,32);
            pdu->uiMsgType=ENUM_MSG_TYPE_SEND_MESSAGE_RESPONSE;
            MyTcpServer::Getinstance().resend(fname,pdu);
            break;
        };
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:{
            char name[32]={'\0'};
            char new_dir_name[32]={'\0'};

            memcpy(name,pdu->caData,32);
            memcpy(new_dir_name,pdu->caData+32,32);
            QString curpath=QString("%1").arg((char*)(pdu->caMsg));
            curpath=curpath.left(pdu->uiMsgLen);
            QDir dir;
            PDU *p =mkPDU(0);
            qDebug()<<curpath;
            p->uiMsgType=ENUM_MSG_TYPE_CREATE_DIR_RESPONSE;
            if(dir.exists(curpath)){
                QString str_new_path=curpath+"/"+new_dir_name;
                int ret=dir.exists(str_new_path);
                if(ret){
                    strcpy(p->caData,DIR_EXIST);
                }else{
                    int ret=dir.mkdir(str_new_path);
                    qDebug()<<ret;
                    strcpy(p->caData,DIR_SUCCESS);
                };
            }else{
                strcpy(p->caData,DIR_ERROR);
            };
            this->write((char*)p,p->uiPDULen);
            break;
        };
        case ENUM_MSG_TYPE_CREATE_FLUSH_REQUEST:{
            qDebug()<<"ENUM_MSG_TYPE_CREATE_FLUSH_REQUEST";
            QString curpath=QString((char*)pdu->caMsg).left(pdu->uiMsgLen);
            QDir dir(curpath);
            QFileInfoList fileInfo=dir.entryInfoList();
            PDU *p=mkPDU(sizeof(FileInfo)*fileInfo.size());
            FileInfo *info=NULL;
            for(int i=0;i<fileInfo.size();i++){
                info=(FileInfo *)(p->caMsg)+i;
                QString name=fileInfo[i].fileName();
                memcpy(info->FileName,name.toStdString().c_str(),name.size());
                if(fileInfo[i].isDir()){
                    info->fileType=0;
                }else if(fileInfo[i].isFile()){
                    info->fileType=1;
                };
            };
            p->uiMsgType=ENUM_MSG_TYPE_CREATE_FLUSH_RESPONSE;
            this->write((char*)p,p->uiPDULen);
            free(p);
            p=NULL;
            break;
        };
        case ENUM_MSG_TYPE_DEL_DIR_REQUEST:{
            char name[32]={'\0'};
            char del_dir_name[32]={'\0'};

            memcpy(name,pdu->caData,32);
            memcpy(del_dir_name,pdu->caData+32,32);
            QString curpath=QString((char*)pdu->caMsg).left(pdu->uiMsgLen);
            curpath=QString("%1/%2").arg(curpath).arg(del_dir_name);
            qDebug()<<curpath;
            QFileInfo info(curpath);
            PDU *p=mkPDU(0);
            p->uiMsgType=ENUM_MSG_TYPE_DEL_DIR_RESPONSE;
            if(info.isDir()){
                QDir dir;
                dir.setPath(curpath);
                dir.removeRecursively();
                strcpy_s(p->caData,DEL_SUCCESS);
            }else{
                strcpy_s(p->caData,DEL_FAIL);
            };
            write((char*)p,p->uiPDULen);
            free(p);
            p=NULL;
            break;
        };
        case ENUM_MSG_TYPE_RENAME_DIR_REQUEST:{
            char name[32]={'\0'};
            char re_name[32]={'\0'};
            memcpy(name,pdu->caData,32);
            memcpy(re_name,pdu->caData+32,32);
            char *curpath=new char[pdu->uiMsgLen];
            memcpy(curpath,pdu->caMsg,pdu->uiMsgLen);
            QString cur=QString(curpath).left(pdu->uiMsgLen);
            QString path=QString("%1/%2").arg(cur);
            QDir dir;
            PDU *p=mkPDU(0);
            qDebug()<<path.arg(name);
            p->uiMsgType=ENUM_MSG_TYPE_RENAME_DIR_RESPONSE;
            int ret=dir.rename(path.arg(name),path.arg(re_name));
            if(ret){
                strcpy_s(p->caData,RENAME_SUCCESS);
            }else{
                strcpy_s(p->caData,RENAME_FAIL);
            }
            write((char*)p,p->uiPDULen);
            free(p);
            p=NULL;
            break;
        };
        case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:{
            qDebug()<<"ENUM_MSG_TYPE_ENTER_DIR_REQUEST";
            char name[32]={'\0'};
            memcpy(name,pdu->caData,32);
            QString curpath=QString("%1").arg((char*)pdu->caMsg);
            curpath=curpath.left(pdu->uiMsgLen);
            curpath=QString(("%1/%2")).arg(curpath).arg(name);
            qDebug()<<curpath;
            QDir dir;
            QFileInfo fileinfo(curpath);
            if(fileinfo.isDir()){
                QDir dir;
                dir.setPath(curpath);
                QFileInfoList fileInfo=dir.entryInfoList();
                PDU *p=mkPDU(sizeof(FileInfo)*fileInfo.size());
                memcpy(p->caData,name,32);
                FileInfo *info=NULL;
                for(int i=0;i<fileInfo.size();i++){
                    info=(FileInfo *)(p->caMsg)+i;
                    QString name=fileInfo[i].fileName();
                    memcpy(info->FileName,name.toStdString().c_str(),name.size());
                    if(fileInfo[i].isDir()){
                        info->fileType=0;
                    }else if(fileInfo[i].isFile()){
                        info->fileType=1;
                    };
                };
                p->uiMsgType=ENUM_MSG_TYPE_CREATE_FLUSH_RESPONSE;
                this->write((char*)p,p->uiPDULen);
                free(p);
                p=NULL;
            }else{
                PDU *res_p=mkPDU(curpath.size());
                res_p->uiMsgType=ENUM_MSG_TYPE_ENTER_DIR_RESPONSE;
                memcpy(res_p->caMsg,curpath.toStdString().c_str(),curpath.size());
                strcpy_s(res_p->caData,DIR_NO_EXIST);
                this->write((char*)res_p,res_p->uiPDULen);
                free(res_p);
                res_p=NULL;
            };
            break;
        };
        case ENUM_MSG_TYPE_UPLOAD_REQUEST:{
            char filename[32]={'\0'};
            qint64 filesize;
            sscanf(pdu->caData,"%s %lld",filename,&filesize);
            QString curpath=QString("%1").arg((char*)pdu->caMsg);
            curpath=curpath.left(pdu->uiMsgLen);
            curpath=QString("%1/%2").arg(curpath).arg(filename);
            mfile.setFileName(curpath);
            if(mfile.open(QIODevice::WriteOnly)){
                isUpload=true;
                mTotal=filesize;
                m_iRecved=0;
            };
        };
        case ENUM_MSG_TYPE_DEL_FILE_REQUEST:{
            char name[32]={'\0'};
            memcpy(name,pdu->caData,32);
            QString curpath=QString((char*)pdu->caMsg).left(pdu->uiMsgLen);
            curpath=QString("%1/%2").arg(curpath).arg(name);
            qDebug()<<curpath;
            QFileInfo info(curpath);
            PDU *p=mkPDU(0);
            p->uiMsgType=ENUM_MSG_TYPE_DEL_FILE_RESPONSE;
            if(info.isFile()){
                QDir dir;
                dir.remove(curpath);
                strcpy_s(p->caData,DEL_SUCCESS);
            }else{
                strcpy_s(p->caData,DEL_FAIL);
            };
            write((char*)p,p->uiPDULen);
            free(p);
            p=NULL;
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_REQUEST:{
            char name[32]={'\0'};
            memcpy(name,pdu->caData,32);
            char *curpath=new char[pdu->uiMsgLen];
            memcpy(curpath,pdu->caMsg,pdu->uiMsgLen);
            QString cur=QString(curpath).left(pdu->uiMsgLen);
            QString path=QString("%1/%2").arg(cur).arg(name);
            mfile.setFileName(path);
            qint64 filesize=mfile.size();
            PDU *p=mkPDU(0);
            p->uiMsgType=ENUM_MSG_TYPE_DOWNLOAD_RESPONSE;
            sprintf(p->caData,"%s %lld",name,filesize);
            write((char*)p,p->uiPDULen);
            free(p);
            p=NULL;
            m_timer->start(1000);
            break;
        };
        case ENUM_MSG_TYPE_SHARE_REQUEST:{
            char sendname[32]={'\0'};
            int num;
            sscanf(pdu->caData,"%s %d",sendname,&num);
            PDU *resendPdu=mkPDU(pdu->uiMsgLen-num*32);
            resendPdu->uiMsgType=ENUM_MSG_TYPE_SHARE_RESEND_REQUEST;
            memcpy(resendPdu->caData,sendname,32);
            memcpy((char*)resendPdu->caMsg,(char*)(pdu->caMsg)+num*32,pdu->uiMsgLen-num*32);
            char recvName[32]={"\0"};
            for(int i=0;i<num;i++){
                memcpy(recvName,(char*)(pdu->caMsg)+i*32,32);
                MyTcpServer::Getinstance().resend(recvName,resendPdu);
            };
            free(resendPdu);
            resendPdu=NULL;
            resendPdu=mkPDU(0);
            strcpy_s(resendPdu->caData,SHARE_SUC);
            resendPdu->uiMsgType=ENUM_MSG_TYPE_SHARE_RESPONSE;
            write((char*)resendPdu,resendPdu->uiPDULen);
            free(resendPdu);
            resendPdu=NULL;
            break;
        };
        case ENUM_MSG_TYPE_SHARE_RESEND_RESPONSE:{
            char sendName[32]={"\0"};
            char *path=new char[pdu->uiMsgLen];
            memcpy(path,pdu->caMsg,pdu->uiMsgLen);
            memcpy(sendName,pdu->caData,32);
            QString pat=QString(path).left(pdu->uiMsgLen);
            int index =pat.lastIndexOf("/");
            QString fileName=pat.mid(index);
            QString recvPah=QString("./%1%2").arg(sendName).arg(fileName);
            QFileInfo fileInfo(pat);
            if(fileInfo.isFile()){
                QFile::copy(pat,recvPah);
            }else if(fileInfo.isDir()){
                saveDir(pat,recvPah);
            }
            PDU *p=mkPDU(0);
            p->uiMsgType=ENUM_MSG_TYPE_SHARE_RESEND_RESPONSE;
            write((char*)p,p->uiPDULen);
            break;
        }
        case ENUM_MSG_TYPE_MOVE_REQUEST:{
            int src_len=0;
            int dest_len=0;
            char fileName[32]={"\0"};
            sscanf(pdu->caData,"%d %d %s",&src_len,&dest_len,&fileName);
            char *srcPath=new char[src_len];
            char *destPath=new char[dest_len];
            memcpy(srcPath,pdu->caMsg,src_len);
            memcpy(destPath,pdu->caMsg+src_len+1,dest_len);
            QString ssrcPath=QString(srcPath).left(src_len);
            QString sdestPath=QString(destPath).left(dest_len);
            QFileInfo file(sdestPath);
            PDU *p=mkPDU(0);
            p->uiMsgType=ENUM_MSG_TYPE_MOVE_RESPONSE;

            if(file.isFile()){
                strcpy_s(p->caData,MOVE_NOT_DIR);
                write((char*)p,p->uiPDULen);
                free(p);
                p=NULL;
                break;
            };
            QString dpath=QString("%1/%2").arg(sdestPath).arg(fileName);

            int ret =QFile::rename(ssrcPath,dpath);
            if(ret){
                strcpy_s(p->caData,MOVE_SUC);
                write((char*)p,p->uiPDULen);
                free(p);
                p=NULL;
            }else{
                strcpy_s(p->caData,MOVE_NOT_DIR);
                write((char*)p,p->uiPDULen);
                free(p);
                p=NULL;
            }
        break;
        }

        };
        free(pdu);
        pdu=NULL;
    }else{
        QByteArray buff=readAll();
        mfile.write(buff);
        m_iRecved+=buff.size();
        if(m_iRecved ==mTotal){
            PDU *pdu=mkPDU(0);
            pdu->uiMsgType=ENUM_MSG_TYPE_UPLOAD_RESPONSE;
            isUpload=false;
            mfile.close();
            strcpy_s(pdu->caData,UPLOAD_SUCCESS);
            write((char*)pdu,pdu->uiPDULen);
            free(pdu);
            pdu=NULL;
        }else if(m_iRecved >mTotal){
            isUpload=false;
            PDU *pdu=mkPDU(0);
            pdu->uiMsgType=ENUM_MSG_TYPE_UPLOAD_RESPONSE;
            mfile.close();
            strcpy_s(pdu->caData,UPLOAD_FAIL);
        };
    };
};

void MyTcpSocket::DownloadFile()
{
    m_timer->stop();
    qint64 ret=0;
    char *buffer=new char[4096];
    mfile.open(QIODevice::ReadOnly);
    while(true){
        ret=mfile.read(buffer,4096);
        if(ret>0&&ret<=4096){
            write(buffer,ret);
        }else if(ret <=0){
            break;
        };
    };
    mfile.close();
    delete []buffer;
    buffer=NULL;
}

void MyTcpSocket::clientOffline()
{
    DAO::getInstance().handleLoginout(m_name.toStdString().c_str());
    emit offline(this);

}
