#include "mylistviewlogic.h"

//当前笔记集合是否合法，此函数与类无关
static bool isInvalidCurrentNotesId(const QSet<int> &currentNotesId)
{
    if (currentNotesId.isEmpty())
    {
        return true;
    }

    bool isInvalid = true;
    for (const auto &id : qAsConst(currentNotesId))
    {
        if (id != SpecialNodeID::InvalidNodeId)
        {
            isInvalid = false;
        }
    }

    return isInvalid;
}


myListViewLogic::myListViewLogic(myListView *noteView, NoteListModel *noteModel,
                                 QLineEdit *searchEdit, QToolButton *clearButton, DBManager *dbManager, QObject *parent)
    : QObject(parent),
    m_listView{ noteView },
    m_listModel{ noteModel },
    m_searchEdit{ searchEdit },
    m_clearButton{ clearButton },
    m_dbManager{ dbManager },
    m_needLoadSavedState{ 0 },
    m_lastSelectedNotes{}

{
    m_listDelegate = new NoteListDelegate(m_listView, m_listView);
    m_listView->setItemDelegate(m_listDelegate);
    m_listView->setDbManager(m_dbManager);


    //加载笔记列表
    connect(m_dbManager, &DBManager::notesListReceived, this, &myListViewLogic::loadNoteListModel);
    //移动某些行
    connect(m_listModel, &NoteListModel::rowsAboutToBeMovedC, m_listView,
            &myListView::rowsAboutToBeMoved);
    connect(m_listModel, &NoteListModel::rowsMovedC, m_listView, &myListView::rowsMoved);
    //点击某些笔记
    connect(m_listView, &myListView::notePressed, this,
            [this](const QModelIndexList &indexes) { onNotePressed(indexes); });


    //请求在数据库中操作
    connect(this, &myListViewLogic::requestRemoveNoteDb, dbManager, &DBManager::removeNote,
            Qt::QueuedConnection);
    connect(this, &myListViewLogic::requestMoveNoteDb, dbManager, &DBManager::moveNode,
            Qt::QueuedConnection);
    connect(this, &myListViewLogic::requestSearchInDb, dbManager, &DBManager::searchForNotes,
            Qt::QueuedConnection);
    connect(this, &myListViewLogic::requestClearSearchDb, dbManager, &DBManager::clearSearch,
            Qt::QueuedConnection);
    connect(m_listModel, &NoteListModel::requestUpdatePinnedRelPos, dbManager,
            &DBManager::updateRelPosPinnedNote, Qt::QueuedConnection);
    connect(m_listModel, &NoteListModel::requestUpdatePinnedRelPosAN, dbManager,
            &DBManager::updateRelPosPinnedNoteAN, Qt::QueuedConnection);
    connect(m_listModel, &NoteListModel::requestUpdatePinned, dbManager,
            &DBManager::setNoteIsPinned, Qt::QueuedConnection);

    //删除与恢复
    connect(m_listView, &myListView::deleteNoteRequested, this,
            &myListViewLogic::deleteNoteRequestedI);
    connect(m_listView, &myListView::restoreNoteRequested, this,
            &myListViewLogic::restoreNotesRequestedI);


    connect(m_listModel, &QAbstractItemModel::rowsInserted, this,
            &myListViewLogic::updateListViewLabel);
    connect(m_listModel, &QAbstractItemModel::rowsRemoved, this,
            &myListViewLogic::updateListViewLabel);
    connect(m_listView, &myListView::newNoteRequested, this, &myListViewLogic::requestNewNote);
    connect(m_listView, &myListView::moveNoteRequested, this, &myListViewLogic::moveNoteRequested);
    connect(m_listModel, &NoteListModel::rowCountChanged, this, &myListViewLogic::onRowCountChanged);
    connect(m_listView, &myListView::doubleClicked, this, &myListViewLogic::onNoteDoubleClicked);
    connect(m_listView, &myListView::setPinnedNoteRequested, this,
            &myListViewLogic::onSetPinnedNoteRequested);
    connect(m_listView, &myListView::pinnedCollapseChanged, this,
            &myListViewLogic::onRowCountChanged);
    connect(m_listModel, &NoteListModel::requestOpenNoteEditor, this,
            [this](const QModelIndexList &indexes) {
                for (const auto &index : indexes)
                {
                    if (index.isValid())
                    {
                        m_listView->openPersistentEditorC(index);
                    }
                }
            });
    connect(m_listModel, &NoteListModel::requestCloseNoteEditor, this,
            [this](const QModelIndexList &indexes) {
                for (const auto &index : indexes)
                {
                    if (index.isValid())
                    {
                        m_listView->closePersistentEditorC(index);
                    }
                }
            });
    connect(m_listDelegate, &NoteListDelegate::animationFinished, m_listView,
            &myListView::onAnimationFinished);
    connect(m_listModel, &NoteListModel::requestRemoveNotes, m_listView,
            &myListView::onRemoveRowRequested);
    connect(this, &myListViewLogic::requestNotesListInFolder, m_dbManager,
            &DBManager::onNotesListInFolderRequested, Qt::QueuedConnection);
    connect(m_listModel, &NoteListModel::rowsInsertedC, m_listView, &myListView::onRowsInserted);
    connect(m_listModel, &NoteListModel::selectNotes, this, &myListViewLogic::selectNotes);
    connect(m_listView, &myListView::noteListViewClicked, this,
            &myListViewLogic::onListViewClicked);
}

