#ifndef ALLNOTEBUTTONTREEDELEGATEEDITOR_H
#define ALLNOTEBUTTONTREEDELEGATEEDITOR_H

#include<QObject>
#include<QWidget>
#include<QTreeView>
#include<QStyleOptionViewItem>
#include<QListView>
#include<QModelIndex>
#include<QPaintEvent>
#include"mytreeview.h"


//所有笔记按钮的编辑代理
class AllNoteButtonTreeDelegateEditor : public QWidget
{
    Q_OBJECT
public:
    explicit AllNoteButtonTreeDelegateEditor(QTreeView *view, const QStyleOptionViewItem &option,
                                             const QModelIndex &index, QListView *listView,
                                             QWidget *parent = nullptr);

    void setTheme(Theme::Value theme);


private:
    QStyleOptionViewItem        m_option;                           //视图样式
    QModelIndex                 m_index;                            //当前索引
    QString                     m_displayFont;                      //展示内容的字体
    QFont                       m_titleFont;                        //标题字体
    QFont                       m_numberOfNotesFont;                //笔记数量的字体
    QColor                      m_titleColor;                       //标题颜色
    QColor                      m_titleSelectedColor;               //标题被选中时的颜色
    QColor                      m_activeColor;                      //项处于活动状态的颜色
    QColor                      m_hoverColor;                       //鼠标悬停时的颜色
    QColor                      m_folderIconColor;                  //文件夹图标的颜色
    QColor                      m_numberOfNotesColor;               //笔记数量的颜色
    QColor                      m_numberOfNotesSelectedColor;       //被选中时的笔记数量的颜色
    QTreeView *                 m_view;                             //treeView
    QListView *                 m_listView;                         //listView
    Theme::Value                m_theme;                            //主题

protected:
    virtual void paintEvent(QPaintEvent *event) override;
};

#endif // ALLNOTEBUTTONTREEDELEGATEEDITOR_H
