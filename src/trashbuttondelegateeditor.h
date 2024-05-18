#ifndef TRASHBUTTONDELEGATEEDITOR_H
#define TRASHBUTTONDELEGATEEDITOR_H

#include<QObject>
#include<QWidget>
#include<QTreeView>
#include<QStyleOptionViewItem>
#include<QListView>
#include<QModelIndex>
#include"mytreeview.h"

class TrashButtonDelegateEditor : public QWidget
{
    Q_OBJECT
public:
    explicit TrashButtonDelegateEditor(QTreeView *view, const QStyleOptionViewItem &option,
                                       const QModelIndex &index, QListView *listView,
                                       QWidget *parent = nullptr);

    void setTheme(Theme::Value theme);
};

#endif // TRASHBUTTONDELEGATEEDITOR_H
