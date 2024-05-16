#include "mytreeviewmodel.h"


myTreeViewModel::myTreeViewModel(QObject *parent) : QAbstractItemModel(parent), rootItem(nullptr)
{

}

myTreeViewModel::~myTreeViewModel()
{
    delete rootItem;
}

void myTreeViewModel::appendChildNodeToParent
    (const QModelIndex &parentIndex, const QHash<NodeItem::Roles, QVariant> &data)
{
    if (rootItem)
    {
        //获取传入的类型的对应的枚举值
        const auto type = static_cast<NodeItem::Type>(data[NodeItem::Roles::ItemType].toInt());

        //如果是文件夹项类型
        if (type == NodeItem::Type::FolderItem)
        {
            //获取父节点内部成员变量的指针
            auto parentItem = static_cast<NodeTreeItem *>(parentIndex.internalPointer());
            //父节点为空或者传入的就是根节点
            if (!parentItem || parentItem == rootItem)
            {

                parentItem = rootItem;
                int row = 0;

                //遍历所有子节点
                for (int i = 0; i < parentItem->childCount(); ++i)
                {
                    //获取子节点的项和类型
                    auto childItem = parentItem->child(i);
                    auto childType = static_cast<NodeItem::Type>
                        (childItem->data(NodeItem::Roles::ItemType).toInt());

                    //如果类型是文件夹项并且子项的ID就是默认文件夹ID
                    if (childType == NodeItem::Type::FolderItem
                        &&childItem->data(NodeItem::Roles::NodeId).toInt()
                               == SpecialNodeID::DefaultNotesFolder)
                    {
                        row = i + 1; //获取某个行号
                        break;
                    }
                }

                emit layoutAboutToBeChanged();           //布局即将发生改变的信号，qt自带
                beginInsertRows(parentIndex, row, row);  //开始插入某行，qt自带

                auto nodeItem = new NodeTreeItem(data, parentItem);
                parentItem->insertChild(row, nodeItem);

                endInsertRows();                         //结束插入行,qt自带
                emit layoutChanged();                    //布局发出改变的信号，qt自带

                emit topLevelItemLayoutChanged();        //发出顶级项的布局改变的信号
                //更新folderitem类型的子节点的相对位置
                updateChildRelativePosition(parentItem, NodeItem::Type::FolderItem);
            }

            else  //发出开始插入行的信号，插入，结束插入，更新子节点相对布局的位置
            {
                beginInsertRows(parentIndex, 0, 0);

                auto nodeItem = new NodeTreeItem(data, parentItem);
                parentItem->insertChild(0, nodeItem);

                endInsertRows();

                updateChildRelativePosition(parentItem, NodeItem::Type::FolderItem);
            }


        }
    }
}

void myTreeViewModel::updateChildRelativePosition(NodeTreeItem *parent, const NodeItem::Type type)
{
    int relId = 0;
    //遍历所有子节点
    for (int i = 0; i < parent->childCount(); ++i)
    {
        auto child = parent->child(i);
        auto childType =  //获取子节点类型
            static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt());

        //如果是指定类型的节点
        if (childType == type)
        {
            //对指定类型进行分类处理
            if (type == NodeItem::Type::FolderItem)
            {
                emit requestUpdateNodeRelativePosition(child->data(NodeItem::Roles::NodeId).toInt(),
                                                       relId);
                ++relId;
            }
            else
            {
                qDebug() << __FUNCTION__ << "Wrong type";
                return;
            }

        }
    }
}



QModelIndex myTreeViewModel::folderIndexFromIdPath(const NodePath &idPath)
{
    if (rootItem==nullptr)
    {
        return QModelIndex();
    }


    auto ps = idPath.separate();    //将节点路径拆分
    auto item = rootItem;
    for (const auto &ite : qAsConst(ps))
    {
        bool ok = false;
        auto id = ite.toInt(&ok);   //看看id是否存在，能强转成int

        if (ok==false)//不能
        {
            qDebug() << __FUNCTION__ << "Can't convert to id" << ite;
            return QModelIndex();
        }

        //如果id等于根节点id
        if (id == static_cast<int>(item->data(NodeItem::Roles::NodeId).toInt()))
        {
            continue;
        }


        //遍历所有子节点
        bool foundChild = false;
        for (int i = 0; i < item->childCount(); ++i)
        {
            auto child = item->child(i);

            //子节点不是文件夹，跳过继续
            if (static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt())
                != NodeItem::FolderItem)
            {
                continue;
            }

            //子节点id等于所需id
            if (id == static_cast<int>(child->data(NodeItem::Roles::NodeId).toInt()))
            {
                item = child;           //item代指这个子节点
                foundChild = true;      //找到了子节点，不用打印错误信息
                break;                  //id唯一，找到了就退出
            }
        }

        if (!foundChild)
        {
            //            qDebug() << __FUNCTION__ << "Can't find child id" << id << "inside parent"
            //                     << static_cast<int>(item->data(NodeItem::Roles::NodeId).toInt());
            return QModelIndex();
        }

    }
    //找到了
    return createIndex(item->row(), 0, item);   //范围一个索引，这一行，这个item

}


