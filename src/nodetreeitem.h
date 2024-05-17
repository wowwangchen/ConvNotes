#ifndef NODETREEITEM_H
#define NODETREEITEM_H

#include <QObject>
#include <QHash>
#include <QVector>
#include<QDebug>

//树形结构的节点项空间，所有节点的类型
namespace NodeItem
{
    // We store this enum inside QVariant,
    // and an invalid QVariant conversion return 0\

//对不同的项分类
enum Type
{
    AllNoteButton = 1,   //"所有笔记"按钮项
    TrashButton,         //垃圾桶
    FolderSeparator,     //文件夹分隔线
    //TagSeparator,
    FolderItem,          //文件夹项
    NoteItem,            //笔记项
    //TagItem,
    RootItem             //根节点项
};


//对项的不同属性进行分类
enum Roles
{
    ItemType = Qt::UserRole,        //项的类型
    DisplayText = Qt::DisplayRole,  //项显示的内容
    Icon = Qt::DecorationRole,      //项的图标
    FolderColor = Qt::UserRole + 1,    //项的标签颜色
    IsExpandable,                   //是否可展开
    AbsPath,                        //绝对路径 absolute
    RelPos,                         //相对路径 relative
    ChildCount,                     //子项的数量
    NodeId                          //节点ID
};
}




//此类是数据模型中的一项，包含某个点与操作其的函数
class NodeTreeItem
{
public:
    //传入哈希表(与父节点)
    explicit NodeTreeItem(const QHash<NodeItem::Roles, QVariant> &data,
                          NodeTreeItem *parentItem = nullptr);
    ~NodeTreeItem();
    //添加子节点
    void appendChild(NodeTreeItem *child);
    //在某个位置插入节点
    void insertChild(int row, NodeTreeItem *child);
    //获取某个子节点
    NodeTreeItem *child(int row);
    //移出某个子节点
    void removeChild(int row);
    //移出某个节点，并返回这个节点的指针
    NodeTreeItem *takeChildAt(int row);
    //获取子节点的数量
    int childCount() const;
    //获取列数
    int columnCount() const;
    //递归，获取所有节点(包括子节点和子节点的子节点...等)
    int recursiveNodeCount() const;
    //递归更新文件夹(包括子节点)地址
    void recursiveUpdateFolderPath(const QString &oldP, const QString &newP);
    //根据枚举类型获取哈希值
    QVariant data(NodeItem::Roles role) const;
    //新建哈希项
    void setData(NodeItem::Roles role, const QVariant &d);
    //返回自己所处的行数
    int row() const;
    //返回父节点指针
    NodeTreeItem *parentItem();
    //指定父节点
    void setParentItem(NodeTreeItem *parentItem);
    //将索引为from的子项移动到位置to
    void moveChild(int from, int to);
    //对整个项目根据文件路径升序排序
    void recursiveSort();




private:
    QHash<NodeItem::Roles, QVariant>    m_itemData;         //自己的项的属性，建立哈希映射
    NodeTreeItem*                       m_parentItem;       //父节点
    QVector<NodeTreeItem*>               m_childItems;      //子节点
};


#endif // NODETREEITEM_H
