#include "foldertreedelegateeditor.h"
#include "fontloader.h"
#include <QHBoxLayout>
#include "mytreeview.h"

FolderTreeDelegateEditor::FolderTreeDelegateEditor(QTreeView *view, const QStyleOptionViewItem &option,
                                                   const QModelIndex &index, QListView *listView, QWidget *parent)
    : QWidget(parent),
    m_option(option),
    m_index(index),

#ifdef __APPLE__
    m_displayFont(QFont(QStringLiteral("SF Pro Text")).exactMatch()
                      ? QStringLiteral("SF Pro Text")
                      : QStringLiteral("Roboto")),
#elif _WIN32
    m_displayFont(QFont(QStringLiteral("Segoe UI")).exactMatch() ? QStringLiteral("Segoe UI")
                                                                 : QStringLiteral("Roboto")),
#else
    m_displayFont(QStringLiteral("Roboto")),
#endif



#ifdef __APPLE__
    m_titleFont(m_displayFont, 13, QFont::DemiBold),
#else
    m_titleFont(m_displayFont, 10, QFont::DemiBold),
#endif



    m_titleColor(26, 26, 26),
    m_titleSelectedColor(255, 255, 255),
    m_activeColor(25, 25, 25),
    //m_activeColor(68, 138, 201),
    m_hoverColor(207, 207, 207),                //这里有问题，代理编辑器的颜色
    m_folderIconColor(68, 138, 201),
    m_view(view),
    m_listView(listView),
    m_theme(Theme::Light)
{
    setContentsMargins(0, 0, 0, 0);
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
    m_expandIcon = new QLabel(this);
    m_expandIcon->setMinimumSize({ 18, 20 });
    m_expandIcon->setMaximumSize({ 18, 20 });

    if (m_index.data(NodeItem::Roles::IsExpandable).toBool())
    {
        if (!m_view->isExpanded(m_index))
        {
            layout->addSpacing(10);
        }
    }

#ifdef __APPLE__
    int iconPointSizeOffset = 0;
#else
    int iconPointSizeOffset = -4;
#endif

    //展开折叠图标
//    m_expandIcon->setFont(FontLoader::getInstance().loadFont("Font Awesome 6 Free Solid", "",
//                                                             0 + iconPointSizeOffset));
    m_expandIcon->setFont(FontLoader::getInstance().getFont());

    m_expandIcon->setScaledContents(true);
    layout->addWidget(m_expandIcon);

    if (!m_index.data(NodeItem::Roles::IsExpandable).toBool()
        || (m_index.data(NodeItem::Roles::IsExpandable).toBool() && m_view->isExpanded(m_index))) {
        layout->addSpacing(10);
    }



    //文件夹图标
    m_folderIcon = new PushButtonType(parent);
    m_folderIcon->setMaximumSize({ 19, 21 });
    m_folderIcon->setMinimumSize({ 19, 21 });
    m_folderIcon->setIconSize(QSize(19, 21));
    //QFont materialSymbols("Material Symbols Outlined", 15 + iconPointSizeOffset);
    //m_folderIcon->setFont(materialSymbols);
    m_folderIcon->setFont(FontLoader::getInstance().getFont());
    m_folderIcon->setText(u8"\uf07b"); // folder  \ue2c7  uf07b
    layout->addWidget(m_folderIcon);
    layout->addSpacing(35);




    //展示的内容
    m_label = new LabelEditType(this);
    m_label->setFont(m_titleFont);
    QSizePolicy labelPolicy;
    labelPolicy.setVerticalPolicy(QSizePolicy::Expanding);
    labelPolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    m_label->setSizePolicy(labelPolicy);
    m_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    //正在编辑状态
    connect(m_label, &LabelEditType::editingStarted, this, [this] {
        auto tree_view = dynamic_cast<myTreeView *>(m_view);
        tree_view->setIsEditing(true);
    });
    //编辑完成状态
    connect(m_label, &LabelEditType::editingFinished, this, [this](const QString &label) {
        auto tree_view = dynamic_cast<myTreeView *>(m_view);
        tree_view->onRenameFolderFinished(label);
        tree_view->setIsEditing(false);
    });
    //重命名
    connect(dynamic_cast<myTreeView *>(m_view), &myTreeView::renameFolderRequested, m_label,
            &LabelEditType::openEditor);

    layout->addWidget(m_label);
    layout->addSpacing(10);
    m_contextButton = new PushButtonType(parent);
    m_contextButton->setMaximumSize({ 45, 35 });
    m_contextButton->setMinimumSize({ 45, 35 }); //33 25
    m_contextButton->setCursor(QCursor(Qt::PointingHandCursor));
    m_contextButton->setFocusPolicy(Qt::TabFocus);
    if (m_view->selectionModel()->isSelected(m_index))
    {
        m_contextButton->setStyleSheet(QStringLiteral(R"(QPushButton { )"
                                                      R"(    border: none; )"
                                                      R"(    padding: 0px; )"
                                                      R"(    color: white; )"
                                                      R"(})"
                                                      R"(QPushButton:pressed { )"
                                                      R"(    border: none; )"
                                                      R"(    padding: 0px; )"
                                                      R"(    color: rgb(210, 210, 210); )"
                                                      R"(})")); //210, 210, 210
    }
    else
    {
        m_contextButton->setStyleSheet(QStringLiteral(R"(QPushButton { )"
                                                      R"(    border: none; )"
                                                      R"(    padding: 0px; )"
                                                      R"(    color: rgb(68, 138, 201); )"
                                                      R"(})"
                                                      R"(QPushButton:pressed { )"
                                                      R"(    border: none; )"
                                                      R"(    padding: 0px; )"
                                                      R"(    color: rgb(39, 85, 125); )"
                                                      R"(})"));
    }




#ifdef __APPLE__
    int pointSizeOffset = 0;
#else
    int pointSizeOffset = -4;
#endif


    //更多选项的按钮
//    m_contextButton->setFont(FontLoader::getInstance().loadFont("Font Awesome 6 Free Solid", "",
//                                                                14 + pointSizeOffset));
    m_contextButton->setFont(FontLoader::getInstance().getFont());
    m_contextButton->setText(u8"\uf141"); // fa-ellipsis-h u8"\uf141 \U0001F3F5



    connect(m_contextButton, &QPushButton::clicked, m_view, [this](bool) {
        auto tree_view = dynamic_cast<myTreeView *>(m_view);
        //        tree_view->setCurrentIndexC(m_index);
        tree_view->onCustomContextMenu(tree_view->visualRect(m_index).topLeft()
                                       + m_contextButton->geometry().bottomLeft());
    });
    layout->addWidget(m_contextButton, 0, Qt::AlignRight);
    layout->addSpacing(5);
    connect(m_view, &QTreeView::expanded, this, [this](const QModelIndex &) { update(); });

}