QString myTreeViewModel::getNewFolderPlaceholderName(const QModelIndex &parentIndex)
{
    QString result = "New Folder";
    if (parentIndex.isValid())
    {
        //获取父节点指针
        auto parentItem = static_cast<NodeTreeItem *>(parentIndex.internalPointer());
        if (parentItem)
        {
            //匹配类似于New Folder (1)这样的表达式
            QRegularExpression reg(R"(^New Folder\s\((\d+)\))");


            int n = 0;  //记录文件夹编号

            //遍历所有子节点
            for (int i = 0; i < parentItem->childCount(); ++i)
            {
                auto child = parentItem->child(i);
                auto title = child->data(NodeItem::Roles::DisplayText).toString();//项显示的内容

                //检查文本是否相同且不区分大小写
                if (title.compare("New Folder", Qt::CaseInsensitive) == 0 && n == 0)
                {
                    n = 1;
                }

                auto match = reg.match(title);
                if (match.hasMatch())  //是否匹配
                {
                    auto cn = match.captured(1).toInt();  //提取正则表达式中括号中的内容，也就是文件夹编号
                    if (n <= cn)  //新建一个最新的文件夹，编号自然要比以前的最大值+1
                    {
                        n = cn + 1;
                    }
                }
            }

            //需要添加(n)后缀
            if (n != 0)
            {
                result = QStringLiteral("New Folder (%1)").arg(QString::number(n));
            }

        }
    }
    return result;
}

QModelIndex myTreeViewModel::rootIndex() const
{
    return createIndex(0, 0, rootItem);
}


QModelIndex myTreeViewModel::getAllNotesButtonIndex()
{
    if (rootItem)
    {
        for (int i = 0; i < rootItem->childCount(); ++i)
        {
            auto child = rootItem->child(i);

            auto type = static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt());
            if (type == NodeItem::Type::AllNoteButton)
            {
                return createIndex(i, 0, child);  //这个按钮唯一，直接返回
            }
        }
    }
    return QModelIndex{};
}

QModelIndex myTreeViewModel::getTrashButtonIndex()
{
    if (rootItem)
    {
        for (int i = 0; i < rootItem->childCount(); ++i)
        {
            auto child = rootItem->child(i);

            auto type = static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt());
            if (type == NodeItem::Type::TrashButton)
            {
                return createIndex(i, 0, child);  //这个按钮唯一，直接返回
            }
        }
    }
    return QModelIndex{};
}



QModelIndex myTreeViewModel::getDefaultNotesIndex()
{
    if (rootItem)  //根节点不为空
    {
        for (int i = 0; i < rootItem->childCount(); ++i)
        {
            //获取根节点的类型
            auto child = rootItem->child(i);
            auto type = static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt());


            //是文件夹类型并且枚举ID等于默认笔记文件夹
            if (type == NodeItem::Type::FolderItem
                && child->data(NodeItem::Roles::NodeId).toInt() == SpecialNodeID::DefaultNotesFolder)
            {
                return createIndex(i, 0, child);
            }
        }
    }
    return QModelIndex{};
}

QVector<QModelIndex> myTreeViewModel::getSeparatorIndex()
{
    QVector<QModelIndex> result;

    if (rootItem)
    {
        //获取子节点类型，如果是分隔符类型，添加，最后返回vector
        for (int i = 0; i < rootItem->childCount(); ++i)
        {
            auto child = rootItem->child(i);
            auto type = static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt());
            if (type == NodeItem::Type::FolderSeparator)
            {
                result.append(createIndex(i, 0, child));
            }
        }
    }

    return result;
}

void myTreeViewModel::deleteRow(const QModelIndex &rowIndex, const QModelIndex &parentIndex)
{
    //相关信息
    auto type = static_cast<NodeItem::Type>(rowIndex.data(NodeItem::Roles::ItemType).toInt());
    auto id = rowIndex.data(NodeItem::Roles::NodeId).toInt();

    //需要同时满足时文件夹类型并且不是默认笔记文件夹(相当于id>)
    if (!(type == NodeItem::Type::FolderItem && id > SpecialNodeID::DefaultNotesFolder))
    {
        qDebug() << "Can not delete this row with id" << id;
        return;
    }

    //相关信息
    auto item = static_cast<NodeTreeItem *>(rowIndex.internalPointer());
    int row = item->row();
    auto parentItem = static_cast<NodeTreeItem *>(parentIndex.internalPointer());


    //为这个项的displaytext属性添加内容
    setData(rowIndex, "deleted", NodeItem::DisplayText);



    //如果传入的父节点索引是根节点，删除这个子节点
    if (parentItem == rootItem)
    {
        beginResetModel();                       //开始清除模型中的数据，model内部会进行删除前的准备操作
        //这里不会造成内存泄漏，remove某一行后，delete后，调用item的析构函数，也会删除这某一行的所有子节点
        parentItem->removeChild(row);            //父节点移除这各子节点
        endResetModel();                        //结束清除，刷新视图等
        emit topLevelItemLayoutChanged();       //发送顶级节点发生改变的信号
    }

    //不是父节点，移出所有子节点及其子节点...
    else
    {
        int count = item->recursiveNodeCount();
        beginRemoveRows(parentIndex, row, row + count - 1);     //model中移除数据
        parentItem->removeChild(row);                           //自定义的数据结构中移除
        endRemoveRows();
    }
}

