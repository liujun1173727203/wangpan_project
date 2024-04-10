#ifndef DAO_H
#define DAO_H

#include <QObject>
#include <QSqlDatabase>
#include<QSqlQuery>
class DAO : public QObject
{
    Q_OBJECT
public:
    explicit DAO(QObject *parent = nullptr);
    static DAO& getInstance();
    void init();
    bool handleRegist(const char *name,const char *pwd);
    bool handleLogin(const char *name,const char *pwd);
    bool handleLoginout(const char *name);
    QStringList handleAllOnline();
    int handleSearch(const char * name);
    int handleAddFriend(const char * name,const char * fname);
    void handleaddfriend(const char * name,const char * fname);
    QStringList flushfriend(const char * name);
    bool delFriend(const char * name,const char * fname);

private:
    QSqlDatabase m_db;
};

#endif // DAO_H
