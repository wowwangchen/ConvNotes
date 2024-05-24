#include "mytreeviewlogic.h"
myTreeViewLogic::myTreeViewLogic(myTreeView *treeView, myTreeViewModel *treeModel,
                                 DBManager *dbManager, myListView *listView, QObject *parent)
:   QObject(parent),
    m_treeView{ treeView },
    m_treeModel{ treeModel },
    m_listView{ listView },
    m_dbManager{ dbManager },
    m_needLoadSavedState{ false },
    m_isLastSelectFolder{ true },
    m_lastSelectFolder{},
    m_expandedFolder{}

{
    m_treeDelegate =new myTreeViewDelegate(m_treeView,m_treeView,m_listView);
    m_treeView->setItemDelegate(m_treeDelegate);
    initConnect();


}

void myTreeViewLogic::openFolder(int id)
{
    NodeData target; //目标节点
    //在指定对象上调用方法，指定调用方式，返回值和传入参数
    //这个id是数据库中的id
    QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(NodeData, target), Q_ARG(int, id));

    //节点不是文件夹类型
    if (target.nodeType() != NodeData::Folder)
    {
        qDebug() << __FUNCTION__ << "Target is not folder!";
        return;
    }
    //id是垃圾桶文件夹
    if (target.id() == SpecialNodeID::TrashFolder)
    {
        m_treeView->setCurrentIndexC(m_treeModel->getTrashButtonIndex()); //设置当前索引
    }
    //id是根节点
    else if (target.id() == SpecialNodeID::RootFolder)
    {
        m_treeView->setCurrentIndexC(m_treeModel->getAllNotesButtonIndex());
    }


    else
    {
        auto index = m_treeModel->folderIndexFromIdPath(target.absolutePath());//获取target节点对应的索引
        if (index.isValid())
        {
            m_treeView->setCurrentIndexC(index);
        }
        else
        {
            m_treeView->setCurrentIndexC(m_treeModel->getAllNotesButtonIndex());
        }
    }
}

void myTreeViewLogic::onMoveNodeRequested(int nodeId, int targetId)
{
    NodeData target;

    //获取targetID对应的节点数据
    QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(NodeData, target), Q_ARG(int, targetId));
    //移入的节点不是文件夹类型，退出
    if (target.nodeType() != NodeData::Folder)
    {
        qDebug() << __FUNCTION__ << "Target is not folder!";
        return;
    }

    //发送在数据库中移动的信号
    emit requestMoveNodeInDB(nodeId, target);
}

void myTreeViewLogic::setTheme(Theme::Value theme)
{
    m_treeView->setTheme(theme);
    m_treeDelegate->setTheme(theme);
    m_style->setTheme(theme);
}

void myTreeViewLogic::setLastSavedState(bool isLastSelectFolder, const QString &lastSelectFolder, const QSet<int> &lastSelectTag, const QStringList &expandedFolder)
{
    m_isLastSelectFolder = isLastSelectFolder;
    m_lastSelectFolder = lastSelectFolder;
    m_expandedFolder = expandedFolder;
    m_needLoadSavedState = true;
    //m_lastSelectTags = lastSelectTag;
}

void myTreeViewLogic::updateTreeViewSeparator()
{
    //调用treeView的set
    m_treeView->setTreeSeparator(m_treeModel->getSeparatorIndex(),
                                 m_treeModel->getDefaultNotesIndex());
}

