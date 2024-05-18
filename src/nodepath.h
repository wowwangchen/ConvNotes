#ifndef NODEPATH_H
#define NODEPATH_H

#include<QString>
#include<QStringList>

//不同类型的宏定义
#define FOLDER_MIME "application/x-foldernode"
#define NOTE_MIME "application/x-notenode"
#define PATH_SEPARATOR "/"


//节点路径
class NodePath
{
public:
    NodePath(const QString &path);
    //以'/'对m_path进行分割
    QStringList separate() const;
    //获取"所有笔记"文件夹的路径
    static QString getAllNoteFolderPath();
    //获取垃圾桶文件的路径
    static QString getTrashFolderPath();

    QString path() const;
    NodePath parentPath() const;

private:
    QString m_path;
};

#endif // NODEPATH_H
