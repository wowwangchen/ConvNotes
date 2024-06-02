#ifndef CUSTOMMARKDOWNHIGHLIGHTER_H
#define CUSTOMMARKDOWNHIGHLIGHTER_H

#include <QObject>
#include<QTextDocument>
#include"mytreeview.h"
#include"markdownhighlighter.h"

class CustomMarkdownHighlighter : public MarkdownHighlighter
{
    Q_OBJECT
public:
    CustomMarkdownHighlighter(QTextDocument *parent = nullptr,
                              HighlightingOptions highlightingOptions = HighlightingOption::None);

    void setFontSize(qreal fontSize);
    void setHeaderColors(QColor color);
    void setListsColor(QColor color);

    void setTheme(Theme::Value theme, QColor textColor, qreal fontSize);
};

#endif // CUSTOMMARKDOWNHIGHLIGHTER_H
