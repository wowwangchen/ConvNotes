#ifndef MYLISTVIEW_H
#define MYLISTVIEW_H

#include<QListView>
#include<QObject>
#include<QScrollBar>
#include<QTimer>
#include<QMenu>
#include"dbmanager.h"
#include"notelistmodel.h"
#include"notelistdelegate.h"
#include"mytreeview.h"

class myListViewPrivate;


//自定义列表view
class myListView : public QListView
{
    Q_OBJECT
public:
    explicit myListView(QWidget* parent = nullptr);
    ~myListView();
    //设置不同主题的样式表
    void setupStyleSheet();
    //关闭所有编辑器的持久化
    void closeAllEditor();
    //添加行
    void animateAddedRow(const QModelIndexList &indexes);
    //设置数据库
    void setDbManager(DBManager *newDbManager);


    //设置当前行为活动状态
    void setCurrentRowActive(bool isActive);
    //设置主题，之前的setStyleSheet中有
    void setTheme(Theme::Value theme);
    //是否使用动画
    void setAnimationEnabled(bool isEnabled);
    //是否是否在垃圾桶中
    void setIsInTrash(bool newIsInTrash);
    //设置当前笔记所属文件夹
    void setCurrentFolderId(int newCurrentFolderId);
    //打开持久化编辑器
    void openPersistentEditorC(const QModelIndex &index);
    //关闭持久化编辑器
    void closePersistentEditorC(const QModelIndex &index);
    //设置编辑器的窗口
    void setEditorWidget(int noteId, QWidget *w);
    //重置编辑器的窗口
    void unsetEditorWidget(int noteId, QWidget *w);
    //设置列表相关信息
    void setListViewInfo(const ListViewInfo &newListViewInfo);
    //设置当前的索引
    void setCurrentIndexC(const QModelIndex &index);

    //返回是否在拖动
    bool isDragging() const;
    //置顶笔记集是否折叠
    bool isPinnedNotesCollapsed() const;
    //设置置顶的所有笔记是否折叠
    void setIsPinnedNotesCollapsed(bool newIsPinnedNotesCollapsed);
    //返回所有被选中的项
    QModelIndexList selectedIndex() const;
    //是否对指定区域进行拖拽
    bool isDraggingInsidePinned() const;
    //初始化信号与槽
    void setupSignalsSlots();



protected:
    //事件处理
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    bool viewportEvent(QEvent *e) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dragMoveEvent(QDragMoveEvent *event) override;

    // QAbstractScrollArea interface
protected:
    virtual void scrollContentsBy(int dx, int dy) override;
    virtual void startDrag(Qt::DropActions supportedActions) override;


public slots:
    //打开自定义菜单
    void onCustomContextMenu(QPoint point);
    //移除某些行(改变它们在delegate中的枚举状态)
    void onRemoveRowRequested(const QModelIndexList &indexes);
    //动画结束
    void onAnimationFinished(NoteListState state);
    //初始化
    void init();
    //视图中的选择发生改变
    void selectionChanged(const QItemSelection &selected,
                          const QItemSelection &deselected) override;


signals:
    void deleteNoteRequested(const QModelIndexList &index);
    void restoreNoteRequested(const QModelIndexList &indexes);
    void setPinnedNoteRequested(const QModelIndexList &indexes, bool isPinned);
    void newNoteRequested();
    void moveNoteRequested(int noteId, int folderId);
    void saveSelectedNote(const QSet<int> &noteId);
    void pinnedCollapseChanged();
    void notePressed(const QModelIndexList &selected);
    void noteListViewClicked();




private:
    bool                            m_animationEnabled;        //是否手使用动画
    bool                            m_isMousePressed;          //鼠标是否点击
    bool                            m_mousePressHandled;       //鼠标是否正在操纵
    int                             m_rowHeight;               //行的高度

    QMenu*                          contextMenu;               //编辑的菜单
    QAction*                        deleteNoteAction;          //删除笔记行为
    QAction*                        restoreNoteAction;         //恢复笔记行为
    QAction*                        pinNoteAction;             //置顶笔记行为
    QAction*                        unpinNoteAction;           //取消置顶笔记行为
    QAction*                        newNoteAction;             //新建笔记行为

    DBManager*                      m_dbManager;               //数据库操作

    int                             m_currentFolderId;         //当前笔记所属的文件夹的id
    QVector<QAction *>              m_folderActions;           //移动文件夹的菜单中包含的文件夹列表
    bool                            m_isInTrash;               //是否被放入垃圾桶
    QPoint                          m_dragStartPosition;       //拖动的起始位置
    QPixmap                         m_dragPixmap;              //拖动的像素
    QMap<int, QVector<QWidget *>>   m_openedEditor;            //已经打开的编辑器
    QVector<int>                    m_needRemovedNotes;        //所有需要移出的笔记
    ListViewInfo                    m_listViewInfo;            //列表的信息
    bool                            m_isDragging;              //是否正在拖动
    bool                            m_isDraggingPinnedNotes;   //拖动的是否是指定的笔记
    bool                            m_isPinnedNotesCollapsed;  //指定的笔记是否折叠
    bool                            m_isDraggingInsidePinned;  //拖动的位置是否在置顶区内

private:

    //QT私有实现模式，公有接口和私有实现细节分离
    Q_DECLARE_PRIVATE(myListView)  //声明一个私有类型指针

};


class myListViewPrivae : public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(myTreeView)

public:
    myListViewPrivae() : QAbstractItemViewPrivate(){};
    virtual ~myListViewPrivae() { }
};


#endif // MYLISTVIEW_H
