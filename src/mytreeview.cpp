#include "mytreeview.h"

myTreeView::myTreeView(QWidget* parent) : QTreeView(parent)
{

    setHeaderHidden(true);
    setRootIsDecorated(false);
    setMouseTracking(true);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

}

void myTreeView::setTreeSeparator(const QVector<QModelIndex> &newSeperator, const QModelIndex &defaultNotesIndex)
{
    //关闭之前的所有索引的持久化
    for(const auto &sep : qAsConst(m_treeSeparator))
    {
        closePersistentEditor(sep);
    }

    //设置新的分隔
    m_treeSeparator=newSeperator;

    //启动编辑持久化
    for(const auto &sep : qAsConst(m_treeSeparator))
    {
        openPersistentEditor(sep);
    }

    //设置默认的笔记索引
    m_defaultNotesIndex=defaultNotesIndex;
}

void myTreeView::setIsEditing(bool newIsEditing)
{
    m_isEditing=newIsEditing;
}

const QModelIndex &myTreeView::currentEditingIndex() const
{
    return m_currentEditingIndex;
}

void myTreeView::onRenameFolderFinished(const QString &newName)
{
    //如果当前编辑的索引是可用的
    if(m_currentEditingIndex.isValid())
    {
        //获取这个索引的类型，看是不是文件夹类型
        auto itemType=static_cast<NodeItem::Type>
            ( m_currentEditingIndex.data(NodeItem::Roles::ItemType).toInt() );

        //是的，关闭编辑，在数据库中修改名称
        if(itemType==NodeItem::Type::FolderItem)
        {
            QModelIndex index=m_currentEditingIndex;
            closeCurrentEditor();
            emit renameFolderInDatabase(index,newName);
        }
        else
        {
            qDebug() << __FUNCTION__ << "wrong type";
        }
    }
}

void myTreeView::setCurrentIndexC(const QModelIndex &index)
{
    setCurrentIndex(index);                                                      //选择树形视图的索引
    clearSelection();                                                            //清除之前选中项
    setSelectionMode(QAbstractItemView::SingleSelection);                        //单选模式，最多选择一个
    selectionModel()->setCurrentIndex(index,QItemSelectionModel::SelectCurrent); //设置模型数据索引
}

void myTreeView::setTheme(Theme::Value theme)
{
    updateTheme(theme); //具体设置样式表
    m_theme = theme;    //修改成员变量值
}

Theme::Value myTreeView::theme() const
{
    return m_theme;
}

void myTreeView::updateTheme(Theme::Value theme)
{
    std::string classNames=QString::fromStdString(std::to_string(theme)).toLower().toStdString();
    if (this->styleSheet().isEmpty())
    {
        qWarning() << "setCSSClassesAndUpdate: styleSheet is empty for widget with name "
                   << this->objectName();
    }

    // set the class
    this->setProperty("class", classNames.c_str());
    // update the widget
    this->style()->polish(this);
    this->update();
}

void myTreeView::reExpandC()
{
    //获取之前展开的项的地址
    auto needExpand = std::move(m_expanded);

    //清空成员变量和树视图中的所有项(不清除model)
    m_expanded.clear();
    QTreeView::reset();

    //遍历，展开索引
    for (const auto &path : needExpand)
    {
        //获取当前这个树形结构的模型数据
        auto m_model = dynamic_cast<myTreeViewModel *>(model());
        //通过自定义model类的函数将路径转换为索引
        auto index = m_model->folderIndexFromIdPath(static_cast<NodePath>(path));

        if (index.isValid())
        {
            expand(index);
        }
    }
}

void myTreeView::reExpandC(const QStringList &expanded)
{
    m_expanded.clear();
    m_expanded = expanded.toVector();
    reExpandC();
}

void myTreeView::closeCurrentEditor()
{
    //关闭正在编辑的持久编辑器，然后给一个无效值给成员变量
    //持久编辑器是指当一个项目打开时会一直处于编辑状态知道用户手动关闭
    closePersistentEditor(m_currentEditingIndex);
    m_currentEditingIndex=QModelIndex();
}

