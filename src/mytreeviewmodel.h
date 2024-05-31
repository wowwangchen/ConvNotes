#ifndef MYTREEVIEWMODEL_H
#define MYTREEVIEWMODEL_H

#include <QObject>
#include<QAbstractItemModel>
#include <QHash>
#include <QVector>
#include<QDebug>
#include<QRegularExpression>
#include<QMimeData>
#include"nodepath.h"
#include"nodetreeitem.h"
#include"nodedata.h"
#include"dbmanager.h"
#include "codetranslate.h"




//此类是用于管理数据的一个模型，包含众多nodeTreeItem，并管理众多项
class myTreeViewModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit myTreeViewModel(QObject *parent = nullptr);
     ~myTreeViewModel();
    //添加子节点到父节点，传入父节点，类型
    void appendChildNodeToParent(const QModelIndex &parentIndex,
                                 const QHash<NodeItem::Roles, QVariant> &data);
    //从IDpath中获取文件夹的索引
    QModelIndex folderIndexFromIdPath(const NodePath &idPath);
    //获取新文件夹的占位名称
    QString getNewFolderPlaceholderName(const QModelIndex &parentIndex);
    //获取根节点的索引
    QModelIndex rootIndex() const;
    //获取"所有笔记"按钮的索引
    QModelIndex getAllNotesButtonIndex();
    //获取垃圾桶节点的索引
    QModelIndex getTrashButtonIndex();
    //获取默认笔记的索引
    QModelIndex getDefaultNotesIndex();
    //获取分隔符的索引
    QVector<QModelIndex> getSeparatorIndex();
    //删除某一行
    void deleteRow(const QModelIndex &rowIndex, const QModelIndex &parentIndex);

public:
    //给出行、列、父亲，返回指定索引
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    //获取某个节点的父节点
    virtual QModelIndex parent(const QModelIndex &index) const override;
    //获取某个节点下的行数
    virtual int rowCount(const QModelIndex &parent) const override;
    //获取某个节点下的列数
    virtual int columnCount(const QModelIndex &parent) const override;
    //返回所需数据
    virtual QVariant data(const QModelIndex &index, int role) const override;
    //设置某个索引的指定内容
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    //返回某个索引对应的项的标志
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    //指定数据模型支持的操作类型
    virtual Qt::DropActions supportedDragActions() const override;
    virtual Qt::DropActions supportedDropActions() const override;
    //mime类型支持的列表
    virtual QStringList mimeTypes() const override;
    //获取绝对路径并返回MimeData
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
    //处理拖放操作
    virtual bool dropMimeData(const QMimeData *mime, Qt::DropAction action, int row, int column,
                              const QModelIndex &parent) override;

public slots:
    void setTreeData(const NodeTagTreeData &treeData);


signals:
    //顶级项的布局发生改变
    void topLevelItemLayoutChanged();
    //请求改变节点相对位置的信号
    void requestUpdateNodeRelativePosition(int nodeId, int relativePosition);
    //将文件夹移入垃圾桶
    void requestMoveFolderToTrash(const QModelIndex &index);
    //文件夹拖放完成的信号
    void dropFolderSuccessful(const QString &paths);
    //请求更新绝对路径
    void requestUpdateAbsPath(const QString &oldPath, const QString &newPath);
    //请求展开某个索引的位置
    void requestExpand(const QString &indexPath);
    //请求移动节点位置
    void requestMoveNode(int nodeId, int targetId);

private:
    //为传入的父节点与子节点信息，为其新建子节点
    void loadNodeTree(const QVector<NodeData> &nodeData, NodeTreeItem *rootNode);
    //添加"所有笔记按钮"和垃圾桶按钮
    void appendAllNotesAndTrashButton(NodeTreeItem *rootNode);
    //添加文件夹间的分隔符
    void appendFolderSeparator(NodeTreeItem *rootNode);
    //更新指定类型的子节点的相对位置
    void updateChildRelativePosition(NodeTreeItem *parent, const NodeItem::Type type);




private:
    NodeTreeItem *rootItem;     //根节点(也是一个普通节点)
};

#endif // MYTREEVIEWMODEL_H
