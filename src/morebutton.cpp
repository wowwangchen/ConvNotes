#include "morebutton.h"
#include "ui_morebutton.h"
#include"fontloader.h"
MoreButton::MoreButton(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MoreButton)
{
    ui->setupUi(this);

    setWindowFlag(Qt::Window, false);
    setWindowFlag(Qt::FramelessWindowHint, true);



    QFont myFont=FontLoader::getInstance().getFont();
    ui->pushButton->setFont(myFont);
    ui->pushButton->setText(u8"\uf068");

    ui->pushButton_2->setFont(myFont);
    ui->pushButton_2->setText(u8"\uf067");
}

MoreButton::~MoreButton()
{
    delete ui;
}

void MoreButton::on_pushButton_clicked()
{
    emit reduceSize();
}


void MoreButton::on_pushButton_2_clicked()
{
    emit addSize();
}

void MoreButton::mousePressEvent(QMouseEvent *event)
{
    QRect button1Rect = ui->pushButton->geometry();
    QRect button2Rect = ui->pushButton_2->geometry();
    QPoint clickPos = event->pos();
    if (button1Rect.contains(clickPos) || button2Rect.contains(clickPos))
    {
        QWidget::mousePressEvent(event);
    }
    else
        emit requestHide();

}