void FolderTreeDelegateEditor::setTheme(Theme::Value theme)
{
    m_theme = theme;
    switch (theme)
    {
    case Theme::Light:
    {
        m_hoverColor = QColor(247, 247, 247);
        m_titleColor = QColor(26, 26, 26);
        break;
    }
    case Theme::Dark:
    {
        m_hoverColor = QColor(25, 25, 25);
        m_titleColor = QColor(212, 212, 212);
        break;
    }
    case Theme::Sepia:
    {
        m_hoverColor = QColor(251, 240, 217);
        m_titleColor = QColor(26, 26, 26);
        break;
    }
    }
}

void FolderTreeDelegateEditor::updateDelegate()
{
    //显示的文本
    auto displayName = m_index.data(NodeItem::Roles::DisplayText).toString();
    QFontMetrics fm(m_titleFont);
    displayName = fm.elidedText(displayName, Qt::ElideRight, m_label->contentsRect().width());

    //显示的label的样式表
    if (m_view->selectionModel()->isSelected(m_index))
    {
        m_label->setStyleSheet(QStringLiteral("QLabel{color: rgb(%1, %2, %3);}")
                                   .arg(QString::number(m_titleSelectedColor.red()),
                                        QString::number(m_titleSelectedColor.green()),
                                        QString::number(m_titleSelectedColor.blue())));
        m_folderIcon->setStyleSheet(
            QStringLiteral("QPushButton{border: none; padding: 0px; color: rgb(%1, %2, %3);}")
                .arg(QString::number(m_titleSelectedColor.red()),
                     QString::number(m_titleSelectedColor.green()),
                     QString::number(m_titleSelectedColor.blue())));
    }
    else
    {
        m_label->setStyleSheet(QStringLiteral("QLabel{color: rgb(%1, %2, %3);}")
                                   .arg(QString::number(m_titleColor.red()),
                                        QString::number(m_titleColor.green()),
                                        QString::number(m_titleColor.blue())));
        m_folderIcon->setStyleSheet(
            QStringLiteral("QPushButton{border: none; padding: 0px; color: rgb(%1, %2, %3);}")
                .arg(QString::number(m_folderIconColor.red()),
                     QString::number(m_folderIconColor.green()),
                     QString::number(m_folderIconColor.blue())));
    }

    m_label->setText(displayName);
    auto theme = dynamic_cast<myTreeView*>(m_view)->theme();


    //折叠展开状态的图标
    QColor chevronColor(theme == Theme::Dark ? QColor(169, 160, 172) : QColor(103, 99, 105));
    m_expandIcon->setStyleSheet(QStringLiteral("QLabel{color: rgb(%1, %2, %3);}")
                                    .arg(QString::number(chevronColor.red()),
                                         QString::number(chevronColor.green()),
                                         QString::number(chevronColor.blue())));

    if (m_index.data(NodeItem::Roles::IsExpandable).toBool())
    {
        if (m_view->isExpanded(m_index))
        {
            m_expandIcon->setText(u8"\uf078"); // fa-chevron-down
        }
        else
        {
            m_expandIcon->setText(u8"\uf054"); // fa-chevron-right
        }
    }
}

void FolderTreeDelegateEditor::paintEvent(QPaintEvent *event)
{
    //qDebug()<<__FUNCTION__<<__LINE__<<rect();
    //绘制代理编辑器控件界面
    updateDelegate();
    QPainter painter(this);
    if (m_view->selectionModel()->isSelected(m_index))
    {
        painter.fillRect(rect(), QBrush(m_activeColor));
    }
    else
    {
        auto listView = dynamic_cast<myTreeView *>(m_listView);
        if (listView->isDragging())
        {
            if (m_theme == Theme::Dark)
            {
                painter.fillRect(rect(), QBrush(QColor(35, 52, 69)));  //35, 52, 69
            }
            else
            {
                painter.fillRect(rect(), QBrush(QColor(180, 208, 233)));
            }
        }
        else
        {
            painter.fillRect(rect(), QBrush(m_hoverColor));
        }
    }
    QWidget::paintEvent(event);
}

void FolderTreeDelegateEditor::mouseDoubleClickEvent(QMouseEvent *event)
{
    //双击打开编辑器
    if (m_label->geometry().contains(event->pos()))
    {
        m_label->openEditor();
    }
    else
    {
        QWidget::mouseDoubleClickEvent(event);
    }
}
