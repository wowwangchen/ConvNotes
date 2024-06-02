#ifndef CUSTOMDOCUMENT_H
#define CUSTOMDOCUMENT_H

#include<QTextEdit>
#include<QObject>
#include <QtGui>
#include <QStringList>
#include<QMetaObject>
#include <QTextEdit>


//自定义文本的
class CustomDocument : public QTextEdit
{
    Q_OBJECT

public:
    CustomDocument(QWidget *parent = nullptr);
    //设置视口的范围
    void setDocumentPadding(int left, int top, int right, int bottom);
    //事件过滤
    bool eventFilter(QObject *obj, QEvent *event);
    //打开光标位置的链接
    bool openLinkAtCursorPosition();
    //查找给定文本且在指定位置下的url链接
    QString getMarkdownUrlAtPosition(const QString &text, int position);
    //是否为合法链接
    bool isValidUrl(const QString &urlString);
    //打开链接
    void openUrl(const QString &urlString);
    //从指定文本中获取所有url
    QMap<QString, QString> parseMarkdownUrlsFromText(const QString &text);
    //获取鼠标光标下的链接
    QUrl getUrlUnderMouse();
    //将某块内容向上移动
    void moveBlockUp();
    //将某块内容向下移动
    void moveBlockDown();

signals:
    void resized();
    void mouseMoved();


    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    QStringList _ignoredClickUrlSchemata;
};

#endif // CUSTOMDOCUMENT_H
