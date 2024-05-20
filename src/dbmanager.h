#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include<QVector>
#include<QMap>
#include"nodedata.h"
#include"nodepath.h"

struct NodeTagTreeData
{
    QVector<NodeData> nodeTreeData;
    //QVector<TagData> tagTreeData;
};
using FolderListType = QMap<int, QString>;



//数据库管理类，本地数据库
class DBManager :  public QObject
{
    Q_OBJECT
public:
    explicit DBManager(QObject *parent = nullptr);
    //声明可通过元对象系统调用函数
    //获取节点绝对路径
    Q_INVOKABLE NodePath getNodeAbsolutePath(int nodeId);
    //通过id获取节点
    Q_INVOKABLE NodeData getNode(int nodeId);
    //将某个节点移入垃圾
    Q_INVOKABLE void moveFolderToTrash(const NodeData &node);
    //获取文件夹列表的键值对
    Q_INVOKABLE FolderListType getFolderList();

signals:
    void nodesTagTreeReceived(const NodeTagTreeData &treeData);
    void childNotesCountUpdatedFolder(int folderId, const QString &path, int childCount);

public slots:
    void renameNode(int id, const QString &newName);
    void moveNode(int nodeId, const NodeData &target);
    void updateRelPosNode(int nodeId, int relPos);
};

#endif // DBMANAGER_H
