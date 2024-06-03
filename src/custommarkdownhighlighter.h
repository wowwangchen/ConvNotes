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
    //字体大小
    void setFontSize(qreal fontSize);
    //标题颜色
    void setHeaderColors(QColor color);
    //列表颜色
    void setListsColor(QColor color);

    void setTheme(Theme::Value theme, QColor textColor, qreal fontSize);
};

#endif // CUSTOMMARKDOWNHIGHLIGHTER_H
