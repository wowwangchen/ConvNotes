#include "mainwindow.h"

#include <QApplication>
#include<QTextCodec>
#include<QMetaType>
#include "codetranslate.h"

int main(int argc, char *argv[])
{

    qRegisterMetaType< QVector<NodeData> >("QVector<NodeData>");
    qRegisterMetaType< ListViewInfo >("ListViewInfo");

    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
