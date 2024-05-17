#include "nodedata.h"

NodeData::NodeData()
{

}

int NodeData::id() const
{
    return 0;
}

int NodeData::parentId() const
{
    return 0;
}

NodeData::Type NodeData::nodeType() const
{
    return NodeData::Type::Folder;
}

const QString &NodeData::absolutePath() const
{
    QString s="";
    QString &a=s;
    return a;
}

int NodeData::relativePosition() const
{
    return 0;
}

int NodeData::childNotesCount() const
{
    return 0;
}

QString NodeData::fullTitle() const
{
    return "";
}
