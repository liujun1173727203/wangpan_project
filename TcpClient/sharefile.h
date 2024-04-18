#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include<QPushButton>
#include<QHBoxLayout>
#include<QVBoxLayout>
#include<QButtonGroup>
#include<QScrollArea>
#include<QListWidget>
class sharefile : public QWidget
{
    Q_OBJECT
public:
    explicit sharefile(QWidget *parent = nullptr);
    static sharefile &getShareFile();
    void updatefriend(QListWidget *friendlist);
public slots:
    void cancelSelect();
    void SelectAll();
    void OKPB();
    void Cancel();
private:
    QPushButton *mSelectAllPB;
    QPushButton *mCancelAllPB;
    QPushButton *mOkPB;
    QPushButton *mCancelPB;

    QScrollArea *mSA;
    QWidget *mFriendW;
    QVBoxLayout *mFriendWVBL;
    QButtonGroup *mButtonGroup;
signals:
};

#endif // SHAREFILE_H
