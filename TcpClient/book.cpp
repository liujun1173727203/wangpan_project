#include "book.h"
#include<QInputDialog>
#include "protocol.h"
#include "tcpclient.h"
#include <QMessageBox>
#include <QFileDialog>
#include "sharefile.h"
#include "opewidget.h"
Book::Book(QWidget *parent)
    : QWidget{parent}
{
    m_Time=new QTimer;
     m_pBookListW=new QListWidget;
     m_pReturnPB=new QPushButton("返回");
     m_pCreatePB=new QPushButton("创建文件夹");
     m_pDelPB=new QPushButton("删除文件夹");
     m_pRenamePB=new QPushButton("重命名");
     m_pFlushPB=new QPushButton("刷新文件");

     QVBoxLayout *pDirVBL =new QVBoxLayout;
     pDirVBL->addWidget(m_pReturnPB);
     pDirVBL->addWidget(m_pCreatePB);
     pDirVBL->addWidget(m_pDelPB);
     pDirVBL->addWidget(m_pRenamePB);
     pDirVBL->addWidget(m_pFlushPB);

     m_pUploadPB=new QPushButton("上传文件");
     m_pDownloadPB=new QPushButton("下载文件");
     m_pDelFilePB=new QPushButton("删除文件");
     m_pSharePB=new QPushButton("共享文件");
     m_pMovePB=new QPushButton("移动文件");
        m_pMoveToPB=new QPushButton("移动至");
     QVBoxLayout *pFileVBL =new QVBoxLayout;
     pFileVBL->addWidget(m_pUploadPB);
     pFileVBL->addWidget(m_pDownloadPB);
     pFileVBL->addWidget(m_pDelFilePB);
     pFileVBL->addWidget(m_pSharePB);
     pFileVBL->addWidget(m_pMovePB);
     pFileVBL->addWidget(m_pMoveToPB);
     m_pMoveToPB->setDisabled(true);
     QHBoxLayout *pMain =new QHBoxLayout;
     pMain->addWidget(m_pBookListW);
     pMain->addLayout(pDirVBL);
     pMain->addLayout(pFileVBL);
     setLayout(pMain);
     connect(m_pCreatePB,SIGNAL(clicked(bool)),this,SLOT(createDir()));
     connect(m_pDelPB,SIGNAL(clicked(bool)),this,SLOT(delDir()));
     connect(m_pFlushPB,SIGNAL(clicked(bool)),this,SLOT(flushDir()));
     connect(m_pRenamePB,SIGNAL(clicked(bool)),this,SLOT(renameDir()));
     connect(m_pBookListW,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(enterDir(QModelIndex)));
     connect(m_pReturnPB,SIGNAL(clicked(bool)),this,SLOT(back()));
     connect(m_pUploadPB,SIGNAL(clicked(bool)),this,SLOT(uploadFile()));
     connect(m_Time,SIGNAL(timeout()),this,SLOT(uploaddata()));
     connect(m_pDelFilePB,SIGNAL(clicked(bool)),this,SLOT(delFile()));
     connect(m_pDownloadPB,SIGNAL(clicked(bool)),this,SLOT(downFile()));
     connect(m_pSharePB,SIGNAL(clicked(bool)),this,SLOT(shareFile()));
     connect(m_pMovePB,SIGNAL(clicked(bool)),this,SLOT(moveFile()));
     connect(m_pMoveToPB,SIGNAL(clicked(bool)),this,SLOT(moveToFile()));
     flushDir();
}

QString Book::getFileName()
{
    return curFileName;
}

