#include "mytreeviewdelegate.h"
#include "nodetreeitem.h"
#include "fontloader.h"
#include"pushbuttontype.h"

#include "defaultnotefolderdelegateeditor.h"
#include "trashbuttondelegateeditor.h"
#include "foldertreedelegateeditor.h"
#include "allnotebuttontreedelegateeditor.h"

myTreeViewDelegate::myTreeViewDelegate(QTreeView *view, QObject *parent, QListView *listView)
    : QStyledItemDelegate(parent),

//以下都是成员初始化列表

//定义展示内容的字体
#ifdef __APPLE__            //苹果系统
    m_displayFont(QFont(QStringLiteral("SF Pro Text")).exactMatch()
                      ? QStringLiteral("SF Pro Text")
                      : QStringLiteral("Roboto")),
#elif _WIN32                //windows
    m_displayFont(QFont(QStringLiteral("Segoe UI")).exactMatch() ?
                      QStringLiteral("Segoe UI")
                                                                 : QStringLiteral("Roboto")),
#else                       //其他系统
    m_displayFont(QStringLiteral("Roboto")),
#endif




//定义相关要展示的参数
#ifdef __APPLE__
    m_titleFont(m_displayFont, 13, QFont::DemiBold),
    m_titleSelectedFont(m_displayFont, 13),
    m_dateFont(m_displayFont, 13),
    m_separatorFont(m_displayFont, 12, QFont::Normal),
    m_numberOfNotesFont(m_displayFont, 12, QFont::DemiBold),
#else
    m_titleFont(m_displayFont, 10, QFont::DemiBold),            //标题颜色
    m_titleSelectedFont(m_displayFont, 10),                     //标题被选中的颜色
    m_dateFont(m_displayFont, 10),                              //日期颜色
    m_separatorFont(m_displayFont, 9, QFont::Normal),           //分割线的颜色
    m_numberOfNotesFont(m_displayFont, 9, QFont::DemiBold),     //"笔记数量"的颜色
#endif


    //和操作系统类型无关的参数
    m_titleColor(26, 26, 26),
    m_titleSelectedColor(255, 255, 255),
    m_dateColor(132, 132, 132),
    m_ActiveColor(68, 138, 201),
    m_notActiveColor(175, 212, 228),
    m_hoverColor(180, 208, 233),
    m_applicationInactiveColor(207, 207, 207),
    m_separatorColor(221, 221, 221),
    m_defaultColor(247, 247, 247),
    m_separatorTextColor(143, 143, 143),
    m_currentBackgroundColor(240, 240, 240),
    m_numberOfNotesColor(26, 26, 26, 127),
    m_numberOfNotesSelectedColor(255, 255, 255),
    m_folderIconColor(68, 138, 201),
    m_view(view),
    m_listView(listView),
    m_theme(Theme::Light)
{}

void myTreeViewDelegate::setTheme(Theme::Value theme)
{
    qDebug()<<"1";
    emit themeChanged(theme);
    m_theme = theme;

    switch (theme)
    {
qDebug()<<"2";
    case Theme::Light:
    {
qDebug()<<"3";
        m_titleColor = QColor(26, 26, 26);
        m_dateColor = QColor(26, 26, 26);
        m_defaultColor = QColor(247, 247, 247);
        //        m_ActiveColor = QColor(218, 233, 239);
        m_notActiveColor = QColor(175, 212, 228);
        m_hoverColor = QColor(180, 208, 233);
        m_currentBackgroundColor = QColor(240, 240, 240);  //247, 247, 247
        m_numberOfNotesColor = QColor(26, 26, 26, 127);
        break;
    }

    case Theme::Dark:
    {
        m_titleColor = QColor(212, 212, 212);
        m_dateColor = QColor(212, 212, 212);
        m_defaultColor = QColor(25, 25, 25);
        //        m_ActiveColor = QColor(0, 59, 148);
        m_notActiveColor = QColor(35, 52, 69);
        m_hoverColor = QColor(35, 52, 69);
        m_currentBackgroundColor = QColor(25, 25, 25);
        m_numberOfNotesColor = QColor(212, 212, 212, 127);
        break;
    }

    case Theme::Sepia:
    {
        m_titleColor = QColor(26, 26, 26);
        m_dateColor = QColor(26, 26, 26);
        m_defaultColor = QColor(251, 240, 217);
        //        m_ActiveColor = QColor(218, 233, 239);
        m_notActiveColor = QColor(175, 212, 228);
        m_hoverColor = QColor(180, 208, 233);
        m_currentBackgroundColor = QColor(251, 240, 217);
        m_numberOfNotesColor = QColor(26, 26, 26, 127);
        break;
    }

    }
}


void myTreeViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //qDebug()<<__FUNCTION__<<"paint";
    //设置抗锯齿渲染
    painter->setRenderHint(QPainter::Antialiasing);
    //获取传入的索引的项的类型
    auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());


//图标的位置的偏移量
#ifdef __APPLE__
    int iconPointSizeOffset = 0;
#else
    int iconPointSizeOffset = -4;
#endif


    //填充传入项的区域的背景颜色
    painter->fillRect(option.rect, m_currentBackgroundColor);
    //painter->fillRect(option.rect,QBrush(QColor(240,240,240)));

    //根据不同项进行不同的绘制
    switch (itemType)
    {
    //根节点项
    case NodeItem::Type::RootItem:
    {
        break;
    }

        //所有笔记按钮或者垃圾桶按钮
    case NodeItem::Type::AllNoteButton:
    case NodeItem::Type::TrashButton:
    {
        paintBackgroundSelectable(painter, option, index);

        //图标的位置

        auto iconRect = QRect(option.rect.x() + 22,  //+22
                              option.rect.y() + (option.rect.height() - 15) / 2, 45, 45); //18 20
        //qDebug()<<__FUNCTION__<<option.rect;
        //painter之前的字体留个备份
        QFont previousPainterFont = painter->font();


        //判断传入的项的状态
        if ((option.state & QStyle::State_Selected) == QStyle::State_Selected)
        {
            painter->setPen(m_titleSelectedColor);
        }
        else
        {
            painter->setPen(m_folderIconColor);
        }



        //绘制图标，根据不同主题确认参数
        if (m_theme == Theme::Dark)
        {
            if (itemType == NodeItem::Type::AllNoteButton)  //黑色主题，所有笔记
            {
//                painter->setFont(FontLoader::getInstance().loadFont("Material Symbols Outlined", "",
//                                                                    16 + iconPointSizeOffset));
                painter->setFont(FontLoader::getInstance().getFont());
                painter->drawText(iconRect, u8"\uf07b"); // folder图标  \ue2c7  \U0001F4C1
            }
            else if (itemType == NodeItem::Type::TrashButton)//黑色主题，垃圾桶
            {
                iconRect.setY(iconRect.y() + 2);
//                painter->setFont(FontLoader::getInstance().loadFont("Font Awesome 6 Free Solid", "",
//                                                                    16 + iconPointSizeOffset));
                 painter->setFont(FontLoader::getInstance().getFont());
                painter->drawText(iconRect, u8"\uf1f8"); // fa-trash图标  \U0001F5D1 \uf1f8
            }
        }

        else
        {
            auto iconPath = index.data(NodeItem::Roles::Icon).toString(); //索引对应的图标的位置
            if (itemType == NodeItem::Type::AllNoteButton)      //浅色主题，所有笔记
            {
//                painter->setFont(FontLoader::getInstance().loadFont("Material Symbols Outlined", "",
//                                                                    16 + iconPointSizeOffset));
                 painter->setFont(FontLoader::getInstance().getFont());
                painter->drawText(iconRect, u8"\uf07b"); // folder  \uf07b
            }
            else if(itemType == NodeItem::Type::TrashButton)           //浅色主题，垃圾桶
            {
                iconRect.setY(iconRect.y() + 2);
//                painter->setFont(FontLoader::getInstance().loadFont("Font Awesome 6 Free Solid", "",
//                                                                    16 + iconPointSizeOffset));
                 painter->setFont(FontLoader::getInstance().getFont());
                painter->drawText(iconRect, u8"\uf1f8"); // fa-trash
            }
        }




        //绘制标题
        painter->setFont(previousPainterFont);                                  //设置之前的字体
        auto displayName = index.data(NodeItem::Roles::DisplayText).toString(); //要展示的内容
        QRect nameRect(option.rect);                                            //展示的矩形框
        nameRect.setLeft(iconRect.x() + iconRect.width() + 10);//+5                  //设置矩形框位置,图标的右边5个像素
        nameRect.setWidth(nameRect.width() - 5 - 40);

        //传入的项的被选的状态，来设置不同的标题颜色
        if ((option.state & QStyle::State_Selected) == QStyle::State_Selected)  //选中
        {
            painter->setPen(m_titleSelectedColor);
        }
        else    //未选中
        {
            painter->setPen(m_titleColor);
        }

        //写标题名称
        painter->setFont(m_titleFont);          //设置标题字体
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);



        //写子节点数量
        //子节点数显示的矩形的位置
        auto childCountRect = option.rect;
        childCountRect.setLeft(nameRect.right() + 5);
        childCountRect.setWidth(childCountRect.width() - 5);

        //子节点数,设置绘制的颜色
        auto childCount = index.data(NodeItem::Roles::ChildCount).toInt();
        if ((option.state & QStyle::State_Selected) == QStyle::State_Selected)
        {
            painter->setPen(m_numberOfNotesSelectedColor);
        }
        else
        {
            painter->setPen(m_numberOfNotesColor);
        }

        painter->setFont(m_numberOfNotesFont); //字体
        painter->drawText(childCountRect, Qt::AlignHCenter | Qt::AlignVCenter,
                          QString::number(childCount));
        break;
    } //case






    //文件夹分隔符类型
    case NodeItem::Type::FolderSeparator:
    {
        //要绘制的矩形
        auto textRect = option.rect;            //要绘制的矩形范围
        textRect.moveLeft(textRect.x() + 5);    //设置位置
        textRect.moveBottom(textRect.y() + NoteTreeConstant::folderLabelHeight + 2);

        //展示的内容
        auto displayName = index.data(NodeItem::Roles::DisplayText).toString();

        //颜色字体
        painter->setPen(m_separatorColor);
        painter->setFont(m_separatorFont);

        //绘制内容
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
        break;
    }





    //文件夹类型
    case NodeItem::Type::FolderItem:
    {
        paintBackgroundSelectable(painter, option, index);



        //展开折叠的图标
        auto iconRect = QRect(option.rect.x() + 5,
                              option.rect.y() + (option.rect.height() - 15) / 2, 18, 20);
        QString iconPath;  //图标
//        painter->setFont(FontLoader::getInstance().loadFont("Font Awesome 6 Free Solid", "",
//                                                            10 + iconPointSizeOffset));
         painter->setFont(FontLoader::getInstance().getFont());

        //根据主题的深浅色来选择图标的显示内容
        if (m_theme == Theme::Dark)
        {
            painter->setPen(QColor(169, 160, 172));                         //图标颜色

            //根据文件夹打开折叠来选择图标
            if ((option.state & QStyle::State_Open) == QStyle::State_Open)  //图标内容
            {
                iconPath = u8"\uf078"; // fa-chevron-down   \uf078 \U000002C5
            }
            else
            {
                iconPath = u8"\uf054"; // fa-chevron-right
                iconRect.setX(iconRect.x() + 2);
            }
        }
        else
        {
            painter->setPen(QColor(103, 99, 105));
            if ((option.state & QStyle::State_Open) == QStyle::State_Open)
            {
                iconPath = u8"\uf078 "; // fa-chevron-down  \uf078   U+02C5
            }
            else
            {
                iconPath = u8"\uf054"; // fa-chevron-right U+02C3 \uf054
                iconRect.setX(iconRect.x() + 2);
            }
        }
        //判断这个项是否可以展开,可以就绘制折叠打开的图标
        if (index.data(NodeItem::Roles::IsExpandable).toBool())
        {
            painter->drawText(iconRect, iconPath);
        }





        //文件夹图标
//        QRect folderIconRect(option.rect);
//        folderIconRect.setLeft(iconRect.x() + iconRect.width() + 2);
//        folderIconRect.setTop(option.rect.y() + 10);
//        folderIconRect.setWidth(18);
        QRect folderIconRect = QRect(option.rect.x() + 30, option.rect.y() + (option.rect.height() - 15) / 2, 45, 45);
        //根据是否被选择设置颜色
        if ((option.state & QStyle::State_Selected) == QStyle::State_Selected)
        {
            painter->setPen(m_titleSelectedColor);
        }
        else
        {
            painter->setPen(m_folderIconColor);
        }
//        painter->setFont(FontLoader::getInstance().loadFont("Material Symbols Outlined", "",
//                                                            16 + iconPointSizeOffset)); //字体
         painter->setFont(FontLoader::getInstance().getFont());
        painter->drawText(folderIconRect, u8"\uf07b"); // folder图标   \ue2c7 \U0001F4C1



        //文件夹名字
        QRect nameRect(option.rect);
        //矩形的位置
        nameRect.setLeft(folderIconRect.x() + folderIconRect.width() + 10);
        nameRect.setWidth(nameRect.width() - 5 - 40);
        //测量文本的宽度，进行裁剪
        QFontMetrics fm(m_titleFont);
        auto displayName = index.data(NodeItem::Roles::DisplayText).toString();
        displayName = fm.elidedText(displayName, Qt::ElideRight, nameRect.width());
        //根据是否被选中设置颜色
        if ((option.state & QStyle::State_Selected) == QStyle::State_Selected)
        {
            painter->setPen(m_titleSelectedColor);
        }
        else
        {
            painter->setPen(m_titleColor);
        }
        //字体
        painter->setFont(m_titleFont);
        //绘制
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);


        //子节点数量的文本的绘制
        //显示的矩形的位置
        auto childCountRect = option.rect;
        childCountRect.setLeft(nameRect.right() + 5);
        childCountRect.setWidth(childCountRect.width() - 5);
        //数量
        auto childCount = index.data(NodeItem::Roles::ChildCount).toInt();
        //根据文件夹是否被选中设置颜色
        if ((option.state & QStyle::State_Selected) == QStyle::State_Selected)
        {
            painter->setPen(m_numberOfNotesSelectedColor);
        }
        else
        {
            painter->setPen(m_numberOfNotesColor);
        }
        painter->setFont(m_numberOfNotesFont); //字体
        //绘制
        painter->drawText(childCountRect, Qt::AlignHCenter | Qt::AlignVCenter,
                          QString::number(childCount));
        break;
    } //case


        //笔记项
    case NodeItem::Type::NoteItem:
    {
        paintBackgroundSelectable(painter, option, index);

        //名称
        QRect nameRect(option.rect);
        //设置矩形位置
        nameRect.setLeft(nameRect.x() + 10 + 5);
        nameRect.setWidth(nameRect.width() - 5);
        //显示的内容,调整文本宽度
        auto displayName = index.data(NodeItem::Roles::DisplayText).toString();
        QFontMetrics fm(m_titleFont);
        displayName = fm.elidedText(displayName, Qt::ElideRight, nameRect.width());
        //根据是否被选中设置颜色
        if ((option.state & QStyle::State_Selected) == QStyle::State_Selected)
        {
            painter->setPen(m_titleSelectedColor);
        }
        else
        {
            painter->setPen(m_titleColor);
        }
        painter->setFont(m_titleFont);
        //绘制
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
        break;
    }


    } //switch

}//fun


