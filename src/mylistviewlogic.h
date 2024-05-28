#ifndef MYLISTVIEWLOGIC_H
#define MYLISTVIEWLOGIC_H

#include <QObject>

//联合文件列表的MVD与数据库
class myListViewLogic : public QObject
{
    Q_OBJECT
public:
    explicit myListViewLogic(QObject *parent = nullptr);

signals:

};

#endif // MYLISTVIEWLOGIC_H
