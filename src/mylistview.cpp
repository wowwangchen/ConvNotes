#include "mylistview.h"



myListView::myListView(QWidget *parent) :
    QListView(parent),
    m_animationEnabled(true),
    m_isMousePressed(false),
    m_mousePressHandled(false),
    m_rowHeight(38),
    m_dbManager(nullptr),
    m_currentFolderId{ SpecialNodeID::InvalidNodeId },
    m_isInTrash{ false },
    m_isDragging{ false },
    m_isDraggingPinnedNotes{ false },
    m_isPinnedNotesCollapsed{ false },
    m_isDraggingInsidePinned{ false }

{
    setAttribute(Qt::WA_MacShowFocusRect, false);  //macoc需要

    setupStyleSheet();

    QTimer::singleShot(0, this, SLOT(init()));

    //菜单
    setContextMenuPolicy(Qt::CustomContextMenu); //右键发送打开菜单信号
    connect(this, &QWidget::customContextMenuRequested, this, &myListView::onCustomContextMenu);
    contextMenu = new QMenu(this);

    //菜单上的按钮
    deleteNoteAction = new QAction(tr("Delete Note"), this);
    connect(deleteNoteAction, &QAction::triggered, this, [this] {
        auto indexes = selectedIndexes();
        emit deleteNoteRequested(indexes);
    });
    restoreNoteAction = new QAction(tr("Restore Note"), this);
    connect(restoreNoteAction, &QAction::triggered, this, [this] {
        auto indexes = selectedIndexes();
        emit restoreNoteRequested(indexes);
    });
    pinNoteAction = new QAction(tr("Pin Note"), this);
    connect(pinNoteAction, &QAction::triggered, this, [this] {
        auto indexes = selectedIndexes();
        emit setPinnedNoteRequested(indexes, true);
    });
    unpinNoteAction = new QAction(tr("Unpin Note"), this);
    connect(unpinNoteAction, &QAction::triggered, this, [this] {
        auto indexes = selectedIndexes();
        emit setPinnedNoteRequested(indexes, false);
    });
    newNoteAction = new QAction(tr("New Note"), this);
    connect(newNoteAction, &QAction::triggered, this, [this] {
        emit newNoteRequested(); });

    //相关需要的操作的设置
    m_dragPixmap.load("qrc:/images/notepad.icns");  //macos需要


}

myListView::~myListView()
{
    closeAllEditor();
}

void myListView::setupStyleSheet()
{
    QFile file(":/syles/notelistview.css");
    file.open(QFile::ReadOnly);
    setStyleSheet(file.readAll());
}

void myListView::closeAllEditor()
{
    for (const auto &id : m_openedEditor.keys())
    {
        //获取索引，关闭持久化
        auto index = dynamic_cast<NoteListModel *>(model())->getNoteIndex(id);
        closePersistentEditor(index);
    }
    m_openedEditor.clear();
}

void myListView::animateAddedRow(const QModelIndexList &indexes)
{
    NoteListDelegate *delegate = dynamic_cast<NoteListDelegate *>(itemDelegate());
    if (delegate)
        delegate->setState(NoteListState::Insert, indexes);
}

void myListView::setAnimationEnabled(bool isEnabled)
{
    m_animationEnabled = isEnabled;
}

void myListView::setCurrentRowActive(bool isActive)
{
    //获取当前的代理
    NoteListDelegate *delegate = dynamic_cast<NoteListDelegate *>(itemDelegate());
    if (!delegate)
        return;

    delegate->setActive(isActive);
    //刷新
    viewport()->update(visualRect(currentIndex()));
}


void myListView::setTheme(Theme::Value theme)
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

void myListView::setIsInTrash(bool newIsInTrash)
{
    m_isInTrash = newIsInTrash;
}

void myListView::setCurrentFolderId(int newCurrentFolderId)
{
    m_currentFolderId = newCurrentFolderId;
}

void myListView::openPersistentEditorC(const QModelIndex &index)
{
    if (index.isValid())
    {
        auto id = index.data(NoteListModel::NoteID).toInt();
        m_openedEditor[id] = {};
        openPersistentEditor(index);
    }
}

void myListView::closePersistentEditorC(const QModelIndex &index)
{
    if (index.isValid())
    {
        auto id = index.data(NoteListModel::NoteID).toInt();
        closePersistentEditor(index);
        m_openedEditor.remove(id);
    }
}

