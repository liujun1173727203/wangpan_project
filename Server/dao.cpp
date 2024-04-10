#include "dao.h"
#include<QDebug>
DAO::DAO(QObject *parent)
    : QObject{parent}
{
    m_db=QSqlDatabase::addDatabase("QMYSQL");
}

DAO &DAO::getInstance()
{
    static DAO instance;
    return instance;
}

void DAO::init()
{
    m_db.setHostName("localhost");
    m_db.setPort(3306);
    m_db.setUserName("root");
    m_db.setPassword("123456");
    m_db.setDatabaseName("wangpan");
    if(!m_db.open()){
        qDebug()<<"error";
    }
}

bool DAO::handleRegist(const char *name, const char *pwd)
{
    if(NULL ==name || NULL == pwd){
        return false;
    }
    QSqlQuery query;
    QString sql=QString("insert into Users(username,pwd) values('%1','%2');").arg(name).arg(pwd);
    qDebug()<<sql;
    return query.exec(sql);
}

bool DAO::handleLogin(const char *name, const char *pwd)
{
    if(NULL ==name || NULL == pwd){
        return false;
    }
    QSqlQuery query;
    QString sql=QString("select * from Users where username='%1' and pwd='%2' and online = 0;").arg(name).arg(pwd);
    query.exec(sql);
    qDebug()<<sql;
    if(query.next()){
        sql=QString("update Users set online =1 where username='%1' and pwd='%2';").arg(name).arg(pwd);
        qDebug()<<sql;
        query.exec(sql);
    return true;
    }else{
        return false;
    };

}

bool DAO::handleLoginout(const char *name)
{
    if(NULL ==name){
        return false;
    }
     QSqlQuery query;
     QString sql=QString("update Users set online =0 where username='%1';").arg(name);
     qDebug()<<sql;
     query.exec(sql);
     return true;
}

QStringList DAO::handleAllOnline()
{
    QSqlQuery query;
    QString sql=QString("select * from Users where online= 1;");
    qDebug()<<sql;
    query.exec(sql);
    QStringList lists;
    lists.clear();
    while(query.next()){
        lists.append(query.value(1).toString());
    }
    return lists;

}

int DAO::handleSearch(const char *name)
{
    if(name ==NULL)return -1;
    QSqlQuery query;
    QString sql=QString("select online from Users where username='%1';").arg(name);
    qDebug()<<sql;
    query.exec(sql);
    if(!query.next()){
        return -1;
    }
    if(query.value(0).toInt()){
        return 1;
    }
    return 0;
}

int DAO::handleAddFriend(const char *name, const char *fname)
{
    if(name ==NULL || fname== NULL)return -1;
    QSqlQuery query;
    QString sql=QString("select online from Users where username='%1';").arg(fname);
    qDebug()<<sql;
    query.exec(sql);
    if(!query.next()) return -2;    //不存在
    if(!query.value(0).toInt())return -3;   //buzaixian
    sql=QString("select * from friends where \
    uid=(select id from Users where username='%1) and \
    friendid = (select id from Users where username='%2);").arg(name).arg(fname);
    query.exec(sql);
    if(!query.next())return -1; //不是好友
    return 0;

}

void DAO::handleaddfriend(const char *name, const char *fname)
{
    QString sql=QString("insert into friends(uid,friendid) values('%1','%2');");
    QSqlQuery query;
    QString sql_name=QString("select id from Users where username='%1';");
    query.exec(sql_name.arg(name));
    query.next();
    int nid=query.value(0).toInt();
    query.exec(sql_name.arg(fname));
    query.next();
    int fnid=query.value(0).toInt();
    query.exec(sql.arg(nid).arg(fnid));
    query.exec(sql.arg(fnid).arg(nid));
}

QStringList DAO::flushfriend(const char *name)
{   QSqlQuery q;
    QString sql=QString("select username from Users where online =1 and \
            id in (select friendid from friends where \
            uid =(select id from Users where username='%1'));").arg(name);
    q.exec(sql);
    QStringList friends;
    while(q.next()){
        friends.append(q.value(0).toString());
    };
    return friends;
}

bool DAO::delFriend(const char *name,const char * fname)
{
    QSqlQuery query;
    QString s =QString("select * from friends where uid =(select id from Users where username='%1') and \
friendid =(select id from Users where username='%2');");
    query.exec(s.arg(name).arg(fname));
    if(!query.next())return false;
    QString sql=QString("delete from friends where uid =(select id from Users where username='%1') and \
friendid =(select id from Users where username='%2');");

    qDebug()<<sql.arg(name).arg(fname);
    query.exec(sql.arg(name).arg(fname));
    query.exec(sql.arg(fname).arg(name));
    return true;
}
