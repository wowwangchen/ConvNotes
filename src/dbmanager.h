#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include<QVector>
#include"nodedata.h"
struct NodeTagTreeData
{
    QVector<NodeData> nodeTreeData;
    //QVector<TagData> tagTreeData;
};


//数据库管理类
class DBManager
{
public:
    DBManager();
};

#endif // DBMANAGER_H
