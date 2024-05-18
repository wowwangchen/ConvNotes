#include "nodepath.h"
#include"nodedata.h"

NodePath::NodePath(const QString &path):m_path(path)
{
}

QStringList NodePath::separate() const
{

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    return m_path.split(PATH_SEPARATOR, QString::SkipEmptyParts);
#else
    //第二个参数是跳过空的部分的作用
    return m_path.split(PATH_SEPARATOR, Qt::SkipEmptyParts);
#endif

}

QString NodePath::getAllNoteFolderPath()
{
    return PATH_SEPARATOR + QString::number(SpecialNodeID::RootFolder);
}

QString NodePath::getTrashFolderPath()
{
    /*    /id1/id2      */
    return PATH_SEPARATOR + QString::number(SpecialNodeID::RootFolder) + PATH_SEPARATOR
           + QString::number(SpecialNodeID::TrashFolder);
}

QString NodePath::path() const
{
    return m_path;
}

NodePath NodePath::parentPath() const
{
    auto s = separate();              //QStringList
    s.takeLast();                     //从列表末尾移出并返回最后一个元素，这里相当于返回最后一个元素
    return s.join(PATH_SEPARATOR);    //用分割符将QStringList连接起来，返回
}


