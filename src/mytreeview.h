
#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include <QObject>
#include<QTreeView>
#include<QWidget>
#include<QModelIndex>
#include<QDebug>
#include<QDragEnterEvent>
#include<QDropEvent>
#include<QMimeData>
#include<QMenu>
#include<QtWidgets/5.12.9/QtWidgets/private/qabstractitemview_p.h>
#include"mytreeviewmodel.h"
#include"nodepath.h"


namespace Theme
{
enum Value{Light=1,Dark,Sepia};
};


class myTreeViewPrivate;




//此类为属性结构的框架，对界面、数据、操作进行逻辑统一
class myTreeView:public QTreeView
{

    Q_OBJECT
    Q_DECLARE_PRIVATE(myTreeView)

public:
    myTreeView(QWidget *parent = nullptr);
    //设置树形结构的分隔界限
    void setTreeSeperator(const QVector<QModelIndex> &newSeperator,const QModelIndex &defaultNotesIndex);
    //当前是否正在编辑
    void setIsEditing(bool newIsEditing);
    //返回当前正在编辑的索引
    const QModelIndex &currentEditingIndex() const;
    //重命名文件夹(当前索引)
    void onRenameFolderFinished(const QString &newName);
    //设置当前索引
    void setCurrentIndexC(const QModelIndex &index);
    //设置主题风格
    void setTheme(Theme::Value theme);
    //返回当前的主题枚举类型
    Theme::Value theme() const;
    //根据枚举类型具体设置
    void updateTheme(Theme::Value theme);
    //重新展开树形结构
    void reExpand();
    //关闭当前正在编辑的项
    void closeCurrentEditor();
    //更新正在编辑的节点
    void updateEditingIndex(QPoint pos);
    //设置忽略这次(下次)的操作
    void setIgnoreThisCurrentLoad(bool newIgnoreThisCurrentLoad);
    //返回是否处于拖动状态
    bool isDragging() const;



signals:
    //从数据库中重命名文件夹的信号
    void renameFolderNameInDatabase(const QModelIndex &index, const QString &newName);
    //请求移动节点位置的信号
    void requestMoveNode(int node, int target);
    //请求加载上次选择的笔记
    void requestLoadLastSelectedNote();
    //请求在文件夹中加载笔记
    void loadNotesInFolderRequested(int folderID, bool isRecursive, bool notInterested = false,
                                    int scrollToId = SpecialNodeID::InvalidNodeId);
    //保存选中的节点
    void saveSelected(bool isSelectingFolder, const QString &folder, const QSet<int> &tags);
    //请求删除某个节点
    void deleteNodeRequested(const QModelIndex &index);
    //请求保存展开的列表
    void saveExpand(const QStringList &ex);
    //请求修改文件夹颜色
    void changeFolderColorRequested(const QModelIndex &index);
    //请求重命名文件夹
    void renameFolderRequested();


public slots:
    //自定义菜单
    void onCustomContextMenu(QPoint point);
    //响应请求改变文件夹标签颜色
    void onChangeFolderColorAction();
    //响应请求展开的信号
    void onRequestExpand(const QString &folderPath);
    //更新绝对路径
    void onUpdateAbsPath(const QString &oldPath, const QString &newPath);
    //响应文件夹丢弃成功信号
    void onFolderDropSuccessful(const QString &path);
    //重置界面，刷新界面
    virtual void reset() override;
    //管理选择的状态
    virtual void selectionChanged(const QItemSelection &selected,
                                  const QItemSelection &deselected) override;
    //当前选中的项发生了改变
    virtual void currentChanged(const QModelIndex &current,
                                const QModelIndex &previous) override;
    //删除节点的行为
    void onDeleteNodeAction();
    //展开某个项
    void onExpanded(const QModelIndex &index);
    //折叠某个项
    void onCollapsed(const QModelIndex &index);



private:
    QModelIndex             m_defaultNotesIndex;       //默认的笔记索引
    bool                    m_isEditing;               //当前是否正在编辑
    QModelIndex             m_currentEditingIndex;     //当前正在编辑的索引
    Theme::Value            m_theme;                   //当前的主题
    QVector<QModelIndex>    m_treeSeparator;           //树形结构上下的范围的分隔符
    QVector<QString>        m_expanded;                //展开的文件的地址
    bool                    m_isContextMenuOpened;     //菜单选项是否打开
    QModelIndex             m_needReleaseIndex;        //当前需要释放(取消选择)的节点
    bool                    m_isLastSelectedFolder;    //上次选择的是否是文件夹节点
    QString                 m_lastSelectFolder;        //上次打开的文件夹路径
    bool                    m_ignoreThisCurrentLoad;   //是否忽略这次操作
    QMenu*                  contextMenu;               //自定义菜单
    QAction*                renameFolderAction;        //自定义菜单中的内容，重命名文件夹
    QAction*                deleteFolderAction;        //删除文件夹
    QAction*                addSubfolderAction;        //添加文件夹分支
    QAction*                changeFolderColorAction;   //改变文件夹标签的颜色
    //QAction*                clearSelectionAction;      //清空选择的项




protected:
    //拖入事件处理函数，判断是否接受这个操作
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    //放置事件处理函数，处理动作，用于修改节点的父子关系
    virtual void dropEvent(QDropEvent *event) override;
    //拖动移动事件处理函数，拖动过程中的事件处理
    virtual void dragMoveEvent(QDragMoveEvent *event) override;
    //判断是否开始拖动
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    //设置是否折叠或展开
    virtual void mousePressEvent(QMouseEvent *event) override;
    //释放需要取消选择的节点
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    //取消需要释放的节点
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    //判断是否需要关闭持久化编辑器
    virtual void leaveEvent(QEvent *event) override;
};

//私有类，对myTreeView生成一个只能访问私有成员的私有类

class myTreeViewPrivate : public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(myTreeView)

public:
    myTreeViewPrivate() : QAbstractItemViewPrivate(){};
    virtual ~myTreeViewPrivate() { }
};


#endif // MYTREEVIEW_H
