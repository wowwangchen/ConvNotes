#include "noteeditorlogic.h"

NoteEditorLogic::NoteEditorLogic(QObject *parent)
    : QObject{parent}
{

}

QString NoteEditorLogic::getFirstLine(const QString &str)
{
    return QString{};
}

QString NoteEditorLogic::getSecondLine(const QString &str)
{
    return QString{};
}
