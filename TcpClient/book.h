#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListwidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include "protocol.h"
class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    QString getFileName();
public slots:
    void createDir();
    void delDir();
    void flushDir();
    void updateFileList(PDU *pdu);
    void renameDir();
    void enterDir(const QModelIndex &x);
    void back();
    void uploadFile();
    void uploaddata();
    void delFile();
    void downFile();
    void shareFile();
    void moveFile();
    void moveToFile();


signals:
private:
    QString uploadfile;
    QListWidget *m_pBookListW;
    QPushButton *m_pReturnPB;
    QPushButton *m_pCreatePB;
    QPushButton *m_pDelPB;
    QPushButton *m_pRenamePB;
    QPushButton *m_pFlushPB;
    QPushButton *m_pUploadPB;
    QPushButton *m_pDownloadPB;
    QPushButton *m_pDelFilePB;
    QPushButton *m_pSharePB;
    QPushButton *m_pMovePB;
    QPushButton *m_pMoveToPB;
    QTimer *m_Time;
    QString curFileName;

    QString srcPath;
    QString fileName;
    QString destPath;
};

#endif // BOOK_H