void myTreeView::updateEditingIndex(QPoint pos)
{
    auto index = indexAt(pos);

    //如果点击的索引不等于当前编辑的节点并且菜单没打开并且当前不在编辑状态
    if (indexAt(pos) != m_currentEditingIndex && !m_isContextMenuOpened && !m_isEditing)
    {
        //获取节点类型
        auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
        if (itemType == NodeItem::Type::FolderItem || itemType == NodeItem::Type::TrashButton
            || itemType == NodeItem::Type::AllNoteButton)
        {
            closePersistentEditor(m_currentEditingIndex); //关闭当前编辑的节点的持久化编辑
            openPersistentEditor(index);                  //打开新节点的持久化编辑器
            m_currentEditingIndex = index;                //将新节点设为当前编辑节点
        }
        else
        {
            closeCurrentEditor();
        }
    }
}

void myTreeView::setIgnoreThisCurrentLoad(bool newIgnoreThisCurrentLoad)
{
    m_ignoreThisCurrentLoad = newIgnoreThisCurrentLoad;
}

bool myTreeView::isDragging() const
{
    return state()==DraggingState;
}

void myTreeView::onCustomContextMenu(QPoint point)
{
    QModelIndex index = indexAt(point);  //根据坐标获取树形结构的项的索引
    if (index.isValid())
    {
        //获取类型
        auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
        contextMenu->clear();


        //如果是文件夹类型
        if (itemType == NodeItem::Type::FolderItem)
        {
            auto id = index.data(NodeItem::Roles::NodeId).toInt(); //获取索引对应的ID

            //如果这个ID不是默认文件夹
            if (id != SpecialNodeID::DefaultNotesFolder)
            {
                //菜单打开置为true，添加以下动作
                m_isContextMenuOpened = true;
                contextMenu->addAction(renameFolderAction);
                contextMenu->addAction(deleteFolderAction);
                contextMenu->addSeparator();
                contextMenu->addAction(addSubfolderAction);
                contextMenu->exec(viewport()->mapToGlobal(point));
            }
        }
    }
}

void myTreeView::onChangeFolderColorAction()
{
    //获取当前编辑的项的类型
    auto itemType = static_cast<NodeItem::Type>(
        m_currentEditingIndex.data(NodeItem::Roles::ItemType).toInt());
    //如果是文件夹类型，就发送请求改变颜色的信号(对应的槽函数在主界面中)
    if (itemType == NodeItem::Type::FolderItem)
    {
        auto index = m_currentEditingIndex;
        emit changeFolderColorRequested(index);
    }
}

void myTreeView::onRequestExpand(const QString &folderPath)
{
    auto m_model = dynamic_cast<myTreeViewModel*>(model());
    expand(m_model->folderIndexFromIdPath(folderPath));  //从路径中获取索引，展开
}

void myTreeView::onUpdateAbsPath(const QString &oldPath, const QString &newPath)
{
    //元素转化操作，源容器的开头、结尾，目标容器的开头，转换操作自定义
    std::transform(m_expanded.begin(), m_expanded.end(), m_expanded.begin(), [&](QString s) {
        s.replace(s.indexOf(oldPath), oldPath.size(), newPath);
        return s;
    });
}

void myTreeView::onFolderDropSuccessful(const QString &path)
{
    auto m_model = dynamic_cast<myTreeViewModel*>(model());  //获取模型数据
    auto index = m_model->folderIndexFromIdPath(path);       //调用自定义模型的函数：从路径中获取索引
    if (index.isValid())
    {
        setCurrentIndexC(index);
    }
    else
    {
        setCurrentIndexC(m_model->getAllNotesButtonIndex());
    }
}

void myTreeView::reset()
{
    closeCurrentEditor();
    reExpandC();
}

void myTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTreeView::selectionChanged(selected, deselected);
    //如果设置了忽略这次操作
    if (m_ignoreThisCurrentLoad)
    {
        return;
    }


    auto indexes = selectedIndexes();  //获取所有选中的索引
    //QSet<int> tagIds;



    //遍历所有的选中的索引进行操作
    for (const auto index : qAsConst(indexes))
    {
        //获取类型
        auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
        switch (itemType)
        {
            //根节点、文件夹分隔线、笔记项，跳过
        case NodeItem::Type::RootItem:
        case NodeItem::Type::FolderSeparator:
        case NodeItem::Type::NoteItem:
        {
            return;
        }


            //以下情况，获取路径，设置上次选择文件夹路径，从这个路径中加载笔记，保存选择
            //"所有笔记"按钮
        case NodeItem::Type::AllNoteButton:
        {
            auto folderPath = index.data(NodeItem::Roles::AbsPath).toString();
            m_lastSelectFolder = folderPath;
            m_isLastSelectedFolder = true;
            emit loadNotesInFolderRequested(SpecialNodeID::RootFolder, true);
            emit saveSelected(true, NodePath::getAllNoteFolderPath(), {});
            return;
        }
        //垃圾桶
        case NodeItem::Type::TrashButton:
        {
            auto folderPath = index.data(NodeItem::Roles::AbsPath).toString();
            m_lastSelectFolder = folderPath;
            m_isLastSelectedFolder = true;
            emit loadNotesInFolderRequested(SpecialNodeID::TrashFolder, true);
            emit saveSelected(true, NodePath::getTrashFolderPath(), {});
            return;
        }
        //文件夹项
        case NodeItem::Type::FolderItem:
        {
            auto folderId = index.data(NodeItem::Roles::NodeId).toInt();
            auto folderPath = index.data(NodeItem::Roles::AbsPath).toString();
            m_lastSelectFolder = folderPath;
            m_isLastSelectedFolder = true;
            emit saveSelected(true, folderPath, {});
            emit loadNotesInFolderRequested(folderId, false);
            return;
        }

        }
    }
}

void myTreeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);
    //获取当前项的类型
    auto itemType = static_cast<NodeItem::Type>(current.data(NodeItem::Roles::ItemType).toInt());
    if (itemType == NodeItem::Type::FolderItem)
    {
        clearSelection();
        setSelectionMode(QAbstractItemView::SingleSelection);
        selectionModel()->setCurrentIndex(current, QItemSelectionModel::Current);//选择模型，管理选择状态
    }

}

void myTreeView::onDeleteNodeAction()
{
    //获取正在编辑的节点的类型和ID
    auto itemType = static_cast<NodeItem::Type>(
        m_currentEditingIndex.data(NodeItem::Roles::ItemType).toInt());
    auto id = m_currentEditingIndex.data(NodeItem::Roles::NodeId).toInt();


    //如果是文件夹项或者笔记项
    if (itemType == NodeItem::Type::FolderItem || itemType == NodeItem::Type::NoteItem)
    {
        //如果ID>默认笔记文件夹ID,也就是是新创建的节点
        if (id > SpecialNodeID::DefaultNotesFolder)
        {
            auto index = m_currentEditingIndex;
            emit deleteNodeRequested(index);  //发送删除信号
        }
    }
}

void myTreeView::onExpanded(const QModelIndex &index)
{
    //将要展开的节点加入到全部集合中
    m_expanded.push_back(index.data(NodeItem::Roles::AbsPath).toString());
    //发送保存信号
    emit saveExpand(QStringList::fromVector(m_expanded));
}

void myTreeView::onCollapsed(const QModelIndex &index)
{
    //在保存的展开的项的集合中移除包含目标项的项
    m_expanded.removeAll(index.data(NodeItem::Roles::AbsPath).toString());
    //发送保存信号
    emit saveExpand(QStringList::fromVector(m_expanded));
}

