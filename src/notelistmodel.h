#ifndef NOTELISTMODEL_H
#define NOTELISTMODEL_H

#include <QAbstractItemModel>
#include <QObject>

//自定义列表的数据模型
class NoteListModel : public QAbstractItemModel
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
    QModelIndex getNoteIndex(int id);
    bool isFirstPinnedNote(const QModelIndex &index) const;
     bool hasPinnedNote() const;
    bool isFirstUnpinnedNote(const QModelIndex &index) const;
};

#endif // NOTELISTMODEL_H
