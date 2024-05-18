#ifndef DEFAULTNOTEFOLDERDELEGATEEDITOR_H
#define DEFAULTNOTEFOLDERDELEGATEEDITOR_H

#include<QObject>
#include<QWidget>
#include<QTreeView>
#include<QStyleOptionViewItem>
#include<QListView>
#include<QModelIndex>
#include"mytreeview.h"


//默认笔记文件夹的编辑代理
class DefaultNoteFolderDelegateEditor : public QWidget
{
    Q_OBJECT
public:
    explicit DefaultNoteFolderDelegateEditor(QTreeView *view, const QStyleOptionViewItem &option,
                                             const QModelIndex &index, QListView *listView,
                                             QWidget *parent = nullptr);

    void setTheme(Theme::Value theme);

private:        //见allnotebuttontreedelegate
    QStyleOptionViewItem m_option;
    QModelIndex m_index;
    QString m_displayFont;
    QFont m_titleFont;
    QFont m_numberOfNotesFont;
    QColor m_titleColor;
    QColor m_titleSelectedColor;
    QColor m_activeColor;
    QColor m_hoverColor;
    QColor m_folderIconColor;
    QColor m_numberOfNotesColor;
    QColor m_numberOfNotesSelectedColor;
    QTreeView *m_view;
    QListView *m_listView;
    Theme::Value m_theme;
    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
};

#endif // DEFAULTNOTEFOLDERDELEGATEEDITOR_H
