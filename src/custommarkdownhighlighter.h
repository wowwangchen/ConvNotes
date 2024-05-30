#ifndef CUSTOMMARKDOWNHIGHLIGHTER_H
#define CUSTOMMARKDOWNHIGHLIGHTER_H

#include <QObject>
#include<QTextDocument>
#include"mytreeview.h"
class CustomMarkdownHighlighter : public QObject
{
    Q_OBJECT
public:
    CustomMarkdownHighlighter(QTextDocument *parent = nullptr);

    void setFontSize(qreal fontSize);
    void setHeaderColors(QColor color);
    void setListsColor(QColor color);

    void setTheme(Theme::Value theme, QColor textColor, qreal fontSize);
};

#endif // CUSTOMMARKDOWNHIGHLIGHTER_H
