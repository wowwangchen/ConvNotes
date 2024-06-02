#include "customDocument.h"
#include <QDebug>
#include <QGuiApplication>
#include <QTextCursor>
#include <QMessageBox>

CustomDocument::CustomDocument(QWidget *parent) : QTextEdit(parent)
{
    installEventFilter(this);                           //启动事件过滤器
    viewport()->installEventFilter(this);               //视口安装事件过滤器
    setMouseTracking(true);                             //鼠标追踪
    setAttribute(Qt::WidgetAttribute::WA_Hover, true);  //启动悬停属性
}

/*!
 * \brief CustomDocument::setDocumentPadding
 * We use a custom document for MainWindow::m_textEdit
 * so we can set the document padding without the (upstream Qt) issue
 * where the vertical scrollbar gets padded with the text as well.
 * This way, only the text gets padded, and the vertical scroll bar stays where it is.
 * \param left
 * \param top
 * \param right
 * \param bottom
 */
void CustomDocument::setDocumentPadding(int left, int top, int right, int bottom)
{
    setViewportMargins(left, top, right, bottom);
}

void CustomDocument::resizeEvent(QResizeEvent *event)
{
    QTextEdit::resizeEvent(event);
    emit resized();     //发送信号
}

void CustomDocument::mouseMoveEvent(QMouseEvent *event)
{
    QTextEdit::mouseMoveEvent(event);
    emit mouseMoved();      //鼠标移动的信号
}

bool CustomDocument::eventFilter(QObject *obj, QEvent *event)
{

    //悬停
    if (event->type() == QEvent::HoverMove)
    {
        // if hovering and the control key is active, check whether the mouse is over a link
        if (QGuiApplication::keyboardModifiers() == Qt::ExtraButton24
            && getUrlUnderMouse().isValid())
        {
            viewport()->setCursor(Qt::PointingHandCursor);  //手形状
        }
        else
        {
            viewport()->setCursor(Qt::IBeamCursor);         //默认样式
        }
    }

    //鼠标点击
    else if (event->type() == QEvent::KeyPress)
    {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        //按下了ctrl键
        if (keyEvent->key() == Qt::Key_Control)
        {
            //根据是否是url选择鼠标样式
            auto url = getUrlUnderMouse();
            viewport()->setCursor(url.isValid() ? Qt::PointingHandCursor : Qt::IBeamCursor);
        }

        //按下了alt键
        else if (keyEvent->modifiers().testFlag(Qt::AltModifier))
        {
            // alt+上，将这个块上移
            if (keyEvent->key() == Qt::Key_Up)
            {
                moveBlockUp();
                return true;
            }
            //alt+下
            else if (keyEvent->key() == Qt::Key_Down)
            {
                moveBlockDown();
                return true;
            }
        }
    }

    //鼠标释放
    else if (event->type() == QEvent::MouseButtonRelease)
    {

        auto *mouseEvent = static_cast<QMouseEvent *>(event);

        //之前点击了鼠标左键和附加键
        if ((obj == viewport()) && (mouseEvent->button() == Qt::LeftButton)
            && (QGuiApplication::keyboardModifiers() == Qt::ExtraButton24))
        {
            // open the link (if any) at the current position
            // in the noteTextEdit

            viewport()->setCursor(Qt::IBeamCursor); //文本光标

            openLinkAtCursorPosition();             //打开链接

            return true;
        }
    }

    //键盘释放
    else if (event->type() == QEvent::KeyRelease)
    {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        //ctrl键，设置文本光标
        if (keyEvent->key() == Qt::Key_Control)
        {
            viewport()->setCursor(Qt::IBeamCursor);
        }
    }

    return QTextEdit::eventFilter(obj, event);
}


QString CustomDocument::getMarkdownUrlAtPosition(const QString &text, int position)
{
    QString url;

    //获取传入的文本范围内的所有键值对
    const QMap<QString, QString> urlMap = parseMarkdownUrlsFromText(text);
    QMap<QString, QString>::const_iterator i = urlMap.constBegin();


    //遍历所有键值对，判断是否有内容在选中的内容下
    for (; i != urlMap.constEnd(); ++i)
    {
        const QString &linkText = i.key();      //键,string
        const QString &urlString = i.value();   //值,string

        const int foundPositionStart = text.indexOf(linkText);   //键开始的地方

        if (foundPositionStart >= 0)
        {
            //结尾的地方
            const int foundPositionEnd = foundPositionStart + linkText.size();

            //判断输入的地方是否在这个string的范围之内
            if ((position >= foundPositionStart) && (position < foundPositionEnd))
            {
                url = urlString;
                break;
            }
        }
    }

    return url;
}




