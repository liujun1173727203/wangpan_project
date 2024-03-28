#ifndef DAO_H
#define DAO_H

#include <QObject>

class DAO : public QObject
{
    Q_OBJECT
public:
    explicit DAO(QObject *parent = nullptr);

signals:
};

#endif // DAO_H
