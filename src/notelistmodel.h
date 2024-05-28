#ifndef NOTELISTMODEL_H
#define NOTELISTMODEL_H


#include <QAbstractListModel>
#include <QObject>
#include<QMimeData>
#include"nodedata.h"
#include"dbmanager.h"



//自定义列表的数据模型
class NoteListModel : public QAbstractListModel
{
    Q_OBJECT
public:

    //笔记的各种信息
    enum NoteRoles
    {
        NoteID = Qt::UserRole + 1,
        NoteFullTitle,
        NoteCreationDateTime,
        NoteLastModificationDateTime,
        NoteDeletionDateTime,
        NoteContent,
        NoteScrollbarPos,
        NoteTagsList,
        NoteIsTemp,
        NoteParentName,
        NoteTagListScrollbarPos,
        NoteIsPinned,
    };


    explicit NoteListModel(QObject *parent = nullptr);
    ~NoteListModel();

    //添加
    QModelIndex addNote(const NodeData &note);
    //插入
    QModelIndex insertNote(const NodeData &note, int row);
    //移除
    void removeNotes(const QModelIndexList &noteIndexes);
    //移除多行
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    //移动
    bool moveRow(const QModelIndex &sourceParent, int sourceRow,
                 const QModelIndex &destinationParent, int destinationChild);
    //清除
    void clearNotes();
    //设置笔记数据
    void setNoteData(const QModelIndex &index, const NodeData &note);


    //获取索引对应的笔记的信息
    const NodeData &getNote(const QModelIndex &index) const;
    //通过id获取笔记索引
    QModelIndex getNoteIndex(int id) const;


    //初始化笔记列表,从info中的parent中获取部分
    void setListNote(const QVector<NodeData> &notes, const ListViewInfo &inf);
    //获取相关数据
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    //设置相关数据
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    //返回index对应的项的标志位(搜索帮助文档Qt::ItemFlags)
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    //获取行数
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    //排序
    void sort(int column, Qt::SortOrder order) override;


    //模型的拖放操作
    virtual Qt::DropActions supportedDropActions() const override;
    virtual Qt::DropActions supportedDragActions() const override;
    //模型支持的mime类型列表
    virtual QStringList mimeTypes() const override;
    //返回指定索引的mime数据
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
    //处理拖动而放下的mime数据
    virtual bool dropMimeData(const QMimeData *mime, Qt::DropAction action, int row, int column,
                              const QModelIndex &parent) override;


    //是否是第一个置顶笔记
    bool isFirstPinnedNote(const QModelIndex &index) const;
    //是否是第一个非置顶的笔记
    bool isFirstUnpinnedNote(const QModelIndex &index) const;
    //获取第一个置顶笔记的节点
    QModelIndex getFirstPinnedNote() const;
    //获取第一个非置顶的笔记的节点
    QModelIndex getFirstUnpinnedNote() const;
    //是否有置顶的笔记
    bool hasPinnedNote() const;

    //设置某些笔记是否置顶，同时会进行插入操作
    void setNotesIsPinned(const QModelIndexList &indexes, bool isPinned);


private:
    //更新指定笔记的相对位置
    void updatePinnedRelativePosition();
    //判断是否属于allnote这个文件夹
    bool isInAllNote() const;
    //获取某行的节点信息
    NodeData &getRef(int row);
    const NodeData &getRef(int row) const;


private:
    //所有笔记集合
    QVector<NodeData> m_noteList;
    //所有指定笔记集合
    QVector<NodeData> m_pinnedList;
    //整个列表相关信息
    ListViewInfo m_listViewInfo;



signals:
    //行数改变
    void rowCountChanged();
    //更新置顶的内容
    void requestUpdatePinned(int noteId, bool isPinned);
    //更新置顶笔记的相对位置
    void requestUpdatePinnedRelPos(int noteId, int pos);
    //更新置顶笔记的相对位置
    void requestUpdatePinnedRelPosAN(int noteId, int pos);
    //移除某些笔记
    void requestRemoveNotes(QModelIndexList index);
    //插入多行
    void rowsInsertedC(const QModelIndexList &rows);
    //多行即将被移动
    void rowsAboutToBeMovedC(const QModelIndexList &source);
    //移动多行
    void rowsMovedC(const QModelIndexList &dest);
    //请求关闭某些编辑器
    void requestCloseNoteEditor(const QModelIndexList &indexes);
    //请求打开某些编辑器
    void requestOpenNoteEditor(const QModelIndexList &indexes);
    //选择某些笔记
    void selectNotes(const QModelIndexList &indexes);


};

#endif // NOTELISTMODEL_H