void myListView::setEditorWidget(int noteId, QWidget *w)
{
    //打开的编辑器中是否包含这个笔记节点
    if (m_openedEditor.contains(noteId))
    {
        m_openedEditor[noteId].push_back(w); //包含就为它设置一个窗口
    }
    else
    {
        qDebug() << __FUNCTION__ << "Error: note id" << noteId << "is not in opened editor list";
    }
}

void myListView::unsetEditorWidget(int noteId, QWidget *w)
{
    if (m_openedEditor.contains(noteId))
    {
        m_openedEditor[noteId].removeAll(w); //移出
    }
}

void myListView::setListViewInfo(const ListViewInfo &newListViewInfo)
{
    m_listViewInfo = newListViewInfo;
}

void myListView::setCurrentIndexC(const QModelIndex &index)
{

    setCurrentIndex(index);  //基类的设置
    clearSelection();       //清除之前的选中状态
    setSelectionMode(QAbstractItemView::SingleSelection); //单选
    selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);//设置为当前选中项
}

bool myListView::isDragging() const
{
    return m_isDragging;
}

bool myListView::isPinnedNotesCollapsed() const
{
    return m_isPinnedNotesCollapsed;
}

void myListView::setIsPinnedNotesCollapsed(bool newIsPinnedNotesCollapsed)
{
    m_isPinnedNotesCollapsed = newIsPinnedNotesCollapsed; //先设置变量

    //遍历，更新所有项的大小(?)
    for (int i = 0; i < model()->rowCount(); ++i)
    {
        auto index = model()->index(i, 0);
        if (index.isValid())
        {
            emit itemDelegate()->sizeHintChanged(index);
        }
    }
    update(); //刷新
    emit pinnedCollapseChanged(); //发送指定折叠改变的信号
}

QModelIndexList myListView::selectedIndex() const
{
    return selectedIndexes(); //基类成员函数
}

bool myListView::isDraggingInsidePinned() const
{
    return m_isDraggingInsidePinned;
}


