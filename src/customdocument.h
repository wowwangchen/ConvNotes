#ifndef CUSTOMDOCUMENT_H
#define CUSTOMDOCUMENT_H

#include<QTextEdit>

//自定义文本的
class CustomDocument : public QTextEdit
{
public:
    CustomDocument(QWidget* parent=nullptr);
};

#endif // CUSTOMDOCUMENT_H
