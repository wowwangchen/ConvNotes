#ifndef MYTREEVIEWLOGIC_H
#define MYTREEVIEWLOGIC_H

#include <QObject>
#include<QMessageBox>
#include"mytreeview.h"
#include"mytreeviewmodel.h"
#include"mytreeviewdelegate.h"
#include"mylistview.h"
#include"dbmanager.h"
#include"customapplicationstyle.h"
#include "codetranslate.h"

//用于将树形结构的MVD和数据库进行操作
class myTreeViewLogic : public QObject
{
    Q_OBJECT
public:
    //构造函数，传入treeView，数据模型，数据库管理，listView，父类指针
    explicit myTreeViewLogic(myTreeView *treeView, myTreeViewModel *treeModel, DBManager *dbManager,
                             myListView *listView, QObject *parent = nullptr);
    //建立信号与槽
    void initConnect();
    //传入ID，打开文件夹
    void openFolder(int id);
    //响应节点位置移动
    void onMoveNodeRequested(int nodeId, int targetId);
    //设置主题
    void setTheme(Theme::Value theme);
    //设置上次的状态
    void setLastSavedState(bool isLastSelectFolder, const QString &lastSelectFolder,
                           const QSet<int> &lastSelectTag, const QStringList &expandedFolder);

private slots:
    //更新分隔符
    void updateTreeViewSeparator();
    //加载数据模型
    void loadTreeModel(const NodeTagTreeData &treeData);
    //响应重命名节点
    void onRenameNodeRequestedFromTreeView(const QModelIndex &index, const QString &newName);
    //响应删除文件夹
    void onDeleteFolderRequested(const QModelIndex &index);
    //响应文件夹的子节点数改变了
    void onChildNoteCountChangedFolder(int folderId, const QString &absPath, int notesCount);
    //响应添加文件夹
    void onAddFolderRequested(bool fromPlusButton);
    //void onAddTagRequested();
    //void onRenameTagRequestedFromTreeView(const QModelIndex &index, const QString &newName);
    //void onChangeTagColorRequested(const QModelIndex &index);
    //void onDeleteTagRequested(const QModelIndex &index);
    //void onChildNotesCountChangedTag(int tagId, int notesCount);

signals:
    //请求在数据库中重命名节点
    void requestRenameNodeInDB(int id, const QString &newName);
    //请求在数据库中移动节点位置
    void requestMoveNodeInDB(int id, const NodeData &target);
    //笔记移动
    void noteMoved(int nodeId, int targetId);
    //void requestRenameTagInDB(int id, const QString &newName);
    //void requestChangeTagColorInDB(int id, const QString &newColor);
    //void addNoteToTag(int noteId, int tagId);



private:
    myTreeView*             m_treeView;                 //文件夹树
    myTreeViewModel*        m_treeModel;                //树的数据模型
    myTreeViewDelegate*     m_treeDelegate;             //文件夹树的视图代理
    myListView*             m_listView;                 //笔记列表
    DBManager*              m_dbManager;                //数据库管理
    CustomApplicationStyle* m_style;                    //样式表风格
    bool                    m_needLoadSavedState;       //是否需要加载保存的状态
    bool                    m_isLastSelectFolder;       //上次是否是选择的文件夹
    QString                 m_lastSelectFolder;         //上次选择的文件夹名称
    //QSet<int>               m_lastSelectTags;
    QStringList             m_expandedFolder;           //展开的文件夹列表
};

#endif // MYTREEVIEWLOGIC_H