void myListViewLogic::selectNote(const QModelIndex &noteIndex)
{
    if (noteIndex.isValid())
    {
        //获取笔记节点，设置选中状态，listview滚动到这个地方
        const auto &note = m_listModel->getNote(noteIndex);
        m_listView->selectionModel()->select(noteIndex, QItemSelectionModel::ClearAndSelect);
        m_listView->setCurrentIndexC(noteIndex);
        m_listView->scrollTo(noteIndex);
        emit showNotesInEditor({ note });
    }
    else
    {
        qDebug() << __FUNCTION__ << "noteIndex is not valid";
    }
}

const ListViewInfo &myListViewLogic::listViewInfo() const
{
    return m_listViewInfo;
}

void myListViewLogic::selectFirstNote()
{
    if (m_listModel->rowCount() > 0)
    {
        QModelIndex index = m_listModel->index(0, 0);
        if (index.isValid())
        {
            //选中第一个笔记
            m_listView->setCurrentIndexC(index);
            const auto &firstNote = m_listModel->getNote(index);
            emit showNotesInEditor({ firstNote });
        }
    }
    else
    {
        emit closeNoteEditor();
    }

}

void myListViewLogic::setTheme(Theme::Value theme)
{
    m_listView->setTheme(theme);        //列表
    m_listDelegate->setTheme(theme);    //代理
    m_listView->update();               //刷新界面
}

bool myListViewLogic::isAnimationRunning()
{
    return m_listDelegate->animationState() == QTimeLine::Running;
}

void myListViewLogic::setLastSavedState(const QSet<int> &lastSelectedNotes, int needLoadSavedState)
{
    m_needLoadSavedState = needLoadSavedState;
    m_lastSelectedNotes = lastSelectedNotes;
}

void myListViewLogic::requestLoadSavedState(int needLoadSavedState)
{
    m_needLoadSavedState = needLoadSavedState;
}

