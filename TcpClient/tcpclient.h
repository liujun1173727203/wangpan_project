#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include<QFile>
#include"p2pchat.h"
#include<QTcpSocket>
QT_BEGIN_NAMESPACE
namespace Ui {
class TcpClient;
}
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadConfig();
    static TcpClient &getInstance();
    QTcpSocket &getSocket();
    QString getName();
    QMap<QString,P2pchat *> &getChatList();
public slots:
    void showConnect();
    void recvMsg();
private slots:
    // void on_send_clicked();

    void on_login_clicked();

    void on_regist_clicked();

    void on_loginout_clicked();

private:
    Ui::TcpClient *ui;
    QString m_strIP;
    quint16 m_usPort;
    QString m_cName;
    QTcpSocket m_socket;

};
#endif // TCPCLIENT_H
