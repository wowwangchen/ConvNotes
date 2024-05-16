#include "nodepath.h"

NodePath::NodePath(const QString &path)
{
    Q_UNUSED(path);
}

QString NodePath::getAllNoteFolderPath()
{
    return "";
}

QString NodePath::getTrashFolderPath()
{
    return "";
}

QStringList NodePath::separate() const
{
    return QStringList{};
}