void myListViewLogic::selectAllNotes()
{
    if (m_listModel->rowCount() > 50)
    {

#ifdef Q_OS_MAC
        auto btn = QMessageBox::question(nullptr,
                                         "Are you sure you want to select more than 50 notes?",
                                         "Selecting more than 50 notes to show in the editor might "
                                         "cause the app to hang.  Do you want to continue?");
#else
        auto btn = QMessageBox::question(nullptr,
                                         "Are you sure you want to select more than 50 notes?",
                                         "Selecting more than 50 notes to show in the editor might "
                                         "cause the app to hang. Do you want to continue?");
#endif


        if (btn != QMessageBox::Yes)
        {
            return;
        }
    }

    //选择全部
    m_listView->clearSelection();
    m_listView->setSelectionMode(QAbstractItemView::MultiSelection);
    m_listView->selectAll();

    //    QModelIndexList indexes;
    //    for (int i = 0; i < m_listModel->rowCount(); ++i) {
    //        auto index = m_listModel->index(i, 0);
    //        if (index.isValid()) {
    //            indexes.append(index);
    //            m_listView->setCurrentIndex(index);
    //            m_listView->selectionModel()->setCurrentIndex(index,
    //            QItemSelectionModel::SelectCurrent);
    //        }
    //    }

    onNotePressed(m_listView->selectedIndex());
}

void myListViewLogic::loadNoteListModel(const QVector<NodeData> &noteList, const ListViewInfo &inf)
{
    auto currentNotesId = m_listViewInfo.currentNotesId;
    m_listViewInfo = inf;

    //根据父节点是否是根节点来判断是否在allnotes中
    if (m_listViewInfo.parentFolderId == SpecialNodeID::RootFolder)
    {
        m_listDelegate->setIsInAllNotes(true);
    }
    else
    {
        m_listDelegate->setIsInAllNotes(false);
    }

    //设置列表信息
    m_listModel->setListNote(noteList, m_listViewInfo);
    m_listView->setListViewInfo(m_listViewInfo);
    updateListViewLabel();


    //判断父节点是否是垃圾桶
    if ( m_listViewInfo.parentFolderId == SpecialNodeID::TrashFolder)
    {
        emit setNewNoteButtonVisible(false);  //在垃圾桶中就不用显示了
        m_listView->setIsInTrash(true);
    }
    else
    {
        emit setNewNoteButtonVisible(true);
        m_listView->setIsInTrash(false);
    }


    m_listView->setCurrentFolderId(m_listViewInfo.parentFolderId);


    //是否需要新建笔记
    if (m_listViewInfo.needCreateNewNote)
    {
        m_listViewInfo.needCreateNewNote = false;
        QTimer::singleShot(50, this, &myListViewLogic::requestNewNote); //50ms后发送信号
    }

    //笔记集合全部合法并且当前当前没有在搜索笔记
    if (!m_listViewInfo.isInSearch && !isInvalidCurrentNotesId(currentNotesId))
    {
        //设置当前位置为一个合法的节点
        if (m_listViewInfo.scrollToId != SpecialNodeID::InvalidNodeId)
        {
            currentNotesId = { m_listViewInfo.scrollToId };
            m_listViewInfo.scrollToId = SpecialNodeID::InvalidNodeId;
        }
        //若笔记集合不为空，则将所有合法的节点都添加
        if (!currentNotesId.isEmpty())
        {
            QModelIndexList indexes;
            for (const auto &id : qAsConst(currentNotesId))
            {
                if (id != SpecialNodeID::InvalidNodeId)
                {
                    indexes.append(m_listModel->getNoteIndex(id));
                }
            }
            if (!indexes.isEmpty())
            {
                selectNotes(indexes);
                return;
            }
        }
    }

    //需要加载保存的状态
    if (m_needLoadSavedState > 0)
    {
        m_needLoadSavedState -= 1;
        //获取上次所有选择的节点
        if (!m_lastSelectedNotes.isEmpty())
        {
            QModelIndexList indexes;
            for (const auto &id : qAsConst(m_lastSelectedNotes))
            {
                if (id != SpecialNodeID::InvalidNodeId)
                {
                    indexes.append(m_listModel->getNoteIndex(id));
                }
            }
            if (!indexes.isEmpty())
            {
                selectNotes(indexes);
                return;
            }
        }
    }


    selectFirstNote();
}

