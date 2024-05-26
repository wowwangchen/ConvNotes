#include "notelistmodel.h"

NoteListModel::NoteListModel(QObject *parent)
    : QAbstractItemModel{parent}
{

}

QModelIndex NoteListModel::getNoteIndex(int id)
{
    return QModelIndex{};
}
