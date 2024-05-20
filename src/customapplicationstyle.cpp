#include "customapplicationstyle.h"

CustomApplicationStyle::CustomApplicationStyle()
{

}

void CustomApplicationStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{

}

void CustomApplicationStyle::setTheme(Theme::Value theme)
{
    m_theme=theme;
}
