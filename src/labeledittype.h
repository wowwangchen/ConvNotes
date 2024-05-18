#ifndef LABELEDITTYPE_H
#define LABELEDITTYPE_H

#include<QLabel>
#include<QLineEdit>
#include<QBoxLayout>


//自定义label，在标签上实现编辑功能
class LabelEditType : public QLabel
{
    Q_OBJECT
public:
    explicit LabelEditType(QWidget *parent = nullptr);

public slots:
    //打开编辑器
    void openEditor();
    //完成编辑后的操作
    void onFinishedEdit();

signals:
    //开始编辑的信号
    void editingStarted();
    //完成编辑的信号
    void editingFinished(const QString &text);

private:
    QLineEdit *m_editor;
};

#endif // LABELEDITTYPE_H
