#ifndef DEFAULTNOTEFOLDERDELEGATEEDITOR_H
#define DEFAULTNOTEFOLDERDELEGATEEDITOR_H

#include<QObject>
#include<QWidget>
#include<QTreeView>
#include<QStyleOptionViewItem>
#include<QListView>
#include<QModelIndex>
#include"mytreeview.h"


class DefaultNoteFolderDelegateEditor : public QWidget
{
    Q_OBJECT
public:
    explicit DefaultNoteFolderDelegateEditor(QTreeView *view, const QStyleOptionViewItem &option,
                                             const QModelIndex &index, QListView *listView,
                                             QWidget *parent = nullptr);

    void setTheme(Theme::Value theme);
};

#endif // DEFAULTNOTEFOLDERDELEGATEEDITOR_H
