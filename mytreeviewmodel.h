#ifndef MYTREEVIEWMODEL_H
#define MYTREEVIEWMODEL_H

#include <QObject>
#include<QAbstractItemModel>
#include"nodepath.h"


//树形结构的节点项空间，所有节点的类型
namespace NodeItem
{
    // We store this enum inside QVariant,
    // and an invalid QVariant conversion return 0\
//对不同的项分类
enum Type {
    AllNoteButton = 1,   //"所有笔记"按钮项
    TrashButton,         //垃圾桶
    FolderSeparator,     //文件夹分隔线
    //TagSeparator,
    FolderItem,          //文件夹项
    NoteItem,            //笔记项
    //TagItem,
    RootItem             //根节点项
};

//对项的不同属性进行分类
enum Roles
{
    ItemType = Qt::UserRole,        //项的类型
    DisplayText = Qt::DisplayRole,  //项显示的内容
    Icon = Qt::DecorationRole,      //项的图标
    TagColor = Qt::UserRole + 1,    //项的标签颜色
    IsExpandable,                   //是否可展开
    AbsPath,                        //绝对路径 absolute
    RelPos,                         //相对路径 relative
    ChildCount,                     //子项的数量
    NodeId                          //节点ID
};
}


class NodeTreeItem
{

};



class myTreeViewModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    myTreeViewModel();
    //获取垃圾桶节点的索引
    QModelIndex getTrashButtonIndex();
    //从path中获取文件夹的索引
    QModelIndex folderIndexFromIdPath(const NodePath &idPath);
    //获取"所有笔记按钮"的索引
    QModelIndex getAllNotesButtonIndex();
};

#endif // MYTREEVIEWMODEL_H
