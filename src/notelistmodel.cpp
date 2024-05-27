#include "notelistmodel.h"

NoteListModel::NoteListModel(QObject *parent)
    : QAbstractListModel{parent}
{

}

NoteListModel::~NoteListModel()
{

}

QModelIndex NoteListModel::addNote(const NodeData &note)
{
    //根据笔记的是否置顶，选择添加的位置
    if (!note.isPinnedNote())
    {
        const int rowCnt = rowCount();

        beginInsertRows(QModelIndex(), rowCnt, rowCnt);
        m_noteList << note;
        endInsertRows();

        emit rowsInsertedC({ createIndex(rowCnt, 0) }); //真正改变
        emit rowCountChanged();

        return createIndex(rowCnt, 0);  //返回这个笔记的索引

    }
    else
    {
        //笔记是置顶状态，则在置顶区添加
        const int rowCnt = m_pinnedList.size();

        beginInsertRows(QModelIndex(), rowCnt, rowCnt);
        m_pinnedList << note;
        endInsertRows();

        emit rowsInsertedC({ createIndex(rowCnt, 0) });
        emit rowCountChanged();
        return createIndex(rowCnt, 0);
    }
}

QModelIndex NoteListModel::insertNote(const NodeData &note, int row)
{
    //若笔记需要置顶状态
    if (note.isPinnedNote())
    {
        if (row > m_pinnedList.size())
        {
            row = m_pinnedList.size();
        }
        else if (row < 0)
        {
            row = 0;
        }

        beginInsertRows(QModelIndex(), row, row);
        m_pinnedList.insert(row, note);
        endInsertRows();

        emit rowsInsertedC({ createIndex(row, 0) });
        emit rowCountChanged();
        return createIndex(row, 0);
    }

    //不是置顶状态则添加到普通笔记
    else
    {
        if (row < m_pinnedList.size())
        {
            row = m_pinnedList.size();
        }
        else if (row > (m_pinnedList.size() + m_noteList.size()))
        {
            row = m_pinnedList.size() + m_noteList.size();
        }
        beginInsertRows(QModelIndex(), row, row);
        m_noteList.insert(row - m_pinnedList.size(), note);
        endInsertRows();
        emit rowsInsertedC({ createIndex(row, 0) });
        emit rowCountChanged();
        return createIndex(row, 0);
    }
}

void NoteListModel::removeNotes(const QModelIndexList &noteIndexes)
{
    emit requestRemoveNotes(noteIndexes);
}

bool NoteListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    //传入的行数不合法
    if (row < 0 || (row + count) > (m_pinnedList.size() + m_noteList.size()))
    {
        return false;
    }

    //移除
    beginRemoveRows(parent, row, row + count - 1);
    for (int r = row; r < row + count; ++r)
    {
        if (r < m_pinnedList.size())  //置顶的笔记
        {
            m_pinnedList.takeAt(r);//移除
        }
        else                          //普通笔记
        {
            auto rr = r - m_pinnedList.size();
            m_noteList.takeAt(rr);
        }
    }
    endRemoveRows();

    emit rowCountChanged(); //这里改变了，实施index更新
    return true;
}

bool NoteListModel::moveRow(const QModelIndex &sourceParent, int sourceRow, const QModelIndex &destinationParent, int destinationChild)
{
    //判断传入的源行和目标行的值是否合法
    if (sourceRow < 0 || sourceRow >= rowCount() || destinationChild < 0
        || destinationChild >= rowCount())
    {
        return false;
    }


    //从置顶区移入置顶区,直接移动
    if (sourceRow < m_pinnedList.size() && destinationChild < m_pinnedList.size())
    {
        if (beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent,
                          destinationChild))
        {
            m_pinnedList.move(sourceRow, destinationChild); //移动位置
            endMoveRows();

            emit rowsAboutToBeMovedC({ createIndex(sourceRow, 0) });
            emit rowsMovedC({ createIndex(destinationChild, 0) });
            emit rowCountChanged();

            return true;
        }
    }

    //从普通区移入置顶区
    if (sourceRow >= m_pinnedList.size() && destinationChild >= m_pinnedList.size())
    {
        //获取在普通区的相对索引
        sourceRow = sourceRow - m_pinnedList.size();
        destinationChild = destinationChild - m_pinnedList.size();
        if (beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent,
                          destinationChild))
        {
            m_noteList.move(sourceRow, destinationChild);   //移动
            endMoveRows();

            emit rowsAboutToBeMovedC({ createIndex(sourceRow, 0) });
            emit rowsMovedC({ createIndex(destinationChild + 1, 0) });
            emit rowCountChanged();
            return true;
        }
    }
    return false;
}

