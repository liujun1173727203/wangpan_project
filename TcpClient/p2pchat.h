#ifndef P2PCHAT_H
#define P2PCHAT_H

#include <QWidget>
#include"protocol.h"
namespace Ui {
class P2pchat;
}

class P2pchat : public QWidget
{
    Q_OBJECT

public:
    explicit P2pchat(QWidget *parent = nullptr);
    ~P2pchat();
    void setChatName(QString name);
    void setmyName(QString name);
    void recvChatMsg(const char * msg);
private slots:
    void on_sendmsg_clicked();

private:
    Ui::P2pchat *ui;
    QString chatName;
    QString myName;
};

#endif // P2PCHAT_H