void myListViewLogic::onNotePressed(const QModelIndexList &indexes)
{
    QVector<NodeData> notes;
    QModelIndex lastIndex;

    //获取传入的索引对应的所有笔记
    for (const auto &index : indexes)
    {
        if (index.isValid())
        {
            const auto &note = m_listModel->getNote(index);
            notes.append(note);
            lastIndex = index;
        }
    }

    //滚动到倒数第二个笔记
    m_listView->scrollTo(lastIndex);

    emit showNotesInEditor(notes);//显示这些笔记
    m_listView->setCurrentRowActive(false);
}

void myListViewLogic::deleteNoteRequestedI(const QModelIndexList &indexes)
{
    if (!indexes.empty())
    {
        bool isInTrash = false;
        QVector<NodeData> needDelete;   //节点
        QModelIndexList needDeleteI;    //索引

        //遍历所有传入的节点
        for (const auto &index : qAsConst(indexes))
        {
            if (index.isValid())
            {
                //获取id对应的节点
                auto id = index.data(NoteListModel::NoteID).toInt();
                NodeData note;
                QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                                          Q_RETURN_ARG(NodeData, note), Q_ARG(int, id));
                if (note.parentId() == SpecialNodeID::TrashFolder)
                {
                    isInTrash = true;
                }
                needDeleteI.append(index);
                needDelete.append(note);
            }
        }
        if (isInTrash)
        {
            //永久删除，移除model中，关闭持久化编辑器，数据库中移除
            auto btn = QMessageBox::question(
                nullptr, "Are you sure you want to delete this note permanently",
                "Are you sure you want to delete this note permanently? It will not be "
                "recoverable.");
            if (btn == QMessageBox::Yes)
            {
                selectNoteDown();
                bool needClose = false;
                if (m_listModel->rowCount() == needDeleteI.size())
                {
                    needClose = true;
                }
                m_listModel->removeNotes(needDeleteI);
                if (needClose)
                {
                    emit closeNoteEditor();
                }
                for (const auto &note : qAsConst(needDelete))
                {
                    emit requestRemoveNoteDb(note);
                }
            }
        }
        //不在垃圾桶中，移动节点到垃圾桶即可
        else
        {
            selectNoteDown();
            bool needClose = false;
            if (m_listModel->rowCount() == needDeleteI.size())
            {
                needClose = true;
            }
            m_listModel->removeNotes(needDeleteI);
            if (needClose)
            {
                emit closeNoteEditor();
            }
            for (const auto &note : qAsConst(needDelete))
            {
                emit requestRemoveNoteDb(note);
            }
        }
    }
}

void myListViewLogic::restoreNotesRequestedI(const QModelIndexList &indexes)
{
    QModelIndexList needRestoredI;
    QSet<int> needRestored;


    //遍历所有索引，父节点是垃圾桶就添加到要恢复的列表中
    for (const auto &index : qAsConst(indexes))
    {
        if (index.isValid())
        {
            auto id = index.data(NoteListModel::NoteID).toInt();
            NodeData note;
            QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                                      Q_RETURN_ARG(NodeData, note), Q_ARG(int, id));
            if (note.parentId() == SpecialNodeID::TrashFolder)
            {
                needRestoredI.append(index);
                needRestored.insert(note.id());
            }
            else
            {
                qDebug() << "Note id" << id << "is currently not in Trash";
            }
        }
    }

    //从列表中移除，添加到默认笔记文件夹中
    bool needClose = false;
    if (m_listModel->rowCount() == needRestoredI.size())
    {
        needClose = true;
    }
    m_listModel->removeNotes(needRestoredI);
    if (needClose)
    {
        emit closeNoteEditor();
    }


    NodeData defaultNotesFolder;
    QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(NodeData, defaultNotesFolder),
                              Q_ARG(int, SpecialNodeID::DefaultNotesFolder));
    for (const auto &id : qAsConst(needRestored))
    {
        emit requestMoveNoteDb(id, defaultNotesFolder);
    }
}

