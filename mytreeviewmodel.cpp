#include "mytreeviewmodel.h"

myTreeViewModel::myTreeViewModel()
{

}

QModelIndex myTreeViewModel::getTrashButtonIndex()
{
    return QModelIndex{};
}

QModelIndex myTreeViewModel::folderIndexFromIdPath(const NodePath &idPath)
{
    Q_UNUSED(idPath);
    return QModelIndex{};
}

QModelIndex myTreeViewModel::getAllNotesButtonIndex()
{
    return QModelIndex{};
}
