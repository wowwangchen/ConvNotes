#include "custommarkdownhighlighter.h"



CustomMarkdownHighlighter::CustomMarkdownHighlighter(QTextDocument *parent, HighlightingOptions highlightingOptions)
    : MarkdownHighlighter(parent, highlightingOptions)
{
    setListsColor(QColor(35, 131, 226)); // accent color

    _formats[static_cast<HighlighterState>(HighlighterState::HorizontalRuler)].clearBackground();
}

void CustomMarkdownHighlighter::setFontSize(qreal fontSize)
{

}

void CustomMarkdownHighlighter::setHeaderColors(QColor color)
{

}

void CustomMarkdownHighlighter::setListsColor(QColor color)
{

}

void CustomMarkdownHighlighter::setTheme(Theme::Value theme, QColor textColor, qreal fontSize)
{

}