void myTreeViewLogic::loadTreeModel(const NodeTagTreeData &treeData)
{
    m_treeModel->setTreeData(treeData);

    {
        NodeData node;
        //元对象系统调用函数
        QMetaObject::invokeMethod(m_dbManager, "getChildNotesCountFolder",
                                  Qt::BlockingQueuedConnection, Q_RETURN_ARG(NodeData, node),
                                  Q_ARG(int, SpecialNodeID::RootFolder));

        //所有笔记按钮
        auto index = m_treeModel->getAllNotesButtonIndex();
        if (index.isValid())
        {
            m_treeModel->setData(index, node.childNotesCount(), NodeItem::Roles::ChildCount);
        }
    }
    //垃圾桶按钮
    {
        NodeData node;
        QMetaObject::invokeMethod(m_dbManager, "getChildNotesCountFolder",
                                  Qt::BlockingQueuedConnection, Q_RETURN_ARG(NodeData, node),
                                  Q_ARG(int, SpecialNodeID::TrashFolder));
        auto index = m_treeModel->getTrashButtonIndex();
        if (index.isValid())
        {
            m_treeModel->setData(index, node.childNotesCount(), NodeItem::Roles::ChildCount);
        }
    }

    //如果需要加载保存的状态
    if (m_needLoadSavedState)
    {
        m_needLoadSavedState = false;
        m_treeView->reExpandC(m_expandedFolder); //需要展开的文件夹

        //如果上次选择的是文件夹
        if (m_isLastSelectFolder)
        {
            QModelIndex index;
            //判断上次选择的文件夹是哪种文件夹
            if (m_lastSelectFolder == NodePath::getAllNoteFolderPath())
            {
                index = m_treeModel->getAllNotesButtonIndex();
            }
            else if (m_lastSelectFolder == NodePath::getTrashFolderPath())
            {
                index = m_treeModel->getTrashButtonIndex();
            }
            else
            {
                index = m_treeModel->folderIndexFromIdPath(m_lastSelectFolder);
            }
            //都不是
            if (index.isValid())
            {

                m_treeView->setCurrentIndexC(index);
            }
            else
            {
                m_treeView->setCurrentIndexC(m_treeModel->getAllNotesButtonIndex());
            }
        }
        m_lastSelectFolder.clear();
        m_expandedFolder.clear();
    }
    else
    {
        m_treeView->setCurrentIndexC(m_treeModel->getAllNotesButtonIndex());
    }

    updateTreeViewSeparator();
}

void myTreeViewLogic::onRenameNodeRequestedFromTreeView(const QModelIndex &index, const QString &newName)
{
    //model更新
    m_treeModel->setData(index, newName, NodeItem::Roles::DisplayText);
    auto id = index.data(NodeItem::Roles::NodeId).toInt();

    //数据库更新
    emit requestRenameNodeInDB(id, newName);
}

void myTreeViewLogic::onDeleteFolderRequested(const QModelIndex &index)
{

    //提示对话框的选择的返回值
    auto btn = QMessageBox::question(nullptr, "Are you sure you want to delete this folder",
                                     "Are you sure you want to delete this folder? All notes and "
                                     "any subfolders will be deleted.");

    //确认删除
    if (btn == QMessageBox::Yes)
    {
        //索引对应的特殊节点id
        auto id = index.data(NodeItem::Roles::NodeId).toInt();

        //无法删除的文件夹，直接退出
        if (id < SpecialNodeID::DefaultNotesFolder)
        {
            qDebug() << __FUNCTION__ << "Failed while trying to delete folder with id" << id;
            return;
        }


        //获取数据库中id对应的nodedata
        NodeData node;
        QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(NodeData, node), Q_ARG(int, id));

        auto parentPath = NodePath{ node.absolutePath() }.parentPath();
        auto parentIndex = m_treeModel->folderIndexFromIdPath(parentPath);



        //父节点合法，model中删除这一行，数据库中删除
        if (parentIndex.isValid())
        {
            m_treeModel->deleteRow(index, parentIndex);
            QMetaObject::invokeMethod(m_dbManager, "moveFolderToTrash", Qt::QueuedConnection,
                                      Q_ARG(NodeData, node));
            m_treeView->setCurrentIndexC(m_treeModel->getAllNotesButtonIndex());
        }
        else
        {
            qDebug() << __FUNCTION__ << __LINE__<<"Parent index with path" << parentPath.path()
                     << "is not valid";
        }
    }

    //选择取消
    else
    {
        m_treeView->closePersistentEditor(m_treeModel->getTrashButtonIndex());
        m_treeView->update(m_treeModel->getTrashButtonIndex());
    }
}

void myTreeViewLogic::onChildNoteCountChangedFolder(int folderId, const QString &absPath, int notesCount)
{
    QModelIndex index;

    //判断传入的文件夹是不是特殊文件夹，获取路径对应的索引
    if (folderId == SpecialNodeID::RootFolder)
    {
        index = m_treeModel->getAllNotesButtonIndex();
    }
    else if (folderId == SpecialNodeID::TrashFolder)
    {
        index = m_treeModel->getTrashButtonIndex();
    }
    else
    {
        index = m_treeModel->folderIndexFromIdPath(absPath);
    }

    //如果索引合法，设置新的值
    if (index.isValid())
    {
        m_treeModel->setData(index, notesCount, NodeItem::Roles::ChildCount);
    }
}