void NoteListModel::clearNotes()
{
    beginResetModel();          //开始重置model
    m_pinnedList.clear();       //清除成员变量
    m_noteList.clear();
    endResetModel();

    emit rowCountChanged();     //发送行数改变的信号
}

void NoteListModel::setNoteData(const QModelIndex &index, const NodeData &note)
{
    if (!index.isValid())
    {
        return;
    }

    //在置顶区
    auto row = index.row();
    if (row < m_pinnedList.size())
    {
        m_pinnedList[row] = note;
    }
    //普通区
    else
    {
        row = row - m_pinnedList.size();
        m_noteList[row] = note;
    }

    emit dataChanged(index,index);   //this->index(index.row()), this->index(index.row())
}

const NodeData &NoteListModel::getNote(const QModelIndex &index) const
{
    auto row = index.row();

    //分为置顶区和普通区
    if (row < m_pinnedList.size())
    {
        return m_pinnedList.at(row);
    }
    else
    {
        row = row - m_pinnedList.size();  //置顶区和普通区的index是一起的，但从index找到普通区的节点，需要减去置顶区的笔记数
        return m_noteList.at(row);
    }
}

QModelIndex NoteListModel::getNoteIndex(int id) const
{
    //一个个找，遍历置顶区和普通区
    for (int i = 0; i < m_pinnedList.size(); ++i)
    {
        if (m_pinnedList[i].id() == id)
        {
            return createIndex(i, 0);
        }
    }


    for (int i = 0; i < m_noteList.size(); ++i)
    {
        if (m_noteList[i].id() == id)
        {
            return createIndex(i + m_pinnedList.size(), 0);
        }
    }

    return QModelIndex{};
}

void NoteListModel::setListNote(const QVector<NodeData> &notes, const ListViewInfo &inf)
{
    //先重置,然后处理
    beginResetModel();

    m_pinnedList.clear();
    m_noteList.clear();
    m_listViewInfo = inf;

    //列表的父节点不是垃圾桶
    if ((m_listViewInfo.parentFolderId != SpecialNodeID::TrashFolder))
    {
        //添加到置顶区和非置顶区
        for (const auto &note : qAsConst(notes))
        {
            if (note.isPinnedNote())
            {
                m_pinnedList.append(note);
            }
            else
            {
                m_noteList.append(note);
            }
        }
    }
    //父节点是垃圾桶，就不分是否置顶了
    else
    {
        m_noteList = notes;
    }

    sort(0, Qt::AscendingOrder); //升序排序节点

    endResetModel();


    emit rowCountChanged();
}

QVariant NoteListModel::data(const QModelIndex &index, int role) const
{
    //传入的索引范围不合法
    if (index.row() < 0 || index.row() >= (m_noteList.count() + m_pinnedList.count()))
    {
        return QVariant();
    }
    //传入的需求信息不合法
    if (role < NoteID || role > NoteIsPinned)
    {
        return QVariant();
    }

    //获取这个笔记节点数据
    const NodeData &note = getRef(index.row());

    //根据role枚举类型，返回想要的对应的数据

    //笔记ID
    if (role == NoteID)
    {
        return note.id();
    }
    //笔记标题
    else if (role == NoteFullTitle)
    {
        auto text = note.fullTitle().trimmed(); //除去前后空白符
        //响应修改
        if (text.startsWith("#"))
        {
            text.remove(0, 1);
            text = text.trimmed();
        }
        return text;
    }
    //笔记创建时间
    else if (role == NoteCreationDateTime)
    {
        return note.creationDateTime();
    }
    //上次修改时间
    else if (role == NoteLastModificationDateTime)
    {
        return note.lastModificationdateTime();
    }
    //删除(移入垃圾桶)的时间
    else if (role == NoteDeletionDateTime)
    {
        return note.deletionDateTime();
    }
    //笔记的内容
    else if (role == NoteContent)
    {
        return note.content();
    }
    //笔记滚动的位置
    else if (role == NoteScrollbarPos)
    {
        return note.scrollBarPosition();
    }
    //是否为临时笔记，也就是刚创建是否要保存
    else if (role == NoteIsTemp)
    {
        return note.isTempNote();
    }
    //笔记的父节点名称
    else if (role == NoteParentName)
    {
        return note.parentName();
    }
    //笔记是否置顶
    else if (role == NoteIsPinned)
    {
        return note.isPinnedNote();
    }

    return QVariant();
}

