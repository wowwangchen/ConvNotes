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

class NodeData
{
public:
    NodeData();
};

#endif // NODEDATA_H