void myListViewLogic::updateListViewLabel()
{
    QString l1, l2;
    if (m_listViewInfo.parentFolderId == SpecialNodeID::RootFolder)
    {
        l1 = "All Notes";
    }
    else if (m_listViewInfo.parentFolderId == SpecialNodeID::TrashFolder)
    {
        l1 = "Trash";
    }
    else
    {
        NodeData parentFolder;
        QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(NodeData, parentFolder),
                                  Q_ARG(int, m_listViewInfo.parentFolderId));
        l1 = parentFolder.fullTitle();
    }

    l2 = QString::number(m_listModel->rowCount());
    emit listViewLabelChanged(l1, l2);      //父节点和节点数



}

void myListViewLogic::onRowCountChanged()
{
    //关闭持久化编辑器
    m_listView->closeAllEditor();
    m_listDelegate->clearSizeMap();

    //遍历所有的行
    for (int i = 0; i < m_listModel->rowCount(); ++i)
    {
        auto index = m_listModel->index(i, 0);          //获取索引
        auto y = m_listView->visualRect(index).y();     //获取y坐标
        auto range = abs(m_listView->viewport()->height()); //列表的高度范围
        if (y < -range)
        {
            continue;
        }
        else if (y > 2 * range)
        {
            break;
        }

        //设置范围内的编辑器打开
        m_listView->openPersistentEditorC(index);
    }
}

void myListViewLogic::onNoteDoubleClicked(const QModelIndex &index)
{
    //点击的这个索引不合法或者不在搜索的范围内
    if (!index.isValid() || !m_listViewInfo.isInSearch)
    {
        return;
    }

    //否则，清除搜索
    auto id = index.data(NoteListModel::NoteID).toInt();
    clearSearch(false, id);
}

void myListViewLogic::onSetPinnedNoteRequested(const QModelIndexList &indexes, bool isPinned)
{
    m_listModel->setNotesIsPinned(indexes, isPinned);
}

void myListViewLogic::onListViewClicked()
{
    //笔记行数>1，不止一个
    if (m_listModel->rowCount() > 1)
    {
        //判断当前行是否是临时笔记，若是且在范围内，
        QModelIndex index = m_listView->currentIndex();
        if (m_listModel->data(index, NoteListModel::NoteIsTemp).toBool())
        {
            if (index.row() < m_listModel->rowCount() - 1)
            {
                selectNoteDown();
            }
            else
            {
                selectNoteUp();
            }
        }
    }
    //笔记行数<=1
    else
    {
        //如果是临时笔记，关闭
        QModelIndex index = m_listView->currentIndex();
        if (m_listModel->data(index, NoteListModel::NoteIsTemp).toBool())
        {
            emit closeNoteEditor();
        }
    }
}

void myListViewLogic::moveNoteToTop(const NodeData &note)
{
    //根据笔记获取索引
    QModelIndex noteIndex = m_listModel->getNoteIndex(note.id());

    if (noteIndex.isValid())
    {
        m_listView->scrollToTop(); //滚动到顶部

        // move the current selected note to the top (unless it's already there)
        QModelIndex destinationIndex;
        //笔记是置顶的就选择第一个笔记
        if (note.isPinnedNote())
        {
            destinationIndex = m_listModel->index(0);
        }
        //否则选择第一个普通区的索引
        else
        {
            destinationIndex = m_listModel->index(m_listModel->getFirstUnpinnedNote().row());
        }
        if (noteIndex == destinationIndex)
        {
            return;
        }
        //将这个索引移动到目标位置
        m_listModel->moveRow(noteIndex, noteIndex.row(), destinationIndex, destinationIndex.row());

        // update the current item
        noteIndex = destinationIndex;
        m_listView->setCurrentIndexC(noteIndex);
    }

    else
    {
        qDebug() << "ListViewLogic::moveNoteToTop : Note is not in list";
    }
}