bool NoteListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    //判断索引是否合法
    if (index.row() < 0 || index.row() >= (m_noteList.count() + m_pinnedList.count()))
    {

        return false;
    }


    //通过行数获取节点数据
    NodeData &note = getRef(index.row());

    if (role == NoteID)
    {
        note.setId(value.toInt());
    }
    else if (role == NoteFullTitle)
    {
        note.setFullTitle(value.toString());
    }
    else if (role == NoteCreationDateTime)
    {
        note.setCreationDateTime(value.toDateTime());
    }
    else if (role == NoteLastModificationDateTime)
    {
        note.setLastModificationDateTime(value.toDateTime());
    }
    else if (role == NoteDeletionDateTime)
    {
        note.setDeletionDateTime(value.toDateTime());
    }
    else if (role == NoteContent)
    {
        note.setContent(value.toString());
    }
    else if (role == NoteScrollbarPos)
    {
        note.setScrollBarPosition(value.toInt());
    }
    else if (role == NoteIsTemp)
    {
        note.setIsTempNote(value.toBool());
    }
    else if (role == NoteParentName)
    {
        note.setParentName(value.toString());
    }
    else
    {
        return false;
    }

    //this->index(index.row()), this->index(index.row())
    emit dataChanged(index,index, QVector<int>(1, role));//发送节点数据改变的信号，通知数据库或者视图改变
    return true;
}

Qt::ItemFlags NoteListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;  //可用可拖拽
    }

    return QAbstractListModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled
           | Qt::ItemIsDropEnabled;  //基类标志位+可用可拖拽+可编辑
}

int NoteListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_noteList.size() + m_pinnedList.size();
}

void NoteListModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(column)
    Q_UNUSED(order)

    //父节点是垃圾桶，根据删除时间降序，越大越先
    if (m_listViewInfo.parentFolderId == SpecialNodeID::TrashFolder)
    {
        std::stable_sort(m_noteList.begin(), m_noteList.end(),
                         [](const NodeData &lhs, const NodeData &rhs) {
                             return lhs.deletionDateTime() > rhs.deletionDateTime();
                         });
    }
    //父节点不是垃圾桶
    else
    {
        //先排置顶区，如果是在allnote文件夹内，根据relativePosAN；否则根据relativePosition
        std::stable_sort(m_pinnedList.begin(), m_pinnedList.end(),
                         [this](const NodeData &lhs, const NodeData &rhs)
                         {
                             if (isInAllNote())
                             {
                                 return lhs.relativePosAN() < rhs.relativePosAN();
                             }
                             else
                             {
                                 return lhs.relativePosition() < rhs.relativePosition();
                             }
                         });
        //再排普通区，根据上次修改的时间排序，越大越近
        std::stable_sort(m_noteList.begin(), m_noteList.end(),
                         [](const NodeData &lhs, const NodeData &rhs)
                         {
                             return lhs.lastModificationdateTime() > rhs.lastModificationdateTime();
                         });
    }

    emit dataChanged(index(0), index(rowCount() - 1));
}

Qt::DropActions NoteListModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions NoteListModel::supportedDragActions() const
{
    return Qt::MoveAction;
}

QStringList NoteListModel::mimeTypes() const
{
    return QStringList() << NOTE_MIME;
}

QMimeData *NoteListModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty())
    {
        return nullptr;
    }

    QStringList d; //所有的节点的id
    for (const auto &index : indexes)
    {
        auto id = index.data(NoteListModel::NoteID).toInt();
        d.append(QString::number(id));
    }
    if (d.isEmpty())
    {
        return nullptr;
    }

    QMimeData *mimeData = new QMimeData;  //处理剪贴板和拖放操作的核心数据模型类
    mimeData->setData(NOTE_MIME, d.join(QStringLiteral(PATH_SEPARATOR)).toUtf8()); //list用 '/'连接起来
    return mimeData;
}

