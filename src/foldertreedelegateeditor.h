#ifndef FOLDERTREEDELEGATEEDITOR_H
#define FOLDERTREEDELEGATEEDITOR_H

#include<QObject>
#include<QWidget>
#include<QTreeView>
#include<QStyleOptionViewItem>
#include<QListView>
#include<QModelIndex>
#include"mytreeview.h"



class FolderTreeDelegateEditor : public QWidget
{
    Q_OBJECT
public:
    explicit FolderTreeDelegateEditor(QTreeView *view, const QStyleOptionViewItem &option,
                                      const QModelIndex &index, QListView *listView,
                                      QWidget *parent = nullptr);

    void setTheme(Theme::Value theme);
};

#endif // FOLDERTREEDELEGATEEDITOR_H
