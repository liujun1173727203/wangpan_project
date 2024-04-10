#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include <QString>
class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
signals:
    void offline(MyTcpSocket *socket);
public:
    explicit MyTcpSocket(QObject *parent = nullptr);
    QString getName();
public slots:
    void recvMsg();
    void clientOffline();

private:
    QString m_name;
};

#endif // MYTCPSOCKET_H