bool NoteListModel::dropMimeData(const QMimeData *mime, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(column);

    //不包含笔记属性且不是移动行为
    if (!(mime->hasFormat(NOTE_MIME) && action == Qt::MoveAction))
    {
        return false;
    }

    //传入的行不合法，若父节点合法row置为父节点；否则直接为底部
    if (row == -1)
    {
        // valid index: drop onto item
        if (parent.isValid())
        {
            row = parent.row();
        }
        else
        {
            // invalid index: append at bottom, after last toplevel
            row = rowCount(parent);
        }
    }

    //根据与置顶区的数量比较,来判断是否要置顶
    bool toPinned = false;
    if (row >= m_pinnedList.size())
    {
        toPinned = false;
    }
    else
    {
        toPinned = true;
    }


    //将之前mimeData的所有节点的id用/连起来，现在用/隔开
    auto idl = QString::fromUtf8(mime->data(NOTE_MIME)).split(QStringLiteral(PATH_SEPARATOR));
    QSet<int> movedIds;
    QModelIndexList idxe;

    //遍历所有节点，通过节点id获取index，将index添加到idxe
    for (const auto &id_s : qAsConst(idl))
    {
        auto nodeId = id_s.toInt();
        idxe.append(getNoteIndex(nodeId));
    }

    //通过视图全面更新
    emit rowsAboutToBeMovedC(idxe);
    beginResetModel();


    //要置顶
    if (toPinned)
    {
        //遍历刚刚获取的index，未被置顶则置顶
        for (const auto &index : qAsConst(idxe))
        {
            auto &note = getRef(index.row()); //通过行获取nodedata
            if (!note.isPinnedNote())
            {
                note.setIsPinnedNote(true);
                emit requestUpdatePinned(note.id(), true);
                m_pinnedList.prepend(m_noteList.takeAt(index.row() - m_pinnedList.size()));//从notelist取出，放到pinnedlist中
            }
        }
        //遍历获取的id
        for (const auto &id_s : qAsConst(idl))
        {
            auto nodeId = id_s.toInt();
            //在置顶的节点中找到当前处理的节点，将这个数据项移动位置
            for (int i = 0; i < m_pinnedList.size(); ++i)
            {
                if (m_pinnedList[i].id() == nodeId)
                {
                    m_pinnedList.move(i, row);
                    break;
                }
            }
            movedIds.insert(nodeId);    //记录哪些节点被移动了

        }
    }


    //不要置顶
    else
    {
        //遍历所有index，通过row获取节点，插入到移动节点的set中
        for (const auto &index : qAsConst(idxe))
        {
            auto &note = getRef(index.row());
            movedIds.insert(note.id());

            //节点不用置顶，继续
            if (!note.isPinnedNote())
            {
                continue;
            }
            //如果是置顶，设置为不用置顶
            note.setIsPinnedNote(false);
            emit requestUpdatePinned(note.id(), false);

            //如果父节点是垃圾桶，根据删除时间找到应该插入的位置
            int destinationChild = 0;
            if (m_listViewInfo.parentFolderId == SpecialNodeID::TrashFolder)
            {
                auto lastMod = index.data(NoteDeletionDateTime).toDateTime();
                for (destinationChild = 0; destinationChild < m_noteList.size();
                     ++destinationChild)
                {
                    if (m_noteList[destinationChild].deletionDateTime() <= lastMod)
                    {
                        break;
                    }
                }
            }
            //父节点不是垃圾桶，根据上次修改时间选择插入位置
            else
            {
                auto lastMod = index.data(NoteLastModificationDateTime).toDateTime();
                for (destinationChild = 0; destinationChild < m_noteList.size();
                     ++destinationChild)
                {
                    if (m_noteList[destinationChild].lastModificationdateTime() <= lastMod)  //deletionDateTime错的应该
                    {
                        break;
                    }
                }
            }

            //找到位置后，就插入
            m_noteList.insert(destinationChild, m_pinnedList.takeAt(index.row()));
        }
    }

    endResetModel();



    QModelIndexList destinations;
    //获取所有要移动的节点的index
    for (const auto &id : movedIds)
    {
        //通过id获取index
        auto index = getNoteIndex(id);
        if (!index.isValid())
        {
            continue;
        }
        destinations.append(index);
    }

    //发送信号，通知其他对象进行调整
    emit selectNotes(destinations); //选择的笔记
    emit rowsMovedC(destinations);  //要移动的行
    updatePinnedRelativePosition(); //更新置顶的相对位置

    return true;
}

bool NoteListModel::isFirstPinnedNote(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return false;
    }
    if (index.row() > 0)
    {
        return false;
    }

    //通过row获取index
    const NodeData &note = getRef(index.row());
    if (index.row() == 0 && note.isPinnedNote()) //行为0，且是置顶区
    {
        return true;
    }

    return false;
}

