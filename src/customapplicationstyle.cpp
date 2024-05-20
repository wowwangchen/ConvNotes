#include "customapplicationstyle.h"

CustomApplicationStyle::CustomApplicationStyle()
{

}

void CustomApplicationStyle::drawPrimitive(PrimitiveElement element,
                        const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == QStyle::PE_IndicatorItemViewItemDrop)  //拖放项的指示器
    {
        painter->setRenderHint(QPainter::Antialiasing, true); //抗锯齿

        QColor color(207, 207, 207);
        QPen pen(color);
        pen.setWidth(2);
        painter->setPen(pen);

        //高度为0，画一条线
        if (option->rect.height() == 0)
        {
            color.setAlpha(50);
            painter->setBrush(color);
            painter->drawLine(option->rect.topLeft(), option->rect.topRight());
        }
        //否则用带有透明度的矩阵填充
        else
        {
            color.setAlpha(200);
            painter->fillRect(option->rect, color);
        }
    }
    //不是的话就调用基类的方法
    else
    {
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
}

void CustomApplicationStyle::setTheme(Theme::Value theme)
{
    m_theme=theme;
}
