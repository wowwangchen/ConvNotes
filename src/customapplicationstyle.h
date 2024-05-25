#ifndef CUSTOMAPPLICATIONSTYLE_H
#define CUSTOMAPPLICATIONSTYLE_H

#include <QProxyStyle>
#include"mytreeview.h"
#include<QStyleOption>


//自定义界面风格代理
class CustomApplicationStyle : public QProxyStyle
{
public:
    CustomApplicationStyle();

    //重写和定制各种绘制元素的方法
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                       const QWidget *widget) const;
    void setTheme(Theme::Value theme);

private:
    Theme::Value m_theme;
};

#endif // CUSTOMAPPLICATIONSTYLE_H
