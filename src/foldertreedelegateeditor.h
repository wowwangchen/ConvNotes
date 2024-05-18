#ifndef FOLDERTREEDELEGATEEDITOR_H
#define FOLDERTREEDELEGATEEDITOR_H

#include<QObject>
#include<QWidget>
#include<QTreeView>
#include<QStyleOptionViewItem>
#include<QListView>
#include<QModelIndex>
#include<QMouseEvent>
#include"mytreeview.h"
#include"labeledittype.h"
#include"pushbuttontype.h"


//普通文件夹的编辑代理
class FolderTreeDelegateEditor : public QWidget
{
    Q_OBJECT
public:
    explicit FolderTreeDelegateEditor(QTreeView *view, const QStyleOptionViewItem &option,
                                      const QModelIndex &index, QListView *listView,
                                      QWidget *parent = nullptr);

    void setTheme(Theme::Value theme);


private:
    QStyleOptionViewItem        m_option;               //对于项的属性的描述
    QModelIndex                 m_index;                //当前编辑器所编辑的项的索引
    QString                     m_displayFont;          //展示的内容的字体
    QFont                       m_titleFont;            //标题字体
    QColor                      m_titleColor;           //标题颜色
    QColor                      m_titleSelectedColor;   //被选中时标题的颜色
    QColor                      m_activeColor;          //活动状态的颜色
    QColor                      m_hoverColor;           //鼠标悬停时的颜色
    QColor                      m_folderIconColor;      //文件夹图标颜色
    QTreeView *                 m_view;                 //代指myTreeView
    QListView *                 m_listView;             //listView
    LabelEditType *             m_label;                //自定义label，能够在标签上编辑
    PushButtonType *            m_folderIcon;           //文件夹图标按钮，能够根据不同状态更换图标
    QPixmap                     m_expanded;             //展开时的图标
    QPixmap                     m_notExpanded;          //不展开时的图标
    QLabel *                    m_expandIcon;           //展示图标
    PushButtonType *            m_contextButton;        //内容按钮
    Theme::Value                m_theme;                //主题
    void                        updateDelegate();       //更新代理(样式表)

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
};

#endif // FOLDERTREEDELEGATEEDITOR_H