void myListView::onCustomContextMenu(QPoint point)
{
    QModelIndex index = indexAt(point); //获取当前点击的位置对应的索引

    if (index.isValid())
    {
        //遍历所有选中的索引
        auto indexList = selectionModel()->selectedIndexes();
        if (!indexList.contains(index))
        {
            setCurrentIndexC(index);//设置为当前选中项
            indexList = selectionModel()->selectedIndexes(); //刷新所有选中项
        }

        //获取所有笔记的id的集合
        QSet<int> notes;
        //遍历list获取所有笔记的id
        for (const auto &idx : qAsConst(indexList))
        {
            notes.insert(idx.data(NoteListModel::NoteID).toInt());
        }


        //设置菜单显示哪些内容
        contextMenu->clear();


        //如果在垃圾桶中
        if (m_isInTrash)
        {
            if (notes.size() > 1)
            {
                restoreNoteAction->setText(tr("Restore Notes"));
            }
            else
            {
                restoreNoteAction->setText(tr("Restore Note"));
            }
            contextMenu->addAction(restoreNoteAction);
        }


        //笔记的数量大于1，添加删除按钮
        if (notes.size() > 1)
        {
            deleteNoteAction->setText(tr("Delete Notes"));
        }
        else
        {
            deleteNoteAction->setText(tr("Delete Note"));
        }
        contextMenu->addAction(deleteNoteAction);



        //不在tag中也不再垃圾桶中
        if ((!m_listViewInfo.isInTag)
            && (m_listViewInfo.parentFolderId != SpecialNodeID::TrashFolder))
        {

            contextMenu->addSeparator();

            if (notes.size() > 1)
            {
                pinNoteAction->setText(tr("Pin Notes"));
                unpinNoteAction->setText(tr("Unpin Notes"));



                //设置关于置顶、取消指定按钮，要显示哪些
                enum class ShowAction { NotInit, ShowPin, ShowBoth, ShowUnpin };
                ShowAction a = ShowAction::NotInit;
                for (const auto &idx : qAsConst(indexList))
                {
                    if (idx.data(NoteListModel::NoteIsPinned).toBool())//已经是置顶了
                    {
                        if (a == ShowAction::ShowPin)
                        {
                            a = ShowAction::ShowBoth;
                            break;
                        }
                        else
                        {
                            a = ShowAction::ShowUnpin;
                        }
                    }
                    else //还不是置顶
                    {
                        if (a == ShowAction::ShowUnpin)
                        {
                            a = ShowAction::ShowBoth;
                            break;
                        }
                        else
                        {
                            a = ShowAction::ShowPin;
                        }
                    }
                }

                //添加按钮
                switch (a)
                {
                case ShowAction::ShowPin:
                    contextMenu->addAction(pinNoteAction);
                    break;
                case ShowAction::ShowUnpin:
                    contextMenu->addAction(unpinNoteAction);
                    break;
                default:
                    contextMenu->addAction(pinNoteAction);
                    contextMenu->addAction(unpinNoteAction);
                }
            }


            //数量<=1
            else
            {
                pinNoteAction->setText(tr("Pin Note"));
                unpinNoteAction->setText(tr("Unpin Note"));
                auto isPinned = index.data(NoteListModel::NoteIsPinned).toBool();
                //只添加置顶或不置顶按钮中的一个
                if (!isPinned)//当前没有置顶
                {
                    contextMenu->addAction(pinNoteAction);
                }
                else
                {
                    contextMenu->addAction(unpinNoteAction);
                }
            }
        }

        contextMenu->addSeparator();



        //移动的操作
        if (m_dbManager)
        {
            //清楚之前的文件夹菜单的所有文件夹代表的动作
            for (auto action : qAsConst(m_folderActions))
            {
                delete action;
            }
            m_folderActions.clear();

            //又添加一个子菜单，包含移动笔记的位置的操作
            auto m = contextMenu->addMenu("Move to");


            //获取所有文件夹,id对应string
            FolderListType folders;
            QMetaObject::invokeMethod(m_dbManager, "getFolderList", Qt::BlockingQueuedConnection,
                                      Q_RETURN_ARG(FolderListType, folders));


            //遍历所有除自己外的文件夹路径，添加到子菜单的动作中
            for (const auto &id : folders.keys())
            {
                if (id == m_currentFolderId)
                {
                    continue;
                }

                //添加文件夹列表作为行为，表示移动笔记的位置
                auto action = new QAction(folders[id], this);
                //点击了某个项，就遍历所有选中的文件夹，发送移动文件夹位置的信号
                connect(action, &QAction::triggered, this, [this, id] {
                    auto indexes = selectedIndexes();
                    for (const auto &selectedIndex : qAsConst(indexes))
                    {
                        if (selectedIndex.isValid())
                        {
                            emit moveNoteRequested(
                                    selectedIndex.data(NoteListModel::NoteID).toInt(), id);
                        }
                    }
                });
                m->addAction(action);
                m_folderActions.append(action);
            }
            contextMenu->addSeparator();
        }



        //最后如果不在垃圾桶中，添加新建按钮
        if (!m_isInTrash)
        {
            contextMenu->addAction(newNoteAction);
        }

        //进入事件循环
        contextMenu->exec(viewport()->mapToGlobal(point));
    }
}

void myListView::onRemoveRowRequested(const QModelIndexList &indexes)
{
    if (!indexes.isEmpty())
    {
        //遍历所有传入的索引，添加到vector中
        for (const auto index : qAsConst(indexes))
        {
            m_needRemovedNotes.push_back(index.data(NoteListModel::NoteID).toInt());
        }

        //获取delegate，设置这些索引的状态
        NoteListDelegate *delegate = dynamic_cast<NoteListDelegate *>(itemDelegate());
        if (delegate)
        {
            if (m_animationEnabled)
            {
                delegate->setState(NoteListState::Remove, indexes);
            }
            else
            {
                delegate->setState(NoteListState::Normal, indexes);
            }
        }
    }
}


void myListView::onAnimationFinished(NoteListState state)
{
    //如果是移除状态，就获取model实施移除动作
    if (state == NoteListState::Remove)
    {
        auto model = dynamic_cast<NoteListModel *>(this->model());

        if (model)
        {
            //遍历要删除的id，获取索引然后删除
            for (const auto id : qAsConst(m_needRemovedNotes))
            {
                auto index = model->getNoteIndex(id);
                model->removeRow(index.row());
            }

            m_needRemovedNotes.clear();
        }
    }
}

