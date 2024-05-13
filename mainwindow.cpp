#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initWindow();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initWindow()
{
    //窗口分割比例
    ui->splitter->setStretchFactor(0, 5);
    ui->splitter->setStretchFactor(1, 5);
    ui->splitter->setStretchFactor(2, 10);




}

