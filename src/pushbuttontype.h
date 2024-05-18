#ifndef PUSHBUTTONTYPE_H
#define PUSHBUTTONTYPE_H
#include <QPushButton>
#include<QEvent>


//自定义按钮，增加图标修改功能
class PushButtonType : public QPushButton
{
    Q_OBJECT
public:
    explicit PushButtonType(QWidget *parent = nullptr);

    //更换不同的图标
    void setNormalIcon(const QIcon &newNormalIcon);     //普通图标
    void setHoveredIcon(const QIcon &newHoveredIcon);   //鼠标悬停图标
    void setPressedIcon(const QIcon &newPressedIcon);   //点击图标

protected:
    bool event(QEvent *event) override;  //事件处理

private:
    QIcon normalIcon;
    QIcon hoveredIcon;
    QIcon pressedIcon;
};

#endif // PUSHBUTTONTYPE_H
