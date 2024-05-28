#ifndef NOTELISTDELEGATEEDITOR_H
#define NOTELISTDELEGATEEDITOR_H

#include <QObject>
#include<QWidget>
#include"notelistdelegate.h"
#include"mylistview.h"
//列表的不同内容的间隔
struct NoteListConstant
{
    static constexpr int leftOffsetX = 20;
    static constexpr int topOffsetY = 10; // space on top of title
    static constexpr int titleDateSpace = 2; // space between title and date
    static constexpr int dateDescSpace = 5; // space between date and description
    static constexpr int descFolderSpace = 14; // space between description and folder name
    static constexpr int lastElSepSpace = 12; // space between the last element and the separator
    static constexpr int nextNoteOffset =
        0; // space between the separator and the next note underneath it
    static constexpr int pinnedHeaderToNoteSpace =
        0; // space between Pinned label to the pinned list
    static constexpr int unpinnedHeaderToNoteSpace =
        0; // space between Notes label and the normal notes list
    static constexpr int lastPinnedToUnpinnedHeader =
        10; // space between the last pinned note to Notes label
};


class NoteListDelegateEditor : public QWidget
{
    Q_OBJECT
public:
    explicit NoteListDelegateEditor(const NoteListDelegate *delegate, myListView *view,
                                    const QStyleOptionViewItem &option, const QModelIndex &index,
                                    QWidget *parent = nullptr);
public slots:

    void setTheme(Theme::Value theme);
    void recalculateSize();

signals:
    void updateSizeHint(int id, const QSize &sz, const QModelIndex &index);
    void nearDestroyed(int id, const QModelIndex &index);
};

#endif // NOTELISTDELEGATEEDITOR_H
