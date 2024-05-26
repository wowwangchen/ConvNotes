#ifndef NOTELISTDELEGATE_H
#define NOTELISTDELEGATE_H

#include <QStyledItemDelegate>
#include <QObject>

//列表中的索引的状态
enum class NoteListState { Normal, Insert, Remove, MoveOut, MoveIn };

//列表类的代理
class NoteListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit NoteListDelegate(QObject *parent = nullptr);
    void setState(NoteListState state,QModelIndexList indexs);
    void setActive(bool isActive);
     void setHoveredIndex(const QModelIndex &hoveredIndex);
     void setRowRightOffset(int rowRightOffset);
};

#endif // NOTELISTDELEGATE_H
