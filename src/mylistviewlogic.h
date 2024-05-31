#ifndef MYLISTVIEWLOGIC_H
#define MYLISTVIEWLOGIC_H

#include <QObject>
#include <QMessageBox>
#include<QLineEdit>
#include<QToolButton>
#include"nodedata.h"
#include"dbmanager.h"
#include"notelistmodel.h"
#include"notelistdelegate.h"
#include"mylistview.h"



//联合文件列表的MVD与数据库
class myListViewLogic : public QObject
{
    Q_OBJECT

public:
    explicit myListViewLogic(myListView *noteView, NoteListModel *noteModel, QLineEdit *searchEdit,
                           QToolButton *clearButton, DBManager *dbManager,
                           QObject *parent = nullptr);
    //选中某个笔记
    void selectNote(const QModelIndex &noteIndex);
    //获取笔记信息结构体
    const ListViewInfo &listViewInfo() const;
    //选择第一个笔记
    void selectFirstNote();
    //设置主题
    void setTheme(Theme::Value theme);
    //是否正在运行动画
    bool isAnimationRunning();
    //设置上次保存的状态是什么
    void setLastSavedState(const QSet<int> &lastSelectedNotes, int needLoadSavedState = 2);
    //请求加载保存的状态
    void requestLoadSavedState(int needLoadSavedState);
    //选中所有的笔记
    void selectAllNotes();



private slots:
    //加载笔记数据
    void loadNoteListModel(const QVector<NodeData> &noteList, const ListViewInfo &inf);
    //点击某个笔记后
    void onNotePressed(const QModelIndexList &indexes);
    //删除某些笔记
    void deleteNoteRequestedI(const QModelIndexList &indexes);
    //恢复某些笔记，添加到默认文件夹中
    void restoreNotesRequestedI(const QModelIndexList &indexes);
    //更新列表
    void updateListViewLabel();
    //行的数量改变了
    void onRowCountChanged();
    //笔记双击
    void onNoteDoubleClicked(const QModelIndex &index);
    //设置某些笔记为置顶的
    void onSetPinnedNoteRequested(const QModelIndexList &indexes, bool isPinned);
    //点击了这个列表
    void onListViewClicked();



public slots:
    //将某个笔记置顶
    void moveNoteToTop(const NodeData &note);
    //设置某个笔记的参数
    void setNoteData(const NodeData &note);
    //笔记编辑关闭
    void onNoteEditClosed(const NodeData &note, bool selectNext);
    //删除某个笔记请求
    void deleteNoteRequested(const NodeData &note);
    //向上选择笔记
    void selectNoteUp();
    //向下选择笔记
    void selectNoteDown();
    //搜索的内容改变了
    void onSearchEditTextChanged(const QString &keyword);
    //清除搜索
    void clearSearch(bool createNewNote = false, int scrollToId = SpecialNodeID::InvalidNodeId);
    //笔记移出
    void onNoteMovedOut(int nodeId, int targetId);
    //设置上次选择的笔记
    void setLastSelectedNote();
    //加载上次选择的笔记
    void loadLastSelectedNoteRequested();
    //从文件中加载笔记
    void onNotesListInFolderRequested(int parentID, bool isRecursive, bool newNote, int scrollToId);
    //选择某些笔记
    void selectNotes(const QModelIndexList &indexes);



signals:
    //展示笔记
    void showNotesInEditor(const QVector<NodeData> &notesData);
    //在数据库中移除笔记
    void requestRemoveNoteDb(const NodeData &noteData);
    //在数据库中移动笔记
    void requestMoveNoteDb(int noteId, const NodeData &targetFolder);
    //请求高亮搜索
    void requestHighlightSearch();
    //关闭持久化笔记编辑器
    void closeNoteEditor();
    //请求在数据库中寻找关键词
    void requestSearchInDb(const QString &keyword, const ListViewInfo &inf);
    //在数据库中清除搜索
    void requestClearSearchDb(const ListViewInfo &inf);
    //清除界面的搜索
    void requestClearSearchUI();
    //请求新建笔记
    void requestNewNote();
    //请求移动笔记
    void moveNoteRequested(int id, int target);
    //列表的内容改变了
    void listViewLabelChanged(const QString &label1, const QString &label2);
    //设置新建笔记按钮可视化
    void setNewNoteButtonVisible(bool visible);
    //请求在文夹中获取笔记里列表
    void requestNotesListInFolder(int parentID, bool isRecursive, bool newNote, int scrollToId);




private:
    myListView*             m_listView;                     //代指笔记列表
    NoteListModel*          m_listModel;                    //代指笔记列表的模型
    NoteListDelegate*       m_listDelegate;                 //代指笔记列表的视图代理
    QLineEdit*              m_searchEdit;                   //搜索框
    QToolButton*            m_clearButton;                  //清空按钮
    DBManager*              m_dbManager;                    //代指数据库
    ListViewInfo            m_listViewInfo;                 //笔记列表的相关信息
    QVector<QModelIndex>    m_editorIndexes;                //开了编辑器的索引

    int                     m_needLoadSavedState;           //需要加载保存的状态
    QSet<int>               m_lastSelectedNotes;            //上次选择的笔记集合

};

#endif // MYLISTVIEWLOGIC_H
