#ifndef NODEDATA_H
#define NODEDATA_H

#include<QObject>

namespace SpecialNodeID
{
enum Value {
    InvalidNodeId = -1,
    RootFolder = 0,
    TrashFolder = 1,
    DefaultNotesFolder = 2,
};
}


//包含一个节点项的所有数据
class NodeData
{
public:
    enum Type { Note = 0, Folder };
    NodeData();
    int id() const;
    int parentId() const;
    NodeData::Type nodeType() const;
    const QString &absolutePath() const;
    int relativePosition() const;
    int childNotesCount() const;
    QString fullTitle() const;
};

#endif // NODEDATA_H
