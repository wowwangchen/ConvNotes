#ifndef CUSTOMDOCUMENT_H
#define CUSTOMDOCUMENT_H

#include<QTextEdit>
class CustomDocument : public QTextEdit
{
public:
    CustomDocument(QWidget *parent = nullptr);
};

#endif // CUSTOMDOCUMENT_H