QUrl CustomDocument::getUrlUnderMouse()
{
    //创建一个临时光标
    auto pos = viewport()->mapFromGlobal(QCursor::pos());  //光标所在位置
    QTextCursor cursor = cursorForPosition(pos);
    const int cursorPosition = cursor.position();

    //计算光标在文本块内的相对位置
    cursor.movePosition(QTextCursor::StartOfBlock);
    const int indexInBlock = cursorPosition - cursor.position();
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor); //移动到文本结尾保持选中状态

    //获取url
    return QUrl(getMarkdownUrlAtPosition(cursor.selectedText(), indexInBlock));
}




bool CustomDocument::openLinkAtCursorPosition()
{
    //找到url
    QUrl const url = getUrlUnderMouse();
    QString const urlString = url.toString();

    //是否为本地文件
    const bool isFileUrl = urlString.startsWith(QLatin1String("file://"));
    //是否为旧版附件
    const bool isLegacyAttachmentUrl = urlString.startsWith(QLatin1String("file://attachments"));
    //是否是本地文件路径
    const bool isLocalFilePath = urlString.startsWith(QLatin1String("/"));

    const bool convertLocalFilepathsToURLs = true;



    if ((url.isValid() && isValidUrl(urlString)) || isFileUrl || isLocalFilePath
        || isLegacyAttachmentUrl)
    {

        if (_ignoredClickUrlSchemata.contains(url.scheme()))
        {
            qDebug() << __func__ << "ignored URL scheme:" << urlString;
            return false;
        }

        // ignore non-existent files
        if (isFileUrl) {
            QString trimmed = urlString.mid(7);
            if (!QFile::exists(trimmed))
            {
                qDebug() << __func__ << ": File does not exist:" << urlString;
                // show a message box
                QMessageBox::warning(
                    nullptr, tr("File not found"),
                    tr("The file <strong>%1</strong> does not exist.").arg(trimmed));
                return false;
            }
        }

        if (isLocalFilePath && !QFile::exists(urlString))
        {
            qDebug() << __func__ << ": File does not exist:" << urlString;
            // show a message box
            QMessageBox::warning(nullptr, tr("File not found"),
                                 tr("The file <strong>%1</strong> does not exist.").arg(urlString));
            return false;
        }

        if (isLocalFilePath && convertLocalFilepathsToURLs)
        {
            openUrl(QString("file://") + urlString);
        }
        else
        {
            openUrl(urlString);
        }

        return true;
    }

    return false;
}


bool CustomDocument::isValidUrl(const QString &urlString)
{
    //正则表达式,多个字母开头 + "://" + 多个字符
    const QRegularExpressionMatch match = QRegularExpression(R"(^\w+:\/\/.+)").match(urlString);
    return match.hasMatch();
}


/**
 * Handles clicked urls
 *
 * examples:
 * - <https://www.qownnotes.org> opens the webpage
 * - <file:///path/to/my/file/QOwnNotes.pdf> opens the file
 *   "/path/to/my/file/QOwnNotes.pdf" if the operating system supports that
 *  handler
 */


//打开这个url
void CustomDocument::openUrl(const QString &urlString)
{
    qDebug() << "CustomDocument " << __func__ << " - 'urlString': " << urlString;

    QDesktopServices::openUrl(QUrl(urlString));
}




