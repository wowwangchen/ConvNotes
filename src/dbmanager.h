#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include<QVector>
#include<QMap>
#include<QDebug>
#include<QString>
#include<QFile>
#include<QtSql/QSqlDatabase>
#include<QSqlQuery>
#include<QSqlError>
#include"nodedata.h"
#include"nodepath.h"

#define DEFAULT_DATABASE_NAME "default_database"

struct NodeTagTreeData
{
    QVector<NodeData> nodeTreeData;
    //QVector<TagData> tagTreeData;
};

using FolderListType = QMap<int, QString>;



//数据库管理类，本地数据库，设置一个数据库的默认地址，每次都连接那个数据库
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
    //将某个节点移入垃圾桶
    Q_INVOKABLE void moveFolderToTrash(const NodeData &node);
    //获取文件夹列表的键值对
    Q_INVOKABLE FolderListType getFolderList();

private:
    //打开数据库
    void open(const QString &path, bool doCreate = false);
    //创建表
    void createTables();
    //更新子节点数量，数据一致性保证
    void recalculateChildNotesCount();
    //更新所有笔记(根节点的子孩子)的数量，数据一致性保证
    void recalculateChildNotesCountAllNotes();
    //更新某个文件夹id对应的子节点的数量，数据一致性保证
    void recalculateChildNotesCountFolder(int folderId);
    //指定文件夹id的子节点+1
    void increaseChildNotesCountFolder(int folderId);
    //指定文件夹id的子节点-1
    void decreaseChildNotesCountFolder(int folderId);
    //传入的节点是否存在
    bool isNodeExist(const NodeData &node);
    //获取所有文件夹的节点
    QVector<NodeData> getAllFolders();
    //更新某个节点(相关参数)
    bool updateNoteContent(const NodeData &note);
    //获取旧版本的notebook
    QList<NodeData> readOldNBK(const QString &fileName);
    //为新的子节点获取下一个可用的位置
    int nextAvailablePosition(int parentId, NodeData::Type nodeType);
    //在数据库中添加一条记录
    int addNodePreComputed(const NodeData &node);

signals:
    void nodesTagTreeReceived(const NodeTagTreeData &treeData);
    void childNotesCountUpdatedFolder(int folderId, const QString &path, int childCount);


public slots:
    void renameNode(int id, const QString &newName);
    void moveNode(int nodeId, const NodeData &target);
    void updateRelPosNode(int nodeId, int relPos);
    int addNode(const NodeData &node);
    int nextAvailableNodeId();

private:
    QString      m_dbpath;      //数据库存储的路径
    QSqlDatabase m_db;          //数据库
};

#endif // DBMANAGER_H