void myTreeViewLogic::onAddFolderRequested(bool fromPlusButton)
{
    //qDebug()<<__FUNCTION__<<__LINE__<<"fromPlusButton"<<fromPlusButton;
    QModelIndex currentIndex;
    //设置当前的索引
    if (fromPlusButton)
    {
        currentIndex = m_treeView->currentIndex();
    }
    else
    {
        currentIndex = m_treeView->currentEditingIndex();
        if (!currentIndex.isValid())
        {
            currentIndex = m_treeView->currentIndex();
        }
    }

    int parentId = SpecialNodeID::RootFolder;
    NodeItem::Type currentType = NodeItem::AllNoteButton;
    QString currentAbsPath;


    //当前节点且不是从添加按钮中请求添加文件
    if (currentIndex.isValid() && !fromPlusButton)
    {
        //这个节点的类型
        auto type =
            static_cast<NodeItem::Type>(currentIndex.data(NodeItem::Roles::ItemType).toInt());

        if (type == NodeItem::FolderItem)
        {
            parentId = currentIndex.data(NodeItem::Roles::NodeId).toInt();
            // we don't allow subfolder under default notes folder
            if (parentId == SpecialNodeID::DefaultNotesFolder)
            {
                parentId = SpecialNodeID::RootFolder;
            }
        }
        else if (type == NodeItem::NoteItem)
        {
            qDebug() << "Cannot create folder under this item";
            return;
        }
        else
        {
            currentIndex = m_treeModel->rootIndex();
        }
    }
    else
    {
        currentType =
            static_cast<NodeItem::Type>(currentIndex.data(NodeItem::Roles::ItemType).toInt());
        if (currentType == NodeItem::FolderItem)
        {
            currentAbsPath = currentIndex.data(NodeItem::Roles::AbsPath).toString();
        }
        currentIndex = m_treeModel->rootIndex();
    }

    //新的节点
    int newlyCreatedNodeId;
    NodeData newFolder;

    newFolder.setNodeType(NodeData::Folder);
    QDateTime noteDate = QDateTime::currentDateTime();
    newFolder.setCreationDateTime(noteDate);
    newFolder.setLastModificationDateTime(noteDate);

    //根据父节点来设置默认填充的名字
    if (parentId != SpecialNodeID::RootFolder)
    {
        newFolder.setFullTitle(m_treeModel->getNewFolderPlaceholderName(currentIndex));
    }
    else
    {
        newFolder.setFullTitle(m_treeModel->getNewFolderPlaceholderName(m_treeModel->rootIndex()));
    }
    newFolder.setParentId(parentId); //设置父节点id(类型)

    //qDebug()<<__FUNCTION__<<__LINE__<<"parentID"<<parentId;

    //数据库中添加节点
    QMetaObject::invokeMethod(m_dbManager, "addNode", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(int, newlyCreatedNodeId), Q_ARG(NodeData, newFolder));

    //相关参数，后面设置为新节点构造函数的参数
    QHash<NodeItem::Roles, QVariant> hs;
    hs[NodeItem::Roles::ItemType] = NodeItem::Type::FolderItem;
    hs[NodeItem::Roles::DisplayText] = newFolder.fullTitle();
    hs[NodeItem::Roles::NodeId] = newlyCreatedNodeId;

    if (parentId != SpecialNodeID::RootFolder)
    {
        hs[NodeItem::Roles::AbsPath] = currentIndex.data(NodeItem::Roles::AbsPath).toString()
                                       + PATH_SEPARATOR + QString::number(newlyCreatedNodeId);
        m_treeModel->appendChildNodeToParent(currentIndex, hs);  //在这个里面new
        if (!m_treeView->isExpanded(currentIndex))
        {
            m_treeView->expand(currentIndex);
        }
    }
    else
    {
        hs[NodeItem::Roles::AbsPath] = PATH_SEPARATOR + QString::number(SpecialNodeID::RootFolder)
                                       + PATH_SEPARATOR + QString::number(newlyCreatedNodeId);
        m_treeModel->appendChildNodeToParent(m_treeModel->rootIndex(), hs);
    }


    if (fromPlusButton)
    {
        if (currentType == NodeItem::FolderItem)
        {
            currentIndex = m_treeModel->folderIndexFromIdPath(currentAbsPath);
        }
        else if (currentType == NodeItem::AllNoteButton)
        {
            currentIndex = m_treeModel->getAllNotesButtonIndex();
        }
        else if (currentType == NodeItem::TrashButton)
        {
            currentIndex = m_treeModel->getTrashButtonIndex();
        }
        else
        {
            currentIndex = m_treeModel->getAllNotesButtonIndex();
        }
        m_treeView->setIgnoreThisCurrentLoad(true);
        m_treeView->reExpandC();
        m_treeView->setCurrentIndexC(currentIndex);
        updateTreeViewSeparator();
        m_treeView->setIgnoreThisCurrentLoad(false);
    }

}