QMap<QString, QString> CustomDocument::parseMarkdownUrlsFromText(const QString &text)
{
    QMap<QString, QString> urlMap;  //键值对
    QRegularExpression regex;       //正则表达式
    QRegularExpressionMatchIterator iterator;  //表达式的迭代器



    // match urls like this: <http://mylink>  匹配<...>的内容
    //    re = QRegularExpression("(<(.+?:\\/\\/.+?)>)");
    regex = QRegularExpression(QStringLiteral("(<(.+?)>)"));
    iterator = regex.globalMatch(text);
    while (iterator.hasNext())
    {
        QRegularExpressionMatch match = iterator.next();
        QString linkText = match.captured(1);
        QString url = match.captured(2);
        urlMap[linkText] = url;
    }


    //[...](...)  带？非贪婪模式，不带？，贪婪模式
    regex = QRegularExpression(R"((\[.*?\]\((.+?)\)))");
    iterator = regex.globalMatch(text);
    while (iterator.hasNext())
    {
        QRegularExpressionMatch match = iterator.next();
        QString linkText = match.captured(1);
        QString url = match.captured(2);
        urlMap[linkText] = url;
    }




    // match urls like this: http://mylink
    regex = QRegularExpression(R"(\b\w+?:\/\/[^\s]+[^\s>\)])");
    iterator = regex.globalMatch(text);
    while (iterator.hasNext())
    {
        QRegularExpressionMatch match = iterator.next();
        QString url = match.captured(0);
        urlMap[url] = url;
    }




    // match urls like this: www.github.com
    regex = QRegularExpression(R"(\bwww\.[^\s]+\.[^\s]+\b)");
    iterator = regex.globalMatch(text);
    while (iterator.hasNext())
    {
        QRegularExpressionMatch match = iterator.next();
        QString url = match.captured(0);
        urlMap[url] = QStringLiteral("http://") + url;
    }



    // match reference urls like this: [this url][1] with this later:
    // [1]: http://domain
    regex = QRegularExpression(R"((\[.*?\]\[(.+?)\]))");
    iterator = regex.globalMatch(text);
    while (iterator.hasNext())
    {
        QRegularExpressionMatch match = iterator.next();
        QString linkText = match.captured(1);
        QString referenceId = match.captured(2);

        QRegularExpression refRegExp(QStringLiteral("\\[") + QRegularExpression::escape(referenceId)
                                     + QStringLiteral("\\]: (.+)"));
        QRegularExpressionMatch urlMatch = refRegExp.match(toPlainText());

        if (urlMatch.hasMatch())
        {
            QString url = urlMatch.captured(1);
            urlMap[linkText] = url;
        }
    }

    return urlMap;
}

void CustomDocument::moveBlockUp()
{
    //获取光标的位置，获取块的内容和水平位置，删除块的内容
    QTextCursor cursor = textCursor();

    if (cursor.blockNumber() > 0)
    {
        QString currentBlock = cursor.block().text();
        const int currentHorizontalPosition = cursor.positionInBlock();

        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        // also remove empty line
        cursor.deletePreviousChar();  //光标删除当前所在的前面的字符，相当于移除空白地方，移动到上一行的结尾

        //移动到这一行的开头
        if (!cursor.movePosition(QTextCursor::StartOfBlock))
        {
            // block above is empty, this is fine
        }
        //插入刚刚删除的内容
        cursor.insertText(currentBlock);
        cursor.insertBlock();

        //光标移动到相对位置
        if (!cursor.movePosition(QTextCursor::PreviousBlock))
        {
            qDebug() << "Could not move to previous block";
        }
        const int startPosition = cursor.position();
        cursor.setPosition(startPosition + currentHorizontalPosition);

        setTextCursor(cursor);
    }
}



void CustomDocument::moveBlockDown()
{
    QTextCursor cursor = textCursor(); //textedit所在光标对象

    //光标所在块的内容
    QString currentBlock = cursor.block().text();
    const int currentHorizontalPosition = cursor.positionInBlock(); //当前水平位置

    //光标移动到块开头，并选中整个块，并删除它
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    // also remove empty line
    cursor.deleteChar();//光标删除当前所在的字符，相当于移除回车,移动到下一行的开头

    //移动到当前块的最后,然后再插入刚刚删除的内容
    if (!cursor.movePosition(QTextCursor::EndOfBlock))
    {
        // block below is empty, this is fine
    }
    cursor.insertBlock();
    cursor.insertText(currentBlock);

    //将光标移动到块的最前面
    if (!cursor.movePosition(QTextCursor::StartOfBlock))
    {
        qDebug() << "Could not move to start of next block";
    }
    const int startPosition = cursor.position();
    //根据之前的水平位置，设置光标位置，来达到伪移动的功能
    cursor.setPosition(startPosition + currentHorizontalPosition);

    setTextCursor(cursor);
}
