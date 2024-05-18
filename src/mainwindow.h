#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QMessageBox>
#include<QDateTime>
#include<QColorDialog>
#include<QMouseEvent>
#include<QEvent>
#include"mytreeview.h"
#include"mytreeviewdelegate.h"
#include"mytreeviewmodel.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


//整个项目的主界面
//分为treeView，listView，textEdit
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void initWindow();
    void initConnect();
    void setColorDialogSS(QColorDialog *dialog);

private slots:
    void settingButton_clicked();
protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual bool eventFilter(QObject *obj, QEvent *event) override;


private:
    Ui::MainWindow *ui;
    myTreeViewModel*        m_treeModel;
    myTreeViewDelegate*     m_treeDelegate;

};
#endif // MAINWINDOW_H
