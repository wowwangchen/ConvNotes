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
    setAttribute(Qt::WA_MacShowFocusRect, false);  //macos需要

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
        m_openedEditor[id] = {};        //列表中添加
        openPersistentEditor(index);    //真正打开持久化编辑器
    }
}

void myListView::closePersistentEditorC(const QModelIndex &index)
{
    if (index.isValid())
    {
        auto id = index.data(NoteListModel::NoteID).toInt();
        closePersistentEditor(index);
        m_openedEditor.remove(id);  //列表成员中移除
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
    Q_D(myListView);  //指向私有成员的指针
    m_isMousePressed = true;

    //判断点击的位置的索引是否合法
    auto index = indexAt(e->pos());
    if (!index.isValid())
    {
        emit noteListViewClicked();
        return;
    }

    //判断索引是否为第一个置顶的索引并且点击位置在矩形内，是的话则展开折叠取反
    auto model = dynamic_cast<NoteListModel *>(this->model());
    if (model && model->isFirstPinnedNote(index))
    {
        auto rect = visualRect(index);
        auto iconRect = QRect(rect.right() - 25, rect.y() + 2, 20, 20);
        if (iconRect.contains(e->pos()))
        {
            setIsPinnedNotesCollapsed(!isPinnedNotesCollapsed());
            m_mousePressHandled = true;
            return;
        }
    }

    //如果是左键
    if (e->button() == Qt::LeftButton)
    {
        m_dragStartPosition = e->pos();
        auto oldIndexes = selectionModel()->selectedIndexes();
        //如果当前选中的索引没有包含点击的索引
        if (!oldIndexes.contains(index))
        {
            //判断是否按下ctrl，是则进入多选模式，将当前索引添加到选中的索引列表中
            if (e->modifiers() == Qt::ControlModifier)
            {
                setSelectionMode(QAbstractItemView::MultiSelection);
                setCurrentIndex(index);
                selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
                auto selectedIndexes = selectionModel()->selectedIndexes();
                emit notePressed(selectedIndexes);
            }
            else
            {
                setCurrentIndexC(index);
                emit notePressed({ index });
            }
            m_mousePressHandled = true;
        }
    }

    //右键点击
    else if (e->button() == Qt::RightButton)
    {
        auto oldIndexes = selectionModel()->selectedIndexes();
        //之前选中的索引不包含当前点击的索引
        if (!oldIndexes.contains(index))
        {
            setCurrentIndexC(index);
            emit notePressed({ index });
        }
    }

    QPoint offset = d->offset();
    d->pressedPosition = e->pos() + offset;
}

void myListView::mouseReleaseEvent(QMouseEvent *e)
{
    m_isMousePressed = false;
    auto index = indexAt(e->pos());
    if (!index.isValid())
    {
        return;
    }

    //左键点击并且之前的鼠标点击事件未处理
    if (e->button() == Qt::LeftButton && !m_mousePressHandled)
    {
        //点击了ctrl,多选模式
        if (e->modifiers() == Qt::ControlModifier)
        {
            setSelectionMode(QAbstractItemView::MultiSelection);
            //如果点击的索引在之前选中的索引中，就取消选中
            auto oldIndexes = selectionModel()->selectedIndexes();
            if (oldIndexes.contains(index) && oldIndexes.size() > 1)
            {
                selectionModel()->select(index, QItemSelectionModel::Deselect);
            }
            //不包含就添加选中
            else
            {
                setCurrentIndex(index);
                selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
            }
            auto selectedIndexes = selectionModel()->selectedIndexes();
            emit notePressed(selectedIndexes);
        }

        else
        {
            setCurrentIndexC(index);
            emit notePressed({ index });
        }
    }


    m_mousePressHandled = false;
    QListView::mouseReleaseEvent(e);
}

bool myListView::viewportEvent(QEvent *e)
{
    if (model())
    {
        switch (e->type())
        {
            //鼠标离开事件
        case QEvent::Leave:
        {
            QPoint pt = mapFromGlobal(QCursor::pos());
            QModelIndex index = indexAt(QPoint(10, pt.y()));
            //如果行数>0，清除视图的悬停状态，更新上一行的视图
            if (index.row() > 0)
            {
                index = model()->index(index.row() - 1, 0);
                NoteListDelegate *delegate = dynamic_cast<NoteListDelegate *>(itemDelegate());
                if (delegate)
                {
                    delegate->setHoveredIndex(QModelIndex());
                    viewport()->update(visualRect(index));
                }
            }
            break;
        }
        default:
            break;
        }

    }

    return QListView::viewportEvent(e);
}

void myListView::dragEnterEvent(QDragEnterEvent *event)
{
    //是否为笔记属性(宏定义)
    if (event->mimeData()->hasFormat(NOTE_MIME))
    {
        event->acceptProposedAction();
    }
    else
    {
        QListView::dragEnterEvent(event);
    }
}

void myListView::dragMoveEvent(QDragMoveEvent *event)
{
    //是笔记类型
    if (event->mimeData()->hasFormat(NOTE_MIME))
    {
        auto index = indexAt(event->pos());
        auto isPinned = index.data(NoteListModel::NoteIsPinned).toBool();
        //不合法
        if (!index.isValid())
        {
            event->ignore();
            return;
        }
        //不是拖动置顶的笔记，并且当前笔记没有置顶
        if (!m_isDraggingPinnedNotes && !isPinned)
        {
            event->ignore();
            return;
        }

        //都满足，设置当前拖动的位置在置顶区内
        m_isDraggingInsidePinned = isPinned;
        event->acceptProposedAction();          //接受事件
        setDropIndicatorShown(true);            //显示拖拽指示器
        QListView::dragMoveEvent(event);
        return;
    }

    else
    {
        event->ignore();
    }
}

void myListView::scrollContentsBy(int dx, int dy)
{
    QListView::scrollContentsBy(dx, dy);
    auto m_listModel = dynamic_cast<NoteListModel *>(model());
    if (!m_listModel)
    {
        return;
    }

    //遍历model中所有行
    for (int i = 0; i < m_listModel->rowCount(); ++i)
    {
        auto index = m_listModel->index(i, 0);
        if (index.isValid())
        {
            //根据视口的高度，选择关闭某些超出范围的编辑器
            auto id = index.data(NoteListModel::NoteID).toInt();

            //当前打开的编辑器包含此节点
            if (m_openedEditor.contains(id))
            {
                auto y = visualRect(index).y();
                auto range = abs(viewport()->height());
                if ((y < -range) || (y > 2 * range))
                {
                    m_openedEditor.remove(id);
                    closePersistentEditor(index);
                }
            }
            //如果之前节点的编辑器未打开，但现在在范围内，打开持久化编辑器
            else
            {
                auto y = visualRect(index).y();
                auto range = abs(viewport()->height());
                if (y < -range)
                {
                    continue;
                }
                else if (y > 2 * range) //证明后面的都超了，直接退出
                {
                    break;
                }
                openPersistentEditorC(index);
            }

        }
    }
}

void myListView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);
    Q_D(myListView);

    //获取选择的所有索引
    auto indexes = selectedIndexes();
    QMimeData *mimeData = d->model->mimeData(indexes);
    if (!mimeData)
    {
        return;
    }
    QRect rect;
    QPixmap pixmap;

    //若只选中了一个
    if (indexes.size() == 1)
    {
        auto current = indexes[0];
        auto id = current.data(NoteListModel::NoteID).toInt();

        //看持久化编辑器是否包含这个节点
        if (m_openedEditor.contains(id))
        {
            //获取一个截图作为拖动图标
            QItemViewPaintPairs paintPairs = d->draggablePaintPairs(indexes, &rect);
            Q_UNUSED(paintPairs);

            //获取对应的值(widget)
            auto wl = m_openedEditor[id];
            if (!wl.empty())
            {
                pixmap = wl.first()->grab(); //对widget截图
            }
            else
            {
                qDebug() << __FUNCTION__ << "Dragging row" << current.row()
                         << "is in opened editor list but editor widget is null";
            }
        }
        else
        {
            pixmap = d->renderToPixmap(indexes, &rect);
        }


        //数据模型中又置顶，且第一个置顶与不指定的笔记都是当前节点
        auto model = dynamic_cast<NoteListModel *>(this->model());
        if (model && model->hasPinnedNote()
            && (model->isFirstPinnedNote(current) || model->isFirstUnpinnedNote(current)))
        {
            QRect r(0, 25, rect.width(), rect.height() - 25);
            pixmap = pixmap.copy(r);
            rect.setHeight(rect.height() - 25);
        }
        rect.adjust(horizontalOffset(), verticalOffset(), 0, 0);
    }



    //选中了多个，用一个图标代替
    else
    {
        pixmap.load(":/image/notepads.png");
        pixmap = pixmap.scaled(pixmap.width() / 4, pixmap.height() / 4, Qt::KeepAspectRatio,
                               Qt::SmoothTransformation);


#ifdef __APPLE__
        QFont m_displayFont(QFont(QStringLiteral("SF Pro Text")).exactMatch()
                                ? QStringLiteral("SF Pro Text")
                                : QStringLiteral("Roboto"));
#elif _WIN32
        QFont m_displayFont(QFont(QStringLiteral("Segoe UI")).exactMatch()
                                ? QStringLiteral("Segoe UI")
                                : QStringLiteral("Roboto"));
#else
        QFont m_displayFont(QStringLiteral("Roboto"));
#endif


        //文本字体
        m_displayFont.setPixelSize(16);
        QFontMetrics fmContent(m_displayFont);
        QString sz = QString::number(indexes.size());
        QRect szRect = fmContent.boundingRect(sz);
        QPixmap px(pixmap.width() + szRect.width(), pixmap.height());
        px.fill(Qt::transparent);

        //显示矩形范围
        QRect nameRect(px.rect());
        QPainter painter(&px);
        painter.setPen(Qt::red);
        painter.drawPixmap(0, 0, pixmap);
        painter.setFont(m_displayFont);
        painter.drawText(nameRect, Qt::AlignRight | Qt::AlignBottom, sz);//放到右下角
        painter.end();
        std::swap(pixmap, px);
        rect = px.rect();
    }


    //遍历所有选中的索引,查找是否拖动置顶的笔记
    m_isDraggingPinnedNotes = false;
    for (const auto &index : qAsConst(indexes))
    {
        //如果其中有置顶的，变量置为true，退出
        if (index.data(NoteListModel::NoteIsPinned).toBool())
        {
            m_isDraggingPinnedNotes = true;
            break;
        }
    }


    QDrag *drag = new QDrag(this);  //创造操作发送拖放数据
    drag->setPixmap(pixmap);
    drag->setMimeData(mimeData);
    //设置浏览的图像的热点位置
    if (indexes.size() == 1)
    {
        drag->setHotSpot(d->pressedPosition - rect.topLeft());
    }
    else
    {
        drag->setHotSpot({ 0, 0 });
    }

    //获取所有编辑器的widget
    auto openedEditors = m_openedEditor.keys();
    m_isDragging = true;
    Qt::DropAction dropAction = drag->exec(Qt::MoveAction); //进入拖动动作事件循环
    //用户取消了拖动，释放资源
    if (dropAction == Qt::IgnoreAction)
    {
        drag->deleteLater();
        mimeData->deleteLater();
    }


#if QT_VERSION > QT_VERSION_CHECK(5, 15, 0)
    d->dropEventMoved = false;
#endif


    //善后
    m_isDragging = false;
    // Reset the drop indicator
    d->dropIndicatorRect = QRect();
    d->dropIndicatorPosition = OnItem;
    closeAllEditor();
    for (const auto &id : qAsConst(openedEditors))
    {
        auto index = dynamic_cast<NoteListModel *>(model())->getNoteIndex(id);
        openPersistentEditorC(index);
    }
    scrollContentsBy(0, 0); //刷新界面


}

void myListView::setDbManager(DBManager *newDbManager)
{
    m_dbManager = newDbManager;
}