QModelIndex myTreeViewModel::index(int row, int column, const QModelIndex &parent) const
{
    //不存在这个索引
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }


    NodeTreeItem *parentItem=nullptr;
    //获取父节点的指针
    if (!parent.isValid())
    {
        parentItem = rootItem;
    }
    else
    {
        parentItem = static_cast<NodeTreeItem *>(parent.internalPointer());
    }


    //获取子节点
    NodeTreeItem *childItem = parentItem->child(row);
    if (childItem) //如果这个项不为空就返回索引
    {
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex myTreeViewModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    NodeTreeItem *childItem = static_cast<NodeTreeItem *>(index.internalPointer());
    if ((!childItem) || (childItem == rootItem))
    {
        return QModelIndex();
    }


    NodeTreeItem *parentItem = childItem->parentItem();
    if ((!parentItem) || (parentItem == rootItem))
    {
        return QModelIndex();
    }
    return createIndex(parentItem->row(), 0, parentItem);
}

int myTreeViewModel::rowCount(const QModelIndex &parent) const
{
    NodeTreeItem *parentItem;
    if (parent.column() > 0)
    {
        return 0;
    }

    if (!parent.isValid())
    {
        parentItem = rootItem;
    }
    else
    {
        parentItem = static_cast<NodeTreeItem *>(parent.internalPointer());
    }

    return parentItem->childCount();
}

int myTreeViewModel::columnCount(const QModelIndex &parent) const
{
    //索引合法，发挥索引对应的节点的列数(1)
    if (parent.isValid())
    {
        return static_cast<NodeTreeItem *>(parent.internalPointer())->columnCount();
    }
    return rootItem->columnCount();
}

QVariant myTreeViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    NodeTreeItem *item = static_cast<NodeTreeItem *>(index.internalPointer());//索引对应的项的指针
    //如果是想看看是否可展开
    if (static_cast<NodeItem::Roles>(role) == NodeItem::Roles::IsExpandable)
    {
        return item->childCount() > 0;
    }
    //如果是想看是不是根节点
    if (item->data(NodeItem::Roles::ItemType) == NodeItem::Type::RootItem)
    {
        return QVariant();
    }
    //调用项的类的data函数,哈希，类型对应的哈希值
    return item->data(static_cast<NodeItem::Roles>(role));

}

bool myTreeViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid())
    {
        //传入的枚举类型是"展示内容"，调用节点类的setdata函数，新建哈希键值对
        if (role == NodeItem::Roles::DisplayText)
        {
            static_cast<NodeTreeItem *>(index.internalPointer())
                ->setData(static_cast<NodeItem::Roles>(role), value);

            emit dataChanged(index, index, { role });
            return true;
        }

        //传入的是"子节点数"枚举类型
        if (role == NodeItem::Roles::ChildCount)
        {
            static_cast<NodeTreeItem *>(index.internalPointer())
                ->setData(static_cast<NodeItem::Roles>(role), value);

            emit dataChanged(index, index, { role });
            return true;
        }
    }
    return false;
}

Qt::ItemFlags myTreeViewModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::ItemIsDropEnabled; //可拖动
    }

    //索引包含的标志+可拖动+可放置
    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

Qt::DropActions myTreeViewModel::supportedDragActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions myTreeViewModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QStringList myTreeViewModel::mimeTypes() const
{
    return QStringList()<<FOLDER_MIME;
}

QMimeData *myTreeViewModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty()) //列表为空
    {
        return nullptr;
    }
    //获取第一个索引的节点的类型
    auto itemType = static_cast<NodeItem::Type>(indexes[0].data(NodeItem::Roles::ItemType).toInt());

    //文件夹项类型
    if (itemType == NodeItem::Type::FolderItem)
    {
        const auto &index = indexes[0];
        //获取id(哈希值)与绝对路径
        auto id = index.data(NodeItem::Roles::NodeId).toInt();
        auto absPath = index.data(NodeItem::Roles::AbsPath).toString();

        //如果是默认笔记文件夹
        if (id == SpecialNodeID::DefaultNotesFolder)
        {
            return nullptr;
        }

        //将绝对路径以toUtf8字节流的形式存储进data中并返回
        QMimeData *mimeData = new QMimeData;
        mimeData->setData(FOLDER_MIME, absPath.toUtf8());
        return mimeData;
    }

    else
    {
        return nullptr;
    }
}