void myListViewLogic::setNoteData(const NodeData &note)
{
    //获取索引
    QModelIndex noteIndex = m_listModel->getNoteIndex(note.id());

    if (noteIndex.isValid())
    {
        QMap<int, QVariant> dataValue;
        //auto wasTemp = noteIndex.data(NoteListModel::NoteIsTemp).toBool();
        dataValue[NoteListModel::NoteContent] = QVariant::fromValue(note.content());
        dataValue[NoteListModel::NoteFullTitle] = QVariant::fromValue(note.fullTitle());
        dataValue[NoteListModel::NoteLastModificationDateTime] =
            QVariant::fromValue(note.lastModificationdateTime());
        dataValue[NoteListModel::NoteIsTemp] = QVariant::fromValue(note.isTempNote());
        dataValue[NoteListModel::NoteScrollbarPos] = QVariant::fromValue(note.scrollBarPosition());
        m_listModel->setItemData(noteIndex, dataValue);
    }
    else
    {
        qDebug() << "ListViewLogic::moveNoteToTop : Note is not in list";
    }
}

void myListViewLogic::onNoteEditClosed(const NodeData &note, bool selectNext)
{
    //笔记是临时笔记
    if (note.isTempNote())
    {
        //获取这个笔记对应的索引
        QModelIndex noteIndex = m_listModel->getNoteIndex(note.id());
        if (noteIndex.isValid())
        {
            //获取这一行
            auto r = noteIndex.row();
            m_listModel->removeNotes({ noteIndex }); //移除


            //若选择下一行
            if (selectNext)
            {
                QModelIndex nextIndex = m_listView->model()->index(r + 1, 0);
                //下一行不合法就选择上一行
                if (!nextIndex.isValid())
                {
                    nextIndex = m_listView->model()->index(r - 1, 0);
                    //上一行也不合法就选择第1行
                    if (!nextIndex.isValid())
                    {
                        nextIndex = m_listModel->index(0, 0);
                    }
                }

                selectNote(nextIndex);
            }
        }
    }
}

void myListViewLogic::deleteNoteRequested(const NodeData &note)
{
    auto index = m_listModel->getNoteIndex(note.id());
    deleteNoteRequestedI({ index });
}

void myListViewLogic::selectNoteUp()
{
    //当前索引
    auto currentIndex = m_listView->currentIndex();
    if (currentIndex.isValid())
    {
        int currentRow = currentIndex.row();
        QModelIndex aboveIndex = m_listView->model()->index(currentRow - 1, 0); //上一行索引

        //上一行合法，选择上一行
        if (aboveIndex.isValid())
        {
            selectNote(aboveIndex);
            m_listView->setCurrentRowActive(false);
        }

        //搜索的内容为空，就聚焦到搜索框
        if (!m_searchEdit->text().isEmpty())
        {
            m_searchEdit->setFocus();
        }
        else
        {
            m_listView->setFocus();
        }
    }
    //不合法就选中第一行
    else
    {
        selectFirstNote();
    }

}

void myListViewLogic::selectNoteDown()
{
    auto currentIndex = m_listView->currentIndex();

    if (currentIndex.isValid())
    {

        int currentRow = currentIndex.row();
        QModelIndex belowIndex = m_listView->model()->index(currentRow + 1, 0);
        if (belowIndex.isValid())
        {
            selectNote(belowIndex);
            m_listView->setCurrentRowActive(false);
        }

        // if the searchEdit is not empty, set the focus to it
        if (!m_searchEdit->text().isEmpty())
        {
            m_searchEdit->setFocus();
        }
        else
        {
            m_listView->setFocus();
        }
    } else {
        selectFirstNote();
    }
}

void myListViewLogic::onSearchEditTextChanged(const QString &keyword)
{
    if (keyword.isEmpty())
    {
        clearSearch();
    }
    else
    {
        //当前列表不在被搜索
        if (!m_listViewInfo.isInSearch)
        {
            auto indexes = m_listView->selectedIndex();
            m_listViewInfo.currentNotesId.clear();
            //清空当前的笔记id，添加为选中的所有节点
            for (const auto &index : qAsConst(indexes))
            {
                if (index.isValid())
                {
                    m_listViewInfo.currentNotesId.insert(index.data(NoteListModel::NoteID).toInt());
                }
            }
        }
        m_clearButton->show();
        emit requestSearchInDb(keyword, m_listViewInfo); //在数据库中搜索
    }
}

