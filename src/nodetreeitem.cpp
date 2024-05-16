#include "nodetreeitem.h"

NodeTreeItem::NodeTreeItem(const QHash<NodeItem::Roles, QVariant> &data, NodeTreeItem *parentItem)
    : m_itemData(data), m_parentItem(parentItem)
{

}

NodeTreeItem::~NodeTreeItem()
{
    qDeleteAll(m_childItems);
}

void NodeTreeItem::appendChild(NodeTreeItem *child)
{
    m_childItems.append(child);
}

void NodeTreeItem::insertChild(int row, NodeTreeItem *child)
{
    m_childItems.insert(row, child);
}

NodeTreeItem *NodeTreeItem::child(int row)
{
    if (row < 0 || row >= m_childItems.size())
    {
        return nullptr;
    }
    return m_childItems.at(row);
}

void NodeTreeItem::removeChild(int row)
{
    if (row < 0 || row >= m_childItems.size())
    {
        return;
    }
    delete m_childItems.takeAt(row);  //vector的成员函数，移出某个元素并返回拷贝
}

NodeTreeItem *NodeTreeItem::takeChildAt(int row)
{
    if (row < 0 || row >= m_childItems.size())
    {
        return nullptr;
    }
    return m_childItems.takeAt(row);//从vector中移除，并返回这个指针
}

int NodeTreeItem::childCount() const
{
    return m_childItems.count();
}

int NodeTreeItem::columnCount() const
{
    return 1;
}

int NodeTreeItem::recursiveNodeCount() const
{
    int res=1;
    for(const auto& child : m_childItems)
    {
        res+=child->recursiveNodeCount();
    }
    return res;
}

void NodeTreeItem::recursiveUpdateFolderPath(const QString &oldP, const QString &newP)
{
    //获取自己的类型对应的哈希值
    auto type = static_cast<NodeItem::Type>(data(NodeItem::Roles::ItemType).toInt());
    if (type != NodeItem::Type::FolderItem)
    {
        return;
    }


    //绝对路径对应的哈希值
    auto currP = data(NodeItem::Roles::AbsPath).toString();
    //将旧地址对应的区间的内容替换为新地址
    currP.replace(currP.indexOf(oldP), oldP.size(), newP);
    setData(NodeItem::Roles::AbsPath, currP);


    //更新每个子节点的路径
    for (auto &child : m_childItems)
    {
        child->recursiveUpdateFolderPath(oldP, newP);
    }

}

QVariant NodeTreeItem::data(NodeItem::Roles role) const
{
    return m_itemData.value(role, QVariant()); //哈希表
}

void NodeTreeItem::setData(NodeItem::Roles role, const QVariant &d)
{
    m_itemData[role] = d;  //新建哈希项
}

int NodeTreeItem::row() const
{
    //如果父节点不为空，那么范围自己在父节点中的行数，否则返回空
    if (m_parentItem)
    {
        return m_parentItem->m_childItems.indexOf(const_cast<NodeTreeItem *>(this));
    }

    return 0;
}

NodeTreeItem *NodeTreeItem::parentItem()
{
    return m_parentItem;
}

void NodeTreeItem::setParentItem(NodeTreeItem *parentItem)
{
    m_parentItem = parentItem;
}

void NodeTreeItem::moveChild(int from, int to)
{
    m_childItems.move(from, to);
}

void NodeTreeItem::recursiveSort()
{
    //获取自己的类型对应的哈希值
    auto type = static_cast<NodeItem::Type>(data(NodeItem::Roles::ItemType).toInt());

    //定义一个比较函数，根据节点的相对路径升序
    auto relPosComparator = [](const NodeTreeItem *a, const NodeTreeItem *b)
    {
        return a->data(NodeItem::Roles::RelPos).toInt() < b->data(NodeItem::Roles::RelPos).toInt();
    };


    //若节点类型是文件夹项
    if (type == NodeItem::Type::FolderItem)
    {
        //升序，再递归子节点
        std::sort(m_childItems.begin(), m_childItems.end(), relPosComparator);
        for (auto &child : m_childItems)
        {
            child->recursiveSort();
        }
    }


    //如果是根节点
    else if (type == NodeItem::Type::RootItem)
    {
        //获得所有类型的vector变量
        QVector<NodeTreeItem *> allNoteButton, trashFolder, folderSep, folderItems;

        //对自己的子节点遍历
        for (const auto child : qAsConst(m_childItems))
        {
            auto childType =   //获取子节点类型，然后根据类型添加到集合中
                static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt());

            if (childType == NodeItem::Type::AllNoteButton)             //"所有笔记"按钮
            {
                allNoteButton.append(child);
            }
            else if (childType == NodeItem::Type::TrashButton)          //垃圾桶
            {
                trashFolder.append(child);
            }
            else if (childType == NodeItem::Type::FolderSeparator)      //文件夹分隔符
            {
                folderSep.append(child);
            }
            else if (childType == NodeItem::Type::FolderItem)           //文件夹项
            {
                folderItems.append(child);
            }
            else
            {
                qDebug() << __FUNCTION__ << "wrong child type " << static_cast<int>(childType);
            }
        }


        //清空自己的子节点，然后将文件夹项排序，然后依次添加到子节点中
        m_childItems.clear();

        std::sort(folderItems.begin(), folderItems.end(), relPosComparator);
        for (auto &child : folderItems)
        {
            child->recursiveSort();
        }

        m_childItems.append(allNoteButton);
        m_childItems.append(trashFolder);
        m_childItems.append(folderSep);
        m_childItems.append(folderItems);
    }
}

