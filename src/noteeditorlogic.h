#ifndef NOTEEDITORLOGIC_H
#define NOTEEDITORLOGIC_H

#include <QObject>


//编辑笔记的
class NoteEditorLogic : public QObject
{
    Q_OBJECT
public:
    explicit NoteEditorLogic(QObject *parent = nullptr);

signals:

};

#endif // NOTEEDITORLOGIC_H
