#ifndef MOREBUTTON_H
#define MOREBUTTON_H

#include <QWidget>
#include<QMouseEvent>
#include<QDebug>
#include<QDialog>
namespace Ui {
class MoreButton;
}

class MoreButton : public QDialog
{
    Q_OBJECT

public:
    explicit MoreButton(QWidget *parent = nullptr);
    ~MoreButton();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

signals:
    void reduceSize();
    void addSize();
    void requestHide();


protected:
    virtual void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::MoreButton *ui;
};

#endif // MOREBUTTON_H