void myListView::init()
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setMouseTracking(true);
    setUpdatesEnabled(true);
    viewport()->setAttribute(Qt::WA_Hover); //启动悬浮事件处理

    setupSignalsSlots(); //信号与槽,与更新视口相关
}

void myListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    //新的选择的项，之前选择的项
    QListView::selectionChanged(selected, deselected);

    QSet<int> ids;
    //将选择的项的id添加到ids中
    for (const auto &index : selectedIndexes())
    {
        ids.insert(index.data(NoteListModel::NoteID).toInt());
    }

    //发送保存选择的笔记的信号
    emit saveSelectedNote(ids);
}

void myListView::setupSignalsSlots()
{
    // remove/add separator
    // current selectected row changed

    //当前行改变了，更新前1行或者2行的显示
    connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this](const QModelIndex &current, const QModelIndex &previous) {
                if (model())
                {
                    if (current.row() < previous.row())
                    {
                        //不是第1行，获取这行的前1行的索引
                        if (current.row() > 0)
                        {
                            QModelIndex prevIndex = model()->index(current.row() - 1, 0);
                            viewport()->update(visualRect(prevIndex));
                        }
                    }

                    //不是前两行，获取这行的前2行的索引
                    if (current.row() > 1)
                    {
                        QModelIndex prevPrevIndex = model()->index(current.row() - 2, 0);
                        viewport()->update(visualRect(prevPrevIndex));
                    }
                }
            });


    //某行被点击了，更新前1或2行的显示
    // row was entered
    connect(this, &myListView::entered, this, [this](const QModelIndex &index) {

        if (model())
        {
            if (index.row() > 1)
            {
                QModelIndex prevPrevIndex = model()->index(index.row() - 2, 0);
                viewport()->update(visualRect(prevPrevIndex));

                QModelIndex prevIndex = model()->index(index.row() - 1, 0);
                viewport()->update(visualRect(prevIndex));

            }
            else if (index.row() > 0)
            {
                QModelIndex prevIndex = model()->index(index.row() - 1, 0);
                viewport()->update(visualRect(prevIndex));
            }

            //设置鼠标悬浮的索引
            NoteListDelegate *delegate = dynamic_cast<NoteListDelegate *>(itemDelegate());
            if (delegate)
                delegate->setHoveredIndex(index);
        }
    });


    //视图窗口被点击了，设置鼠标悬浮的索引
    //如果行数大于1，更新前前一行的显示
    // viewport was entered
    connect(this, &myListView::viewportEntered, this, [this]() {
        if (model() && model()->rowCount() > 1)
        {
            NoteListDelegate *delegate = dynamic_cast<NoteListDelegate *>(itemDelegate());
            if (delegate)
                delegate->setHoveredIndex(QModelIndex());

            QModelIndex lastIndex = model()->index(model()->rowCount() - 2, 0);
            viewport()->update(visualRect(lastIndex));
        }
    });

}

void myListView::mouseMoveEvent(QMouseEvent *event)
{
    //若成员变量为否，进入基类的事件
    if (!m_isMousePressed)
    {
        QListView::mouseMoveEvent(event);
        return;
    }
    //左键点击，
    if (event->buttons() & Qt::LeftButton)
    {
        //判断拖动距离是否超过一定阈值
        if ((event->pos() - m_dragStartPosition).manhattanLength() //曼哈顿距离，网格、棋盘中使用
            >= QApplication::startDragDistance())
        {
            startDrag(Qt::MoveAction);
        }
    }
    //    QListView::mouseMoveEvent(event);
}

void myListView::mousePressEvent(QMouseEvent *e)
{

}

void myListView::mouseReleaseEvent(QMouseEvent *e)
{

}

bool myListView::viewportEvent(QEvent *e)
{
    return true;
}

void myListView::dragEnterEvent(QDragEnterEvent *event)
{

}

void myListView::dragMoveEvent(QDragMoveEvent *event)
{

}

void myListView::scrollContentsBy(int dx, int dy)
{

}

void myListView::startDrag(Qt::DropActions supportedActions)
{

}

void myListView::setDbManager(DBManager *newDbManager)
{
    m_dbManager = newDbManager;
}