void myTreeViewLogic:: initConnect()
{
    //数据库更新，更新model
    connect(m_dbManager, &DBManager::nodesTagTreeReceived, this, &myTreeViewLogic::loadTreeModel,
            Qt::QueuedConnection);
    //顶级布局发生改变
    connect(m_treeModel, &myTreeViewModel::topLevelItemLayoutChanged, this,
            &myTreeViewLogic::updateTreeViewSeparator);
    //添加文件夹
    connect(m_treeView, &myTreeView::addFolderRequested, this,
            [this] { onAddFolderRequested(false); });
    //删除节点
    connect(m_treeView, &myTreeView::deleteNodeRequested, this,
            &myTreeViewLogic::onDeleteFolderRequested);
    //添加文件夹
    connect(m_treeDelegate, &myTreeViewDelegate::addFolderRequested, this,
            [this] { onAddFolderRequested(true); });
    //在数据库中重命名文件夹
    connect(m_treeView, &myTreeView::renameFolderInDatabase, this,
            &myTreeViewLogic::onRenameNodeRequestedFromTreeView);
    //在数据库中重命名节点
    connect(this, &myTreeViewLogic::requestRenameNodeInDB, m_dbManager, &DBManager::renameNode,
            Qt::QueuedConnection);
    //请求在数据库中移动节点
    connect(this, &myTreeViewLogic::requestMoveNodeInDB, m_dbManager, &DBManager::moveNode,
            Qt::QueuedConnection);
    //移动节点
    connect(m_treeView, &myTreeView::moveNodeRequested, this, [this](int nodeId, int targetId) {
        onMoveNodeRequested(nodeId, targetId);
        emit noteMoved(nodeId, targetId);
    });
    //请求展开文件夹
    connect(m_treeModel, &myTreeViewModel::requestExpand, m_treeView, &myTreeView::onRequestExpand);
    //请求更新绝对路径
    connect(m_treeModel, &myTreeViewModel::requestUpdateAbsPath, m_treeView,
            &myTreeView::onUpdateAbsPath);
    //请求移动节点
    connect(m_treeModel, &myTreeViewModel::requestMoveNode, this,
            &myTreeViewLogic::onMoveNodeRequested);
    //请求跟新节点相对位置
    connect(m_treeModel, &myTreeViewModel::requestUpdateNodeRelativePosition, m_dbManager,
            &DBManager::updateRelPosNode, Qt::QueuedConnection);
    //拖放文件夹成功
    connect(m_treeModel, &myTreeViewModel::dropFolderSuccessful, m_treeView,
            &myTreeView::onFolderDropSuccessful);
   //请求将文件夹移入垃圾桶
    connect(m_treeModel, &myTreeViewModel::requestMoveFolderToTrash, this,
            &myTreeViewLogic::onDeleteFolderRequested);
    //数据库中更新文件夹子节点数量
    connect(m_dbManager, &DBManager::childNotesCountUpdatedFolder, this,
            &myTreeViewLogic::onChildNoteCountChangedFolder);


    //connect(m_dbManager, &DBManager::childNotesCountUpdatedTag, this,
    //        &TreeViewLogic::onChildNotesCountChangedTag);
    //connect(m_treeDelegate, &myTreeViewDelegate::addTagRequested, this,
    //        &myTreeViewLogic::onAddTagRequested);
    //connect(m_treeView, &myTreeView::renameTagInDatabase, this,
    //        &myTreeViewLogic::onRenameTagRequestedFromTreeView);
    //connect(this, &myTreeViewLogic::requestRenameTagInDB, m_dbManager, &DBManager::renameTag,
    //        Qt::QueuedConnection);
    //connect(m_treeView, &myTreeView::changeTagColorRequested, this,
    //        &myTreeViewLogic::onChangeTagColorRequested);
    //connect(m_treeView, &myTreeView::deleteTagRequested, this,
    //        &myTreeViewLogic::onDeleteTagRequested);
    //connect(this, &myTreeViewLogic::requestChangeTagColorInDB, m_dbManager,
    //        &DBManager::changeTagColor, Qt::QueuedConnection);
    //connect(m_treeView, &myTreeView::addNoteToTag, this, &TreeViewLogic::addNoteToTag);
    //connect(m_treeModel, &myTreeViewModel::requestUpdateTagRelativePosition, m_dbManager,
    //        &DBManager::updateRelPosTag, Qt::QueuedConnection);
    //connect(m_treeModel, &NodeTreeModel::dropTagsSuccessful, m_treeView,
    //        &NodeTreeView::onTagsDropSuccessful);

    //设置主题风格
    m_style = new CustomApplicationStyle();
    qApp->setStyle(m_style);
}