void myTreeView::dragEnterEvent(QDragEnterEvent *event)
{
    //判断拖入事件是否包含特定格式
    if(event->mimeData()->hasFormat(NOTE_MIME))
    {
        //拖入事件被接收
        event->acceptProposedAction();
    }
    else
    {
        QTreeView::dragEnterEvent(event);
    }
}

void myTreeView::dropEvent(QDropEvent *event)
{
    //判断拖入事件是否包含特定格式，也就是看拖动的是不是节点
    if (event->mimeData()->hasFormat(NOTE_MIME))
    {
        //获取目标项的索引
        auto dropIndex = indexAt(event->pos());

        if (dropIndex.isValid())//判断索引是否合法
        {
            //获取目标项的类型
            auto itemType =
                static_cast<NodeItem::Type>(dropIndex.data(NodeItem::Roles::ItemType).toInt());
            //获取源项的数据,根据 '/' 拆分为一个字符串列表
            auto id_list = QString::fromUtf8(event->mimeData()->data(NOTE_MIME))
                               .split(QStringLiteral(PATH_SEPARATOR)); //宏


            bool ok = false;
            //源项的各个字符串(也就是各个层次地址)遍历
            for (const auto &s : qAsConst(id_list))
            {
                auto nodeId = s.toInt(&ok);  //将字符串转成整型
                if(ok)
                {
                    //文件夹类型，移动文件(夹)位置
                    if (itemType == NodeItem::Type::FolderItem)
                    {
                        emit requestMoveNode(nodeId,dropIndex.data(NodeItem::NodeId).toInt());//源、目标
                        event->acceptProposedAction();
                    }
                    //垃圾桶类型，放入垃圾桶文件夹
                    else if (itemType == NodeItem::Type::TrashButton)
                    {
                        emit requestMoveNode(nodeId,SpecialNodeID::TrashFolder);
                        event->acceptProposedAction();
                    }
                }
            }
        }
    }
    else
    {
        QTreeView::dropEvent(event);
    }
}

void myTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    //笔记类型
    if (event->mimeData()->hasFormat(NOTE_MIME))
    {
        auto index = indexAt(event->pos());     //获取源项的索引
        auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());//获取类型
        if (itemType != NodeItem::Type::AllNoteButton) //如果不是"所有笔记按钮"项
        {
            //updateEditingIndex(event->pos());
            event->acceptProposedAction();
        }
    }

    //不是笔记类型
    else
    {   //文件夹类型
        if (event->mimeData()->hasFormat(FOLDER_MIME))
        {
            auto trashRect =   //获取垃圾桶项的坐标矩阵
                visualRect(dynamic_cast<myTreeViewModel *>(model())->getTrashButtonIndex());

            //拖动到垃圾桶范围内
            if (event->pos().y() > (trashRect.y() + 5)&& event->pos().y() < (trashRect.bottom() - 5))
            {
                setDropIndicatorShown(true); //显示拖放指示器
                QTreeView::dragMoveEvent(event);
                return;
            }
            //在整个树形结构之外，忽略
            if (event->pos().y() > visualRect(m_treeSeparator[1]).y())
            {
                event->ignore();
                return;
            }
            //默认节点之下25的位置以内,忽略
            if (event->pos().y() < visualRect(m_defaultNotesIndex).y() + 25)
            {
                event->ignore();
                return;
            }
        }

        setDropIndicatorShown(true);  //是否显示拖放指示器
        QTreeView::dragMoveEvent(event);
    }
}

