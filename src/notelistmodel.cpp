#include "notelistmodel.h"

NoteListModel::NoteListModel(QObject *parent)
    : QAbstractItemModel{parent}
{

}

QModelIndex NoteListModel::getNoteIndex(int id)
{
    return QModelIndex{};
}

bool NoteListModel::isFirstPinnedNote(const QModelIndex &index) const
{
    return true;
}

bool NoteListModel::hasPinnedNote() const
{
    return true;
}

bool NoteListModel::isFirstUnpinnedNote(const QModelIndex &index) const
{
    return true;
}
