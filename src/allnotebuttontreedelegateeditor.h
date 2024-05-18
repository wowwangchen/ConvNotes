#ifndef ALLNOTEBUTTONTREEDELEGATEEDITOR_H
#define ALLNOTEBUTTONTREEDELEGATEEDITOR_H

#include<QObject>
#include<QWidget>
#include<QTreeView>
#include<QStyleOptionViewItem>
#include<QListView>
#include<QModelIndex>
#include"mytreeview.h"
class AllNoteButtonTreeDelegateEditor : public QWidget
{
    Q_OBJECT
public:
    explicit AllNoteButtonTreeDelegateEditor(QTreeView *view, const QStyleOptionViewItem &option,
                                             const QModelIndex &index, QListView *listView,
                                             QWidget *parent = nullptr);

    void setTheme(Theme::Value theme);
};

#endif // ALLNOTEBUTTONTREEDELEGATEEDITOR_H
