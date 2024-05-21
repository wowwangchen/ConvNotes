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
#define OUTSIDE_DATABASE_NAME "outside_database"

struct NodeTagTreeData
{
    QVector<NodeData> nodeTreeData;
    //QVector<TagData> tagTreeData;
};

struct ListViewInfo
{
    bool        isInSearch;
    bool        isInTag;
    QSet<int>   currentTagList;
    int         parentFolderId;
    QSet<int>   currentNotesId;
    bool        needCreateNewNote;
    int         scrollToId;
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
    //获取旧版本的notebook，直接以字符流的形式读取
    QList<NodeData> readOldNBK(const QString &fileName);
    //为新的子节点获取下一个可用的位置
    int nextAvailablePosition(int parentId, NodeData::Type nodeType);
    //在数据库中添加一条预先处理好的节点数据
    int addNodePreComputed(const NodeData &node);

signals:
    void nodesTagTreeReceived(const NodeTagTreeData &treeData);
    void childNotesCountUpdatedFolder(int folderId, const QString &path, int childCount);
    void notesListReceived(const QVector<NodeData> &noteList, const ListViewInfo &inf);
    void showErrorMessage(const QString &title, const QString &content);


public slots:
    //添加节点(很多参数都未确定，要在函数中确定,前面的addNodePreComputed的参数都是确定好的)
    int addNode(const NodeData &node);
    //移除节点
    void removeNote(const NodeData &note);
    //获得笔记列表
    void onNotesListInFolderRequested(int parentID, bool isRecursive, bool newNote = false,
                                      int scrollToId = SpecialNodeID::InvalidNodeId);
    //打开数据库
    void onOpenDBManagerRequested(const QString &path, bool doCreate);
    //创建或者更新某个节点
    void onCreateUpdateRequestedNoteContent(const NodeData &note);
    //从外部文件中导入笔记
    void onImportNotesRequested(const QString &fileName);
    //从指定文件中恢复笔记
    void onRestoreNotesRequested(const QString &fileName);
    //导出笔记
    void onExportNotesRequested(const QString &fileName);
    //修改数据库的路径
    void onChangeDatabasePathRequested(const QString &newPath);
    //下一个可用的节点的数据库中的id的值
    int nextAvailableNodeId();
    //在数据库中重命名节点
    void renameNode(int id, const QString &newName);
    //在数据库中移动节点位置
    void moveNode(int nodeId, const NodeData &target);
    //寻找带有关键字的节点
    void searchForNotes(const QString &keyword, const ListViewInfo &inf);
    //取消搜索
    void clearSearch(const ListViewInfo &inf);
    //更新节点的相对位置
    void updateRelPosNode(int nodeId, int relPos);
    //更新笔记节点的相对位置
    void updateRelPosPinnedNote(int nodeId, int relPos);
    void updateRelPosPinnedNoteAN(int nodeId, int relPos);
    //设置笔记置顶状态
    void setNoteIsPinned(int noteId, bool isPinned);
    //获取某个文件夹的子节点数量
    NodeData getChildNotesCountFolder(int folderId);
    //版本间迁移数据
    //void onMigrateNotesFromV0_9_0Requested(QVector<NodeData> &noteList);
    //void onMigrateTrashFrom0_9_0Requested(QVector<NodeData> &noteList);
    //void onMigrateNotesFrom1_5_0Requested(const QString &fileName);


private:
    QString      m_dbpath;      //数据库存储的路径
    QSqlDatabase m_db;          //数据库
};

#endif // DBMANAGER_H
