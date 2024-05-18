#include "pushbuttontype.h"



PushButtonType::PushButtonType(QWidget *parent) :QPushButton(parent)
{
}

void PushButtonType::setNormalIcon(const QIcon &newNormalIcon)
{
    normalIcon = newNormalIcon;
    setIcon(newNormalIcon);
}

void PushButtonType::setHoveredIcon(const QIcon &newHoveredIcon)
{
    hoveredIcon = newHoveredIcon;
}

void PushButtonType::setPressedIcon(const QIcon &newPressedIcon)
{
    pressedIcon = newPressedIcon;
}

bool PushButtonType::event(QEvent *event)
{

    //鼠标点击事件
    if (event->type() == QEvent::MouseButtonPress)
    {
        setIcon(pressedIcon);
    }
    //鼠标松开事件
    if (event->type() == QEvent::MouseButtonRelease)
    {
        if (underMouse()) //窗口还在鼠标下
        {
            setIcon(hoveredIcon);
        }
        else
        {
            setIcon(normalIcon);
        }
    }
    //鼠标进入事件
    if (event->type() == QEvent::Enter)
    {
        setIcon(hoveredIcon);
    }
    //鼠标离开事件
    if (event->type() == QEvent::Leave)
    {
        setIcon(normalIcon);
    }

    return QPushButton::event(event);
}
