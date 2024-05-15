#ifndef NODEPATH_H
#define NODEPATH_H

#include<QString>


#define NOTE_MIME "application/x-notenode"
#define FOLDER_MIME "application/x-foldernode"
#define PATH_SEPARATOR "/"


class NodePath
{
public:
    NodePath(const QString &path);
    //获取"所有笔记"文件夹的路径
    static QString getAllNoteFolderPath();
    //获取垃圾桶文件的路径
    static QString getTrashFolderPath();
};

#endif // NODEPATH_H