void myTreeView::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(myTreeView);   //声明一个指向类的私有成员的指针
    QPoint topLeft;
    QPoint bottomRight = event->pos();

    //树形结构是展开或者收缩状态
    if (state() == ExpandingState || state() == CollapsingState)
    {
        return;
    }

    updateEditingIndex(event->pos());
    //如果是拖动状态
    if (state() == DraggingState)
    {
        topLeft = d->pressedPosition - d->offset();
        //鼠标拖动的距离大于系统定义的距离，开始拖动
        if ((topLeft - bottomRight).manhattanLength() > QApplication::startDragDistance())
        {
            d->pressedIndex = QModelIndex();
            startDrag(d->model->supportedDragActions());
            setState(NoState); // the startDrag will return when the dnd operation is done
            stopAutoScroll();
        }
        return;
    }

    //节点有效、不是拖动状态、有按钮按下，设置为拖动状态
    if (d->pressedIndex.isValid() && (state() != DragSelectingState)
        && (event->buttons() != Qt::NoButton))
    {
        setState(DraggingState);
        return;
    }

}

void myTreeView::mousePressEvent(QMouseEvent *event)
{
    Q_D(myTreeView);
    d->delayedAutoScroll.stop();

    auto index = indexAt(event->pos());
    if (index.isValid())
    {
        auto itemType =     //获取点击的索引对应的节点的类型
            static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());

        switch (itemType)
        {
        case NodeItem::Type::FolderItem:
        {
            //鼠标点击如果是文件夹类型，如果点击了图标处，则更换展开折叠状态
            auto rect = visualRect(index);
            auto iconRect = QRect(rect.x() + 5, rect.y() + (rect.height() - 20) / 2, 24, 24);
            if (iconRect.contains(event->pos()))
            {
                if (isExpanded(index))
                {
                    collapse(index);
                }
                else
                {
                    expand(index);
                }
                return;
            }
            setCurrentIndexC(index);
            break;
        }
        //其余有效情况就设置一下当前的索引
        case NodeItem::Type::AllNoteButton:
        case NodeItem::Type::TrashButton:
        {
            setCurrentIndexC(index);
            break;
        }
        default:
        {
            break;
        }
        }
    }
    //点击的节点持久化编辑
    QPoint pos = event->pos();
    QPersistentModelIndex _index = indexAt(pos);
    d->pressedAlreadySelected = d->selectionModel->isSelected(_index);
    d->pressedIndex = _index;
    d->pressedModifiers = event->modifiers();
    QPoint offset = d->offset();
    d->pressedPosition = pos + offset;
    updateEditingIndex(event->pos());
}

void myTreeView::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    Q_D(myTreeView);  //指向私有成员的指针

    if (m_needReleaseIndex.isValid())  //需要释放的索引有效
    {

        selectionModel()->select(m_needReleaseIndex, QItemSelectionModel::Deselect);
        if (selectionModel()->selectedIndexes().isEmpty())
        {
            //如果上次选择的是文件夹，获取索引，然后请求打开
            if (!m_isLastSelectedFolder)
            {
                auto index = dynamic_cast<myTreeViewModel*>(model())
                                 ->folderIndexFromIdPath(m_lastSelectFolder);
                if (index.isValid())
                {
                    emit requestLoadLastSelectedNote();
                    setCurrentIndexC(index);
                }
                else
                {   //索引不合法，就把当前的索引设置"所有笔记节点"
                    setCurrentIndexC(dynamic_cast<myTreeViewModel*>(model())->getAllNotesButtonIndex());
                }
            }
            else
            {
                setCurrentIndexC(dynamic_cast<myTreeViewModel*>(model())->getAllNotesButtonIndex());
            }
        }
    }
    //需要释放的索引置为空
    m_needReleaseIndex = QModelIndex();
    setState(NoState);
    d->pressedIndex = QPersistentModelIndex();
}

void myTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    Q_D(myTreeView);
    d->pressedIndex = QModelIndex();          //点击的索引置为空
    m_needReleaseIndex = QModelIndex();       //需要释放的索引置为空，也就是不用取消选择了
    //QTreeView::mouseDoubleClickEvent(event);
}

void myTreeView::leaveEvent(QEvent *event)
{
    //如果菜单没打开并且当前不在编辑状态
    if (!m_isContextMenuOpened && !m_isEditing)
    {
        closeCurrentEditor();           //关闭当前持久化编辑器
        QTreeView::leaveEvent(event);
    }
}






