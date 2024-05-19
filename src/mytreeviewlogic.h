#ifndef MYTREEVIEWLOGIC_H
#define MYTREEVIEWLOGIC_H

#include <QObject>
#include"mytreeview.h"
#include"mytreeviewmodel.h"
#include"mytreeviewdelegate.h"
#include"mylistview.h"
#include"dbmanager.h"
#include"customapplicationstyle.h"


//用于将树形结构的MVD逻辑结合为一个整体
class myTreeViewLogic : public QObject
{
    Q_OBJECT
public:
    explicit myTreeViewLogic(myTreeView *treeView, myTreeViewModel *treeModel, DBManager *dbManager,
                             myListView *listView, QObject *parent = nullptr);
    void openFolder(int id);
    void onMoveNodeRequested(int nodeId, int targetId);
    void setTheme(Theme::Value theme);
    void setLastSavedState(bool isLastSelectFolder, const QString &lastSelectFolder,
                           const QSet<int> &lastSelectTag, const QStringList &expandedFolder);
private slots:
    void updateTreeViewSeparator();
    void loadTreeModel(const NodeTagTreeData &treeData);
    void onAddTagRequested();
    void onRenameNodeRequestedFromTreeView(const QModelIndex &index, const QString &newName);
    void onDeleteFolderRequested(const QModelIndex &index);
    void onRenameTagRequestedFromTreeView(const QModelIndex &index, const QString &newName);
    void onChangeTagColorRequested(const QModelIndex &index);
    void onDeleteTagRequested(const QModelIndex &index);
    void onChildNotesCountChangedTag(int tagId, int notesCount);
    void onChildNoteCountChangedFolder(int folderId, const QString &absPath, int notesCount);

signals:
    void requestRenameNodeInDB(int id, const QString &newName);
    void requestRenameTagInDB(int id, const QString &newName);
    void requestChangeTagColorInDB(int id, const QString &newColor);
    void requestMoveNodeInDB(int id, const NodeData &target);
    void addNoteToTag(int noteId, int tagId);
    void noteMoved(int nodeId, int targetId);

private:
    void onAddFolderRequested(bool fromPlusButton);



private:
    myTreeView*             m_treeView;
    myTreeViewModel*        m_treeModel;
    myListView*             m_listView;
    myTreeViewDelegate*     m_treeDelegate;
    DBManager*              m_dbManager;
    CustomApplicationStyle* m_style;
    bool                    m_needLoadSavedState;
    bool                    m_isLastSelectFolder;
    QString                 m_lastSelectFolder;
    QSet<int>               m_lastSelectTags;
    QStringList             m_expandedFolder;
};

#endif // MYTREEVIEWLOGIC_H