void Book::createDir()
{
    QString new_dir=QInputDialog::getText(this,"新建文件夹","文件名");
    if(new_dir.isEmpty()||new_dir.length()==0){
        QMessageBox::warning(this,"新建文件夹","文件名为空");
        return;
    }
    QString strName=TcpClient::getInstance().getName();
    QString curpath=TcpClient::getInstance().getCurPath();
    qDebug()<<curpath;
    PDU *pdu=mkPDU(curpath.size());
    pdu->uiMsgType=ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
    memcpy(pdu->caData,strName.toStdString().c_str(),strName.size());
    memcpy(pdu->caData+32,new_dir.toStdString().c_str(),new_dir.size());
    memcpy(pdu->caMsg,curpath.toStdString().c_str(),curpath.size());
    TcpClient::getInstance().getSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void Book::delDir()
{
    QString name=m_pBookListW->currentItem()->text();
    if(name.length()==0){
        return;
    };
    PDU *p =mkPDU(TcpClient::getInstance().getCurPath().size());
    p->uiMsgType=ENUM_MSG_TYPE_DEL_DIR_REQUEST;
    memcpy(p->caData,TcpClient::getInstance().getName().toStdString().c_str(),TcpClient::getInstance().getName().size());
    memcpy(p->caData+32,name.toStdString().c_str(),name.size());
    memcpy(p->caMsg,TcpClient::getInstance().getCurPath().toStdString().c_str(),TcpClient::getInstance().getCurPath().size());
    TcpClient::getInstance().getSocket().write((char*)p,p->uiPDULen);
    free(p);
    p=NULL;
}

void Book::flushDir()
{
    QString curpath=TcpClient::getInstance().getCurPath();
    qDebug()<<curpath;
    PDU *pdu=mkPDU(curpath.length());
    pdu->uiMsgType=ENUM_MSG_TYPE_CREATE_FLUSH_REQUEST;
    memcpy(pdu->caMsg,curpath.toStdString().c_str(),curpath.length());
    TcpClient::getInstance().getSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}
void Book::updateFileList(PDU *pdu)
{
    qDebug()<<"flush";
    m_pBookListW->clear();
    int size=pdu->uiMsgLen/sizeof(FileInfo);
    FileInfo *fileinfo =NULL;
    for(int i=2;i<size;i++){
        fileinfo=(FileInfo *)(pdu->caMsg)+i;
        QListWidgetItem *item=new QListWidgetItem;
        QString type;
        if(fileinfo->fileType==0){
            item->setIcon(QIcon(QPixmap(":/map/111.png")));
        }else{
            item->setIcon(QIcon(QPixmap(":/map/222.png")));
        }
        item->setText(fileinfo->FileName);
        m_pBookListW->addItem(item);
    }
}

void Book::renameDir()
{
    QString oldName =m_pBookListW->currentItem()->text();
    QString newName=QInputDialog::getText(this,"重命名","文件名");
    QString curpath=TcpClient::getInstance().getCurPath();
    PDU *p=mkPDU(curpath.size());
    memcpy(p->caData,oldName.toStdString().c_str(),oldName.size());
        memcpy(p->caData+32,newName.toStdString().c_str(),newName.size());
    memcpy(p->caMsg,curpath.toStdString().c_str(),curpath.size());
    p->uiMsgType=ENUM_MSG_TYPE_RENAME_DIR_REQUEST;
    TcpClient::getInstance().getSocket().write((char *)p,p->uiPDULen);
}

void Book::enterDir(const QModelIndex &x)
{
    qDebug()<<"enterDir";
    QString name=x.data().toString();
    QString curpath =TcpClient::getInstance().getCurPath();
    PDU *pdu=mkPDU(curpath.size());
    pdu->uiMsgType=ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    memcpy(pdu->caData,name.toStdString().c_str(),name.size());
    memcpy(pdu->caMsg,curpath.toStdString().c_str(),curpath.size());
    TcpClient::getInstance().getSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void Book::back()
{
    QString curpath =TcpClient::getInstance().getCurPath();
    QString root ="./"+TcpClient::getInstance().getName();
    if(curpath == root){
        QMessageBox::warning(this,"返回","到达根目录");
    }else{
        int index =curpath.lastIndexOf("/");
        curpath.remove(index,curpath.size()-index);

        TcpClient::getInstance().setCurPath(curpath.toStdString().c_str());
        qDebug()<<TcpClient::getInstance().getCurPath();
        flushDir();
    }
}

void Book::uploadFile()
{
    QString curpath =TcpClient::getInstance().getCurPath();
    uploadfile=QFileDialog::getOpenFileName();
    if(uploadfile.isEmpty()){
        return;
    };
    int index=uploadfile.lastIndexOf("/");
    QString uploadFileName =uploadfile.right(uploadfile.size()-index-1);
    QFile file(uploadfile);
    int filesize=file.size();
    PDU *pdu=mkPDU(curpath.size());
    pdu->uiMsgType=ENUM_MSG_TYPE_UPLOAD_REQUEST;
    memcpy(pdu->caMsg,curpath.toStdString().c_str(),curpath.size());
    sprintf(pdu->caData,"%s %lld",uploadFileName.toStdString().c_str(),filesize);
    TcpClient::getInstance().getSocket().write((char *)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
    m_Time->start(1000);
}

void Book::uploaddata()
{
    m_Time->stop();
    QFile file(uploadfile);
    qint64 ret=0;
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this,"上传文件","打开文件失败");
        return;
    };
    char *buffer=new char[4096];
    while(true){
        ret=file.read(buffer,4096);
        if(ret>0&&ret<=4096){
            TcpClient::getInstance().getSocket().write(buffer,ret);
        }else if(ret ==0){
            break;
        }else if(ret <0){
            QMessageBox::warning(this,"上传文件","上传文件失败");
            break;
        };
    };
    file.close();
    free(buffer);
    buffer=NULL;
}

void Book::delFile()
{
    QString name=m_pBookListW->currentItem()->text();
    if(name.length()==0){
        return;
    };
    PDU *p =mkPDU(TcpClient::getInstance().getCurPath().size());
    p->uiMsgType=ENUM_MSG_TYPE_DEL_FILE_REQUEST;
    memcpy(p->caData,name.toStdString().c_str(),name.size());
    memcpy(p->caMsg,TcpClient::getInstance().getCurPath().toStdString().c_str(),TcpClient::getInstance().getCurPath().size());
    TcpClient::getInstance().getSocket().write((char*)p,p->uiPDULen);
    free(p);
    p=NULL;
}

void Book::downFile()
{
    QString savepath=QFileDialog::getExistingDirectory();
    if(savepath.isEmpty()){
        QMessageBox::warning(this,"下载文件","未指定下载路径");
        return;
    };
    TcpClient::getInstance().setSavePath(savepath);
    QString name=m_pBookListW->currentItem()->text();
    if(name.length()==0){
        return;
    };
    PDU *p =mkPDU(TcpClient::getInstance().getCurPath().size());
    p->uiMsgType=ENUM_MSG_TYPE_DOWNLOAD_REQUEST;
    memcpy(p->caData,name.toStdString().c_str(),name.size());
    memcpy(p->caMsg,TcpClient::getInstance().getCurPath().toStdString().c_str(),TcpClient::getInstance().getCurPath().size());
    TcpClient::getInstance().getSocket().write((char*)p,p->uiPDULen);
    free(p);
    p=NULL;
}

void Book::shareFile()
{
    QListWidgetItem *item=m_pBookListW->currentItem();
    if(NULL==item){
        QMessageBox::warning(this,"分享文件","未选择文件");
        return;
    };
     curFileName=item->text();
    QListWidget *friendlist =OpeWidget::getInstance().getFriend()->getFriendList();

    sharefile::getShareFile().updatefriend(friendlist);
    sharefile::getShareFile().show();
}

void Book::moveFile()
{
     fileName=m_pBookListW->currentItem()->text();
    if(fileName.length() ==0){
         QMessageBox::warning(this,"移动文件","未选择文件");
         return;
    };
     srcPath=TcpClient::getInstance().getCurPath()+"/"+fileName;
     m_pMoveToPB->setEnabled(true);

}
void Book::moveToFile()
{
     QString filename=m_pBookListW->currentItem()->text();
    if(filename.length() ==0){
        QMessageBox::warning(this,"移动文件","未选择文件");
        return;
    };
    destPath=TcpClient::getInstance().getCurPath()+"/"+filename;
    PDU *p=mkPDU(srcPath.size()+destPath.size()+1);
    p->uiMsgType=ENUM_MSG_TYPE_MOVE_REQUEST;
    sprintf(p->caData,"%d %d %s",srcPath.size(),destPath.size(),fileName.toStdString().c_str());
    memcpy((char*)p->caMsg,srcPath.toStdString().c_str(),srcPath.size());
    memcpy((char*)p->caMsg+srcPath.size()+1,destPath.toStdString().c_str(),destPath.size());
    TcpClient::getInstance().getSocket().write((char*)p,p->uiPDULen);
    free(p);
    p=NULL;
    fileName=NULL;
    srcPath=NULL;
    destPath=NULL;
    m_pMoveToPB->setEnabled(false);

}
