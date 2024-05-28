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
    //构造函数，初始化各种参数
    explicit NoteListDelegate(myListView *view,QObject *parent = nullptr);
    //设置传入的索引列表新的状态
    void setState(NoteListState NewState, QModelIndexList indexes);
    //设置动画
    void setStateI(NoteListState NewState, const QModelIndexList &indexes);
    //获取私有成员m_timeline的状态
    QTimeLine::State animationState();
    //设置动画持续时间
    void setAnimationDuration(const int duration);



    //绘制
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    //设置大小，主要是调整高度
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    //设置代理编辑器
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const override;


    //缓存的大小，少算了一些，提高性能
    QSize bufferSizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        //获取当前鼠标悬停的索引
    const QModelIndex &hoveredIndex() const;
    //设置当前鼠标悬停的索引
    void setHoveredIndex(const QModelIndex &hoveredIndex);
    //设置右边偏移量，用于拖动条
    void setRowRightOffset(int rowRightOffset);
    //设置活动状态
    void setActive(bool isActive);
    //设置主题
    void setTheme(Theme::Value theme);
    //获取主题
    Theme::Value theme() const;
    //设置是否在allnotes文件夹内
    void setIsInAllNotes(bool newIsInAllNotes);
    //返回是否在allnotes内
    bool isInAllNotes() const;
    //清除节点对应大小的键值对
    void clearSizeMap();
    //判断是否需要绘制分隔符
    bool shouldPaintSeparator(const QModelIndex &index, const NoteListModel &model) const;


private:
    //绘制背景，矩形大小
    void paintBackground(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const;
    //
    void paintLabels(QPainter *painter, const QStyleOptionViewItem &option,
                     const QModelIndex &index) const;
    //绘制分割线
    void paintSeparator(QPainter *painter, QRect rect, const QModelIndex &index) const;
    QString parseDateTime(const QDateTime &dateTime) const;



public slots:
    //更新某个节点的大小
    void updateSizeMap(int id, QSize sz, const QModelIndex &index);
    //移除编辑器
    void editorDestroyed(int id, const QModelIndex &index);



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
