#include "dbmanager.h"

DBManager::DBManager(QObject *parent) : QObject(parent)
{

}

NodePath DBManager::getNodeAbsolutePath(int nodeId)
{
    return NodePath("");
}

NodeData DBManager::getNode(int nodeId)
{
    return NodeData{};
}

void DBManager::moveFolderToTrash(const NodeData &node)
{

}

FolderListType DBManager::getFolderList()
{
    return FolderListType{};
}

void DBManager::renameNode(int id, const QString &newName)
{

}

void DBManager::moveNode(int nodeId, const NodeData &target)
{

}

void DBManager::updateRelPosNode(int nodeId, int relPos)
{

}