QSize myTreeViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize result = QStyledItemDelegate::sizeHint(option, index); //项的大小
    auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());//项的类型

    //调整高度

    //文件夹分隔符
    if (itemType == NodeItem::Type::FolderSeparator)
    {
        result.setHeight(NoteTreeConstant::folderLabelHeight); //高度
    }
    //文件夹，垃圾桶，所有文件按钮
    else if (itemType == NodeItem::Type::FolderItem || itemType == NodeItem::Type::TrashButton
             || itemType == NodeItem::Type::AllNoteButton)
    {
        result.setHeight(NoteTreeConstant::folderItemHeight);
    }
    //其余的类型
    else
    {
        result.setHeight(30);
    }

    return result;
}


QWidget *myTreeViewDelegate::createEditor
    (QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //传入的项的类型
    auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());


    //文件夹类型
    if (itemType == NodeItem::Type::FolderSeparator)
    {
        //编辑器样式
        auto widget = new QWidget(parent);
        widget->setContentsMargins(0, 0, 0, 0);
        auto layout = new QHBoxLayout(widget);
        layout->setContentsMargins(5, 7, 0, 0);
        widget->setLayout(layout);
        auto label = new QLabel(widget);
        auto displayName = index.data(NodeItem::Roles::DisplayText).toString();
        label->setStyleSheet(QStringLiteral("QLabel{color: rgb(%1, %2, %3);}")
                                 .arg(QString::number(m_separatorTextColor.red()),
                                      QString::number(m_separatorTextColor.green()),
                                      QString::number(m_separatorTextColor.blue())));
        label->setFont(m_separatorFont);
        label->setText(displayName);
        layout->addWidget(label);
        auto addButton = new PushButtonType(parent);
        addButton->setMaximumSize({ 38, 25 });
        addButton->setMinimumSize({ 38, 25 });
        addButton->setCursor(QCursor(Qt::PointingHandCursor));
        addButton->setFocusPolicy(Qt::TabFocus);

//图标位置偏移量
#ifdef __APPLE__
        int iconPointSizeOffset = 0;
#else
        int iconPointSizeOffset = -4;
#endif

        //设置按钮的字体、图标、样式表
//        addButton->setFont(FontLoader::getInstance().loadFont("Font Awesome 6 Free Solid", "",
//                                                              16 + iconPointSizeOffset));
        addButton->setFont(FontLoader::getInstance().getFont());
        addButton->setText(u8"\uf067"); // fa_plus  \U0000002B \uf067 \U00002795
        addButton->setStyleSheet(QStringLiteral(R"(QPushButton { )"
                                                R"(    border: none; )"
                                                R"(    padding: 0px; )"
                                                R"(    color: rgb(68, 138, 201); )"
                                                R"(})"
                                                R"(QPushButton:hover { )"
                                                R"(    border: none; )"
                                                R"(    padding: 0px; )"
                                                R"(    color: rgb(51, 110, 162); )"
                                                R"(})"
                                                R"(QPushButton:pressed { )"
                                                R"(    border: none; )"
                                                R"(    padding: 0px; )"
                                                R"(    color: rgb(39, 85, 125); )"
                                                R"(})"));


        //创建信号与槽
        if (itemType == NodeItem::Type::FolderSeparator)
        {
            //点击这个按钮添加一个文件夹
            connect(addButton, &QPushButton::clicked, this, &myTreeViewDelegate::addFolderRequested);
        }

        layout->addWidget(addButton, 1, Qt::AlignRight); //右对齐

        return widget;
    }




    //如果是文件夹，设置相应的编辑器控件
    else if (itemType == NodeItem::Type::FolderItem)
    {
        //看节点的ID是不是默认笔记文件夹
        auto id = index.data(NodeItem::Roles::NodeId).toInt();

        if (id == SpecialNodeID::DefaultNotesFolder)
        {
            auto widget =
                new DefaultNoteFolderDelegateEditor(m_view, option, index, m_listView, parent);
            widget->setTheme(m_theme);
            connect(this, &myTreeViewDelegate::themeChanged, widget,
                    &DefaultNoteFolderDelegateEditor::setTheme);
            return widget;
        }

        else
        {
            //qDebug()<<__FUNCTION__<<__LINE__<<"folder";
            auto widget = new FolderTreeDelegateEditor(m_view, option, index, m_listView, parent);
            widget->setTheme(m_theme);
            connect(this, &myTreeViewDelegate::themeChanged, widget,
                    &FolderTreeDelegateEditor::setTheme);
            return widget;
        }
    }

    //垃圾桶按钮
    else if (itemType == NodeItem::Type::TrashButton)
    {
        //qDebug()<<__FUNCTION__<<__LINE__<<"trashDelegate";
        auto widget = new TrashButtonDelegateEditor(m_view, option, index, m_listView, parent);
        widget->setTheme(m_theme);
        connect(this, &myTreeViewDelegate::themeChanged, widget,
                &TrashButtonDelegateEditor::setTheme);
        return widget;
    }

    //所有笔记按钮
    else if (itemType == NodeItem::Type::AllNoteButton)
    {
        auto widget =
            new AllNoteButtonTreeDelegateEditor(m_view, option, index, m_listView, parent);
        widget->setTheme(m_theme);
        connect(this, &myTreeViewDelegate::themeChanged, widget,
                &AllNoteButtonTreeDelegateEditor::setTheme);
        return widget;
    }

    return nullptr;
}


void myTreeViewDelegate::updateEditorGeometry
    (QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    QStyledItemDelegate::updateEditorGeometry(editor, option, index);


    auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
    if (itemType == NodeItem::Type::TrashButton || itemType == NodeItem::Type::AllNoteButton)
    {
        editor->setGeometry(0, editor->y(), option.rect.width(), option.rect.height());
    }
}


void myTreeViewDelegate::paintBackgroundSelectable(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //如果传入的项被选中，则填充活动状态的颜色
    if ((option.state & QStyle::State_Selected) == QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QBrush(m_ActiveColor));
    }

    //如果悬停在项的上面
    else if ((option.state & QStyle::State_MouseOver) == QStyle::State_MouseOver)
    {
        auto treeView = dynamic_cast<myTreeView*>(m_view);
        auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());//项的类型
        if (itemType == NodeItem::Type::TrashButton)
        {
            return;
        }
        if (!treeView->isDragging())  //项只是悬停没有拖动
        {
            painter->fillRect(option.rect, QBrush(m_hoverColor)); //填充悬停的颜色
        }
    }
}





















