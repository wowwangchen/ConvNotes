#ifndef NOTEEDITORLOGIC_H
#define NOTEEDITORLOGIC_H

#include <QObject>
#include<QLabel>
#include<QLineEdit>
#include<QTimer>
#include<QScrollBar>
#include"mytreeview.h"
#include"dbmanager.h"
#include"customdocument.h"
#include"custommarkdownhighlighter.h"


//笔记编写的操控类
class NoteEditorLogic : public QObject
{
    Q_OBJECT
public:
    explicit NoteEditorLogic(CustomDocument *textEdit, QLabel *editorDateLabel,
                             QLineEdit *searchEdit, DBManager *dbManager, QObject *parent = nullptr);
    static QString getFirstLine(const QString &str);
    static QString getSecondLine(const QString &str);
    bool markdownEnabled() const;
    void setMarkdownEnabled(bool enabled);
    static QString getNoteDateEditor(const QString &dateEdited);
    void highlightSearch() const;
    bool isTempNote() const;
    void saveNoteToDB();
    int currentEditingNoteId() const;
    void deleteCurrentNote();


    void setTheme(Theme::Value theme, QColor textColor, qreal fontSize);

    int currentAdaptableEditorPadding() const;
    void setCurrentAdaptableEditorPadding(int newCurrentAdaptableEditorPadding);

    int currentMinimumEditorPadding() const;
    void setCurrentMinimumEditorPadding(int newCurrentMinimumEditorPadding);

public slots:
    void showNotesInEditor(const QVector<NodeData> &notes);
    void onTextEditTextChanged();
    void closeEditor();



private:
    static QDateTime getQDateTime(const QString &date);
    bool isInEditMode() const;
    QString moveTextToNewLinePosition(const QString &inputText, int startLinePosition,
                                      int endLinePosition, int newLinePosition,
                                      bool isColumns = false);
    QMap<QString, int> getTaskDataInLine(const QString &line);
    void replaceTextBetweenLines(int startLinePosition, int endLinePosition, QString &newText);
    void removeTextBetweenLines(int startLinePosition, int endLinePosition);
    void appendNewColumn(QJsonArray &data, QJsonObject &currentColumn, QString &currentTitle,
                         QJsonArray &tasks);
    void addUntitledColumnToTextEditor(int startLinePosition);


signals:
    void requestCreateUpdateNote(const NodeData &note);
    void noteEditClosed(const NodeData &note, bool selectNext);
    void setVisibilityOfFrameRightWidgets(bool);
    void setVisibilityOfFrameRightNonEditor(bool);
    void moveNoteToListViewTop(const NodeData &note);
    void updateNoteDataInList(const NodeData &note);
    void deleteNoteRequested(const NodeData &note);
    void showKanbanView();
    void hideKanbanView();
    void textShown();
    void kanbanShown();
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    void tasksFoundInEditor(QVariant data);
    void clearKanbanModel();
    void resetKanbanSettings();
    void checkMultipleNotesSelected(QVariant isMultipleNotesSelected);
#endif


private:
    CustomDocument*             m_textEdit;
    CustomMarkdownHighlighter*  m_highlighter;
    QLabel*                     m_editorDateLabel;
    QLineEdit*                  m_searchEdit;

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    QWidget *m_kanbanWidget;
#endif

    DBManager*                  m_dbManager;
    QVector<NodeData>           m_currentNotes;
    bool                        m_isContentModified;
    QTimer                      m_autoSaveTimer;
    QColor                      m_spacerColor;
    int                         m_currentAdaptableEditorPadding;
    int                         m_currentMinimumEditorPadding;

};

#endif // NOTEEDITORLOGIC_H
