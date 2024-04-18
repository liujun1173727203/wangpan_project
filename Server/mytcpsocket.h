#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include <QString>
#include <QFile>
#include <QTimer>
class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
signals:
    void offline(MyTcpSocket *socket);
public:
    explicit MyTcpSocket(QObject *parent = nullptr);
    QString getName();
    void saveDir(const QString recvpath,const QString sendpath);
public slots:
    void recvMsg();
    void clientOffline();
    void DownloadFile();

private:
    QString m_name;
    QFile mfile;
    qint64 mTotal;
    qint64 m_iRecved;
    bool isUpload;
    QTimer *m_timer;
};

#endif // MYTCPSOCKET_H
