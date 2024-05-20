#ifndef MYLISTVIEW_H
#define MYLISTVIEW_H

#include<QListView>
#include<QObject>

//自定义列表view
class myListView : public QListView
{
    Q_OBJECT
public:
    explicit myListView(QWidget* parent = nullptr);
};

#endif // MYLISTVIEW_H