bool NoteListModel::isFirstUnpinnedNote(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return false;
    }

    //通过row获取index
    const NodeData &note = getRef(index.row());
    if ( (index.row() - m_pinnedList.size() ) == 0 && !note.isPinnedNote()) //行为0，且是置顶区
    {
        return true;
    }

    return false;
}

QModelIndex NoteListModel::getFirstPinnedNote() const
{
    if (m_pinnedList.isEmpty())
    {
        return QModelIndex();
    }
    return createIndex(0, 0);
}

QModelIndex NoteListModel::getFirstUnpinnedNote() const
{
    //笔记区不为空，笔记区的第一个(置顶区的最后一个 +1 = size)
    if (!m_noteList.isEmpty())
    {
        return createIndex(m_pinnedList.size(), 0);
    }
    else
    {
        return QModelIndex();
    }
}

bool NoteListModel::hasPinnedNote() const
{
    return !m_pinnedList.isEmpty(); //不为空
}

void NoteListModel::setNotesIsPinned(const QModelIndexList &indexes, bool isPinned)
{
    emit requestCloseNoteEditor(indexes);
    QSet<int> needMovingIds;
    QModelIndexList needMovingIndexes;
    for (const auto &index : indexes)
    {
        if (index.isValid())
        {
            NodeData &note = getRef(index.row());
            if (note.isPinnedNote() != isPinned)
            {
                needMovingIds.insert(note.id());
                needMovingIndexes.append(index);
                note.setIsPinnedNote(isPinned);
                emit requestUpdatePinned(note.id(), isPinned);
            }
        }
    }

    if (isPinned)
    {
        emit rowsAboutToBeMovedC(needMovingIndexes);
        beginResetModel();
        for (const auto &id : qAsConst(needMovingIds))
        {
            auto index = getNoteIndex(id);
            if (!index.isValid())
            {
                continue;
            }
            int sourceRow = index.row();
            if (sourceRow < 0 || sourceRow >= rowCount())
            {
                continue;
            }
            m_pinnedList.prepend(m_noteList.takeAt(sourceRow - m_pinnedList.size()));
        }
        endResetModel();
        QModelIndexList destinations;
        for (const auto &id : needMovingIds)
        {
            auto index = getNoteIndex(id);
            if (!index.isValid())
            {
                continue;
            }
            destinations.append(index);
        }
        emit selectNotes(destinations);
        emit rowsMovedC(destinations);
        emit rowCountChanged();
        updatePinnedRelativePosition();
    }
    else
    {
        emit rowsAboutToBeMovedC(needMovingIndexes);
        beginResetModel();
        for (const auto &id : qAsConst(needMovingIds))
        {
            auto index = getNoteIndex(id);
            if (!index.isValid())
            {
                continue;
            }
            int destinationChild = 0;
            if (m_listViewInfo.parentFolderId == SpecialNodeID::TrashFolder)
            {
                auto lastMod = index.data(NoteDeletionDateTime).toDateTime();
                for (destinationChild = 0; destinationChild < m_noteList.size();
                     ++destinationChild)
                {
                    const auto &note = m_noteList[destinationChild];
                    if (note.deletionDateTime() <= lastMod)
                    {
                        break;
                    }
                }
            }
            else
            {
                auto lastMod = index.data(NoteLastModificationDateTime).toDateTime();
                for (destinationChild = 0; destinationChild < m_noteList.size();
                     ++destinationChild)
                {
                    const auto &note = m_noteList[destinationChild];
                    if (note.lastModificationdateTime() <= lastMod)
                    {
                        break;
                    }
                }
            }
            m_noteList.insert(destinationChild, m_pinnedList.takeAt(index.row()));
        }
        endResetModel();
        QModelIndexList destinations;
        for (const auto &id : needMovingIds)
        {
            auto index = getNoteIndex(id);
            if (!index.isValid())
            {
                continue;
            }
            destinations.append(index);
        }
        emit selectNotes(destinations);
        emit rowsMovedC(destinations);
        emit rowCountChanged();
        updatePinnedRelativePosition();
    }
}

void NoteListModel::updatePinnedRelativePosition()
{

}

bool NoteListModel::isInAllNote() const
{

}

NodeData &NoteListModel::getRef(int row)
{

}

const NodeData &NoteListModel::getRef(int row) const
{

}












