#ifndef NOTELISTDELEGATE_H
#define NOTELISTDELEGATE_H

#include <QStyledItemDelegate>
#include <QObject>
#include<QTimeLine>
#include<QQueue>
#include<QPair>
#include"mylistview.h"


//列表中的索引的状态
enum class NoteListState { Normal, Insert, Remove, MoveOut, MoveIn };

//列表类的代理
class NoteListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit NoteListDelegate(myListView *view,QObject *parent = nullptr);
    void setState(NoteListState NewState, QModelIndexList indexes);
    void setActive(bool isActive);
     void setHoveredIndex(const QModelIndex &hoveredIndex);
     void setRowRightOffset(int rowRightOffset);

 signals:
     void themeChanged(Theme::Value theme);
     void animationFinished(NoteListState animationState);


 private:
     myListView*                                m_view;                     //列表视图
     QString                                    m_displayFont;              //展示字体
     QFont                                      m_titleFont;                //标题字体
     QFont                                      m_titleSelectedFont;        //标题被选中时的字体
     QFont                                      m_dateFont;                 //日期字体
     QFont                                      m_headerFont;               //抬头字体
     QColor                                     m_titleColor;               //主题颜色
     QColor                                     m_dateColor;                //日期颜色
     QColor                                     m_contentColor;             //内容颜色
     QColor                                     m_ActiveColor;              //活动时的颜色
     QColor                                     m_notActiveColor;           //非活动时的颜色
     QColor                                     m_hoverColor;               //鼠标悬浮时的颜色
     QColor                                     m_applicationInactiveColor; //应用非活跃时的颜色
     QColor                                     m_separatorColor;           //分割线的颜色
     QColor                                     m_defaultColor;             //默认颜色

     int                                        m_rowHeight;                //行的高度
     int                                        m_maxFrame;                 //最大框架大小
     int                                        m_rowRightOffset;           //行的右边偏移，用于拖动条
     NoteListState                              m_state;                    //当前状态，插入、删除等
     bool                                       m_isActive;                 //是否处于活动状态
     bool                                       m_isInAllNotes;             //是否是在allnotes文件夹中
     QImage                                     m_folderIcon;               //文件夹图标
     Theme::Value                               m_theme;                    //主题
     QTimeLine*                                 m_timeLine;                 //动画效果的时间线
     QModelIndexList                            m_animatedIndexes;          //索引列表
     QModelIndex                                m_hoveredIndex;             //当前悬停的索引
     QMap<int, QSize>                           szMap;                      //节点对应的大小键值对
     QQueue<QPair<QSet<int>, NoteListState>>    animationQueue;             //id集合、状态键值对形成的队列
};

#endif // NOTELISTDELEGATE_H
