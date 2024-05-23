#ifndef MYTREEVIEWDELEGATE_H
#define MYTREEVIEWDELEGATE_H

#include<QObject>
#include<QTreeView>
#include<QListView>
#include<QStyledItemDelegate>
#include<QString>
#include<QFont>
#include<QStringLiteral>
#include<QPainter>
#include<QHBoxLayout>
#include<QLabel>
#include<QPushButton>
#include"mytreeview.h"
#include "codetranslate.h"

//L_DECLARE_ENUM(Theme, Light, Dark, Sepia)


//树形结构中不同的项的高度
struct NoteTreeConstant
{
    static constexpr int folderItemHeight = 45;
    static constexpr int tagItemHeight = 45;
    static constexpr int folderLabelHeight = 50;
    static constexpr int tagLabelHeight = 50;  //+20 -5
};



//树形结构的代理，进行自定义的界面绘制
class myTreeViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    //初始化成员列表 //
    explicit myTreeViewDelegate(QTreeView *view, QObject *parent = nullptr,
                              QListView *listView = nullptr);
    //设置主题，改变变量的值
    void setTheme(Theme::Value theme);
    //绘制"被选择的项"的背景
    void paintBackgroundSelectable(QPainter *painter, const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const;


signals:
    void themeChanged(Theme::Value theme);
    void addFolderRequested();

public:
    //绘制
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;
    //调整项的大小
    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const override;
    //设置编辑项的控件
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const override;
    //更新编辑器控件的位置、大小
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const override;


private:
    QString         m_displayFont;                  //展示的内容的字体
    QFont           m_titleFont;                    //标题的字体
    QFont           m_titleSelectedFont;            //标题被选中时的字体
    QFont           m_dateFont;                     //日期字体
    QFont           m_separatorFont;                //分割线的字体
    QFont           m_numberOfNotesFont;            //笔记序号的字体

    QColor          m_titleColor;                   //标题颜色
    QColor          m_titleSelectedColor;           //标题被选中时的颜色
    QColor          m_dateColor;                    //日期颜色
    QColor          m_ActiveColor;                  //活动状态的颜色
    QColor          m_notActiveColor;               //非活动状态的颜色
    QColor          m_hoverColor;                   //悬停时的颜色
    QColor          m_applicationInactiveColor;     //应用非活动状态的颜色
    QColor          m_separatorColor;               //分割线的颜色
    QColor          m_defaultColor;                 //默认颜色
    QColor          m_separatorTextColor;           //分割线的文本的显色
    QColor          m_currentBackgroundColor;       //当前背景的颜色
    QColor          m_numberOfNotesColor;           //笔记数量的颜色
    QColor          m_numberOfNotesSelectedColor;   //笔记数量被选中时的颜色
    QColor          m_folderIconColor;              //文件夹图标的颜色
    QTreeView*      m_view;                         //treeView
    QListView*      m_listView;                     //listView
    Theme::Value    m_theme;                        //主题风格
};

#endif // MYTREEVIEWDELEGATE_H