void myListViewLogic::clearSearch(bool createNewNote, int scrollToId)
{
    m_listViewInfo.needCreateNewNote = createNewNote;   //是否需要创建新的笔记
    m_listViewInfo.scrollToId = scrollToId;             //滚动到目的位置
    emit requestClearSearchDb(m_listViewInfo);          //在数据库中清除搜索操作
    emit requestClearSearchUI();                        //清除ui
}


void myListViewLogic::onNoteMovedOut(int nodeId, int targetId)
{
    auto index = m_listModel->getNoteIndex(nodeId); //获取索引

    //节点合法
    if (index.isValid())
    {
        //父节点不是根节点且目标节点不是当前的父节点，或者目标节点是根节点
        if ((m_listViewInfo.parentFolderId != SpecialNodeID::RootFolder
             && m_listViewInfo.parentFolderId != targetId)
            || targetId == SpecialNodeID::TrashFolder)
        {
            selectNoteDown();//向下选择笔记，因为当前笔记要移出
            m_listModel->removeNotes({ index });//在当前的model移除
            if (m_listModel->rowCount() == 0)
            {
                emit closeNoteEditor();
            }
        }

        else
        {
            //通过节点id获取节点
            NodeData note;
            QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                                      Q_RETURN_ARG(NodeData, note), Q_ARG(int, nodeId));
            //节点合法
            if (note.id() != SpecialNodeID::InvalidNodeId)
            {
                m_listView->closePersistentEditorC(index);      //关闭持久化
                m_listModel->setNoteData(index, note);          //设置这个笔记的数据
                m_listView->openPersistentEditorC(index);       //打开持久化
            }
            else
            {
                qDebug() << __FUNCTION__ << "Note id" << nodeId << "not found!";
            }
        }
    }
}

void myListViewLogic::setLastSelectedNote()
{
    //选中的所有索引
    auto indexes = m_listView->selectedIndex();

    //将索引转换为id存储
    QSet<int> ids;
    for (const auto &index : qAsConst(indexes))
    {
        if (index.isValid())
        {
            ids.insert(index.data(NoteListModel::NoteID).toInt());
        }
    }

    setLastSavedState(ids, 0);
}

void myListViewLogic::loadLastSelectedNoteRequested()
{
      requestLoadSavedState(2);
}

void myListViewLogic::onNotesListInFolderRequested(int parentID, bool isRecursive, bool newNote, int scrollToId)
{
      //当前列表不在搜索状态并且搜索栏不为空
      if (m_listViewInfo.isInSearch && !m_searchEdit->text().isEmpty())
      {
        //设置当前列表的信息，并且展示清除按钮，然后在数据库中查询
        m_listViewInfo.parentFolderId = parentID;
        m_listViewInfo.currentNotesId.clear();
        m_listViewInfo.isInTag = true;
        m_listViewInfo.needCreateNewNote = false;
        m_listViewInfo.currentTagList = {};
        m_listViewInfo.scrollToId = SpecialNodeID::InvalidNodeId;
        m_clearButton->show();
        emit requestSearchInDb(m_searchEdit->text(), m_listViewInfo);
      }

      else
      {
        emit requestNotesListInFolder(parentID, isRecursive, newNote, scrollToId);
      }
}

void myListViewLogic::selectNotes(const QModelIndexList &indexes)
{
      m_listView->clearSelection(); //清除当前选中的
      m_listView->setSelectionMode(QAbstractItemView::MultiSelection); //设置多选模式

      //选中传入的笔记
      QModelIndex lastIdx;
      for (const auto index : qAsConst(indexes))
      {
        if (index.isValid())
        {
            lastIdx = index;
            m_listView->selectionModel()->select(index, QItemSelectionModel::Select);

            // m_listView->setCurrentIndex(index);
            // m_listView->selectionModel()->setCurrentIndex(index,
            // QItemSelectionModel::SelectCurrent);
        }
      }

      //设置最后一个索引
      onNotePressed(indexes);
}


