#include "notelistdelegate.h"
#include"notelistdelegateeditor.h"
#include"noteeditorlogic.h"
#include"fontloader.h"

NoteListDelegate::NoteListDelegate(myListView *view, QObject *parent):
    QStyledItemDelegate(parent),
    m_view{ view },

    //根据不同操作系统，选择展示字体
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



//根据不同OS,选择标题字体、标题选中字体、日期字体、抬头字体
#ifdef __APPLE__
    m_titleFont(m_displayFont, 13, QFont::DemiBold),
    m_titleSelectedFont(m_displayFont, 13, QFont::DemiBold),
    m_dateFont(m_displayFont, 13),
    m_headerFont(m_displayFont, 10, QFont::DemiBold),
#else
    m_titleFont(m_displayFont, 10, QFont::DemiBold),
    m_titleSelectedFont(m_displayFont, 10),
    m_dateFont(m_displayFont, 10),
    m_headerFont(m_displayFont, 10, QFont::DemiBold),
#endif


    //相关参数，默认值
    m_titleColor(26, 26, 26),
    m_dateColor(26, 26, 26),
    m_contentColor(142, 146, 150),
    m_ActiveColor(218, 233, 239),
    m_notActiveColor(175, 212, 228),
    m_hoverColor(207, 207, 207),
    m_applicationInactiveColor(207, 207, 207),
    m_separatorColor(191, 191, 191),
    m_defaultColor(247, 247, 247),
    m_rowHeight(106),
    m_maxFrame(200),
    m_rowRightOffset(0),
    m_state(NoteListState::Normal),
    m_isActive(false),
    m_isInAllNotes(false),
    m_theme(Theme::Light)

{
    //图标
    m_folderIcon = QImage(":/image/folder.png");


    //动画相关
    m_timeLine = new QTimeLine(300, this);  //动画时间线时长(有this指针，根据qt对象树系统，不用手动析构)
    m_timeLine->setFrameRange(0, m_maxFrame);
    m_timeLine->setUpdateInterval(10);
    m_timeLine->setEasingCurve(QEasingCurve::InCurve); //速度先慢后快


    //项的大小发生改变
    connect(m_timeLine, &QTimeLine::frameChanged, this, [this]() {
        for (const auto &index : qAsConst(m_animatedIndexes))
        {
            emit sizeHintChanged(index);
        }
    });

    //动画已完成
    connect(m_timeLine, &QTimeLine::finished, this, [this]() {
        emit animationFinished(m_state);

        //打开持久化编辑器
        for (const auto &index : qAsConst(m_animatedIndexes))
        {
            m_view->openPersistentEditorC(index);
        }


        //队列不为空
        if (!animationQueue.empty())
        {
            //取出队列中的第一个
            auto a = animationQueue.front();
            animationQueue.pop_front();

            //根据其中的id集合,通过id获取index，添加到indexs中
            QModelIndexList indexes;
            for (const auto &id : qAsConst(a.first))
            {

                auto model = dynamic_cast<NoteListModel *>(m_view->model());
                if (model)
                {
                    auto index = model->getNoteIndex(id);
                    if (index.isValid())
                    {
                        indexes.push_back(index);
                    }
                }
            }
            setStateI(a.second, indexes); //设置这些索引，对应的值
        }
        else
        {
            m_animatedIndexes.clear();
            m_state = NoteListState::Normal; //没有，就设置为普通状态
        }
    });
}


void NoteListDelegate::setState(NoteListState NewState, QModelIndexList indexes)
{
    //动画状态是不运行状态
    if (animationState() != QTimeLine::NotRunning)
    {
        //遍历所有传入的索引，获取节点对应的id
        QSet<int> ids;
        for (const auto &index : qAsConst(indexes))
        {
            auto noteId = index.data(NoteListModel::NoteID).toInt();
            if (noteId != SpecialNodeID::InvalidNodeId)
            {
                ids.insert(noteId);
            }
        }
        if (!ids.empty())
        {
            animationQueue.push_back(qMakePair(ids, NewState)); //这个id集合与状态值组成的键值对添加到队列中
        }
    }
    //为暂停或者运行状态，进入函数
    else
    {
        setStateI(NewState, indexes);
    }
}

void NoteListDelegate::setStateI(NoteListState NewState, const QModelIndexList &indexes)
{
    //先添加到动画索引列表中
    m_animatedIndexes = indexes;

    //定义一个函数，关闭之前的持久化编辑器，定义动画的实施的方向、时间然后启动动画
    auto startAnimation = [this](QTimeLine::Direction diretion, int duration)
    {
        for (const auto &index : qAsConst(m_animatedIndexes))
        {
            m_view->closePersistentEditorC(index);
        }
        m_timeLine->setDirection(diretion);
        m_timeLine->setDuration(duration);
        m_timeLine->start();
    };

    //根据自定义状态枚举，选择动画实施方式
    switch (NewState)
    {
    case NoteListState::Insert:
        startAnimation(QTimeLine::Forward, m_maxFrame);
        break;
    case NoteListState::Remove:
        startAnimation(QTimeLine::Backward, m_maxFrame);
        break;
    case NoteListState::MoveOut:
        startAnimation(QTimeLine::Backward, m_maxFrame);
        break;
    case NoteListState::MoveIn:
        startAnimation(QTimeLine::Backward, m_maxFrame);
        break;
    case NoteListState::Normal:
        m_animatedIndexes.clear();
        break;
    }

    m_state = NewState;
}

void NoteListDelegate::setActive(bool isActive)
{
    m_isActive = isActive;
}

void NoteListDelegate::setTheme(Theme::Value theme)
{
    m_theme = theme;

    //根据主体设置参数
    //标题颜色、日期颜色、内容颜色、默认颜色、活动颜色、非活动颜色、鼠标悬停颜色、应用非活动颜色、分隔符颜色
    switch (m_theme)
    {
    case Theme::Light:
    {
        m_titleColor = QColor(26, 26, 26);
        m_dateColor = QColor(26, 26, 26);
        m_contentColor = QColor(142, 146, 150);
        m_defaultColor = QColor(247, 247, 247);
        m_ActiveColor = QColor(218, 233, 239);
        m_notActiveColor = QColor(175, 212, 228);
        m_hoverColor = QColor(207, 207, 207);
        m_applicationInactiveColor = QColor(207, 207, 207);
        m_separatorColor = QColor(191, 191, 191);
        break;
    }
    case Theme::Dark:
    {
        m_titleColor = QColor(255, 255, 255);
        m_dateColor = QColor(255, 255, 255);
        m_contentColor = QColor(255, 255, 255, 127);
        m_defaultColor = QColor(25, 25, 25);
        m_ActiveColor = QColor(35, 52, 69, 127);
        m_notActiveColor = QColor(35, 52, 69);
        m_hoverColor = QColor(35, 52, 69, 127);
        m_applicationInactiveColor = QColor(35, 52, 69);
        m_separatorColor = QColor(255, 255, 255, 127);
        break;
    }
    case Theme::Sepia:
    {
        m_titleColor = QColor(26, 26, 26);
        m_dateColor = QColor(26, 26, 26);
        m_contentColor = QColor(142, 146, 150);
        m_defaultColor = QColor(251, 240, 217);
        m_ActiveColor = QColor(218, 233, 239);
        m_notActiveColor = QColor(175, 212, 228);
        m_hoverColor = QColor(207, 207, 207);
        m_applicationInactiveColor = QColor(207, 207, 207);
        m_separatorColor = QColor(191, 191, 191);
        break;
    }
    }

    emit themeChanged(m_theme);
}

Theme::Value NoteListDelegate::theme() const
{
    return m_theme;
}

void NoteListDelegate::setIsInAllNotes(bool newIsInAllNotes)
{
    m_isInAllNotes = newIsInAllNotes;
}

bool NoteListDelegate::isInAllNotes() const
{
    return m_isInAllNotes;
}

void NoteListDelegate::clearSizeMap()
{
    szMap.clear();
}

const QModelIndex &NoteListDelegate::hoveredIndex() const
{
    return m_hoveredIndex;
}

bool NoteListDelegate::shouldPaintSeparator(const QModelIndex &index, const NoteListModel &model) const
{
    //当前行为最后一行
    if (index.row() == model.rowCount() - 1)
    {
        return false;
    }


    const auto &selectedIndexes = m_view->selectedIndex();   //所有选中的行
    bool isCurrentSelected = selectedIndexes.contains(index);//当前行是否被选中

    //判断这一行的下一行是否也被选中
    bool isNextRowSelected = false;
    for (const auto &selected : selectedIndexes)
    {
        if (index.row() == selected.row() - 1)
        {
            isNextRowSelected = true;
        }
    }

    //当前行选中且当前行为鼠标悬停的行或者悬停行的上一行
    if (!isCurrentSelected
        && ((index.row() == m_hoveredIndex.row() - 1) || index.row() == m_hoveredIndex.row()))
    {
        return false;
    }

    //只有当前行和下一行同时被选中或者同时未被选中
    if (isCurrentSelected == isNextRowSelected)
    {
        //当前索引不是普通区的第一行的上一行(置顶区的最后一行)且置顶区折叠了
        if (index.row() != model.getFirstUnpinnedNote().row() - 1)
        {
            if (m_view && m_view->isPinnedNotesCollapsed())
            {
                //如果当前索引不是是折叠状态，返回true；是折叠状态返回false
                auto isPinned = index.data(NoteListModel::NoteIsPinned).value<bool>();
                if (!isPinned)
                {
                    return true;
                }
            }
            else
            {
                return true;
            }
        }
    }

    return false;
}

void NoteListDelegate::paintBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //获取项对应的大小
    auto bufferSize = bufferSizeHint(option, index);

    //以这个大小填充一个图片
    QPixmap buffer{ bufferSize };
    buffer.fill(Qt::transparent);

    //在这个图片上进行绘制
    QPainter bufferPainter{ &buffer };
    bufferPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);//抗锯齿，平滑像素转换

    QRect bufferRect = buffer.rect();
    auto isPinned = index.data(NoteListModel::NoteIsPinned).toBool();
    auto model = dynamic_cast<NoteListModel *>(m_view->model());

    //当前这个节点是第一个置顶的笔记并且置顶区是折叠的
    if (model && model->hasPinnedNote() && model->isFirstPinnedNote(index)
        && dynamic_cast<myListView *>(m_view)->isPinnedNotesCollapsed())
    {
        bufferPainter.fillRect(bufferRect, QBrush(m_defaultColor));
    }

    //当前模型是选中状态
    else if ((option.state & QStyle::State_Selected) == QStyle::State_Selected)
    {
        //当前程序是活动状态，获取焦点正在交互
        if (qApp->applicationState() == Qt::ApplicationActive)
        {
            //模型是活动状态，选择不同的填充颜色
            if (m_isActive)
            {
                bufferPainter.fillRect(bufferRect, QBrush(m_ActiveColor));
            }
            else
            {
                bufferPainter.fillRect(bufferRect, QBrush(m_notActiveColor));
            }
        }
        //当前程序不是活动状态
        else if (qApp->applicationState() == Qt::ApplicationInactive)
        {
            bufferPainter.fillRect(bufferRect, QBrush(m_applicationInactiveColor));
        }
    }

    //当前模型是鼠标悬停状态
    else if ((option.state & QStyle::State_MouseOver) == QStyle::State_MouseOver)
    {
        //列表组件被鼠标拖拽住且当前项是置顶的
        if (dynamic_cast<myListView *>(m_view)->isDragging())
        {
            //且这个节点是置顶的
            if (isPinned)
            {
                auto rect = bufferRect;
                rect.setTop(rect.bottom() - 5);
                bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            }
        }
        else
        {
            bufferPainter.fillRect(bufferRect, QBrush(m_hoverColor));
        }
    }

    else
    {
        //置顶区折叠且当前节点不是置顶的
        if (m_view->isPinnedNotesCollapsed())
        {
            if (!isPinned)
            {
                bufferPainter.fillRect(bufferRect, QBrush(m_defaultColor));
            }
        }
        else
        {
            bufferPainter.fillRect(bufferRect, QBrush(m_defaultColor));
        }
    }

    //列表正在被拖拽且当前节点不是置顶且拖拽的地方不是置顶区内部
    if (dynamic_cast<myListView *>(m_view)->isDragging() && !isPinned
        && !dynamic_cast<myListView *>(m_view)->isDraggingInsidePinned())
    {
        //若当前节点是置顶的第一个笔记且当前列表只有一个笔记
        if (model && model->isFirstUnpinnedNote(index)
            && (index.row() == (model->rowCount() - 1)))
        {
            auto rect = bufferRect;
            rect.setHeight(4);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setWidth(3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setLeft(rect.right() - 3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setTop(rect.bottom() - 3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
        }
        //当前节点是第一个置顶的笔记，但还存在其他笔记
        else if (model && model->isFirstUnpinnedNote(index))
        {
            auto rect = bufferRect;
            rect.setHeight(4);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setWidth(3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setLeft(rect.right() - 3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
        }
        //不是置顶且只有这一个节点
        else if (model && (index.row() == (model->rowCount() - 1)))
        {
            auto rect = bufferRect;
            rect.setTop(rect.bottom() - 3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setWidth(3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setLeft(rect.right() - 3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
        }
        //正常情况
        else
        {
            auto rect = bufferRect;
            rect.setWidth(3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setLeft(rect.right() - 3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
        }
    }

    //需要绘制分割线
    if (model && shouldPaintSeparator(index, *model))
    {
        paintSeparator(&bufferPainter, bufferRect, index);
    }


    int rowHeight;
    if (m_animatedIndexes.contains(index))
    {
        if (m_state != NoteListState::MoveIn)
        {
            double rowRate = m_timeLine->currentFrame() / (m_maxFrame * 1.0);
            rowHeight = bufferSize.height() * rowRate;
        }
        else
        {
            double rowRate = 1.0 - m_timeLine->currentFrame() / (m_maxFrame * 1.0);
            rowHeight = bufferSize.height() * rowRate;
        }
    }
    else
    {
        rowHeight = option.rect.height();
    }
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    //当前节点在索引列表中
    if (m_animatedIndexes.contains(index))
    {
        //当前状态是移入状态
        if (m_state == NoteListState::MoveIn)
        {
            //当前笔记是置顶区或普通区的第一个笔记，则y向下移动25个像素，用于绘制分割线
            if (model && model->hasPinnedNote()
                && (model->isFirstPinnedNote(index) || model->isFirstUnpinnedNote(index)))
            {
                painter->drawPixmap(QRect{ option.rect.x(),
                                          option.rect.y() + bufferSize.height() - rowHeight + 25,
                                          option.rect.width(), rowHeight },
                                    buffer,
                                    QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(),
                                          rowHeight });
            }
            else
            {
                painter->drawPixmap(QRect{ option.rect.x(),
                                          option.rect.y() + bufferSize.height() - rowHeight,
                                          option.rect.width(), rowHeight },
                                    buffer,
                                    QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(),
                                          rowHeight });
            }
        }

        //不是移入状态
        else
        {
            //当前笔记是置顶区或普通区的第一个笔记，则y向下移动25个像素，用于绘制分割线
            if (model && model->hasPinnedNote()
                && (model->isFirstPinnedNote(index) || model->isFirstUnpinnedNote(index)))
            {
                painter->drawPixmap(QRect{ option.rect.x(), option.rect.y() + 25,
                                          option.rect.width(), option.rect.height() },
                                    buffer,
                                    QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(),
                                          rowHeight });
            }
            else
            {
                painter->drawPixmap(option.rect, buffer,
                                    QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(),
                                          rowHeight });
            }
        }
    }

    //当前节点不在索引列表中
    else
    {
        painter->drawPixmap(
            option.rect, buffer,
            QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(), rowHeight });
    }


}

void NoteListDelegate::paintLabels(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    //当前节点在索引列表中
    if (m_animatedIndexes.contains(index))
    {
        //设置要绘制的矩形的参数，填充，抗锯齿等
        auto bufferSize = bufferSizeHint(option, index); //获取矩形的范围
        QPixmap buffer{ bufferSize };
        buffer.fill(Qt::transparent);
        QPainter bufferPainter{ &buffer };
        bufferPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        //标题
        QString title{ index.data(NoteListModel::NoteFullTitle).toString() };
        QFont titleFont = (option.state & QStyle::State_Selected) == QStyle::State_Selected
                              ? m_titleSelectedFont
                              : m_titleFont;
        //设置应显示的长度
        QFontMetrics fmTitle(titleFont);
        QRect fmRectTitle = fmTitle.boundingRect(title);

        //日期
        QString date =
            parseDateTime(index.data(NoteListModel::NoteLastModificationDateTime).toDateTime());
        QFontMetrics fmDate(m_dateFont);
        QRect fmRectDate = fmDate.boundingRect(date);

        //父节点的名字
        QString parentName{ index.data(NoteListModel::NoteParentName).toString() };
        QFontMetrics fmParentName(titleFont);
        QRect fmRectParentName = fmParentName.boundingRect(parentName);

        //显示的简介内容
        QString content{ index.data(NoteListModel::NoteContent).toString() };
        content = NoteEditorLogic::getSecondLine(content);  //获取第二行内容
        QFontMetrics fmContent(titleFont);
        QRect fmRectContent = fmContent.boundingRect(content);



        //要用到的数据
        double rowPosX = 0; // option.rect.x();
        double rowPosY = 0; // option.rect.y();
        double rowWidth = option.rect.width();
        auto model = dynamic_cast<NoteListModel *>(m_view->model());


        //如果置顶区折叠且当前笔记置顶，不用画，直接退出
        if (m_view->isPinnedNotesCollapsed())
        {
            auto isPinned = index.data(NoteListModel::NoteIsPinned).value<bool>();
            if (isPinned)
            {
                return;
            }
        }



        //各种根据情况要添加的高度
        int secondYOffset = 0;
        if (index.row() > 0)
        {
            secondYOffset = NoteListConstant::nextNoteOffset;
        }
        int thirdYOffset = 0;
        if (model && model->isFirstPinnedNote(index))
        {
            thirdYOffset = NoteListConstant::pinnedHeaderToNoteSpace;
        }
        int fourthYOffset = 0;
        if (model && model->isFirstUnpinnedNote(index))
        {
            fourthYOffset = NoteListConstant::unpinnedHeaderToNoteSpace;
        }

        int fifthYOffset = 0;
        if (model && model->hasPinnedNote() && !m_view->isPinnedNotesCollapsed()
            && model->isFirstUnpinnedNote(index))
        {
            fifthYOffset = NoteListConstant::lastPinnedToUnpinnedHeader;
        }
        //之前计算的之和
        int yOffsets = secondYOffset + thirdYOffset + fourthYOffset + fifthYOffset;


        //标题的矩形范围
        double titleRectPosX = rowPosX + NoteListConstant::leftOffsetX;
        double titleRectPosY = rowPosY;
        double titleRectWidth = rowWidth - 2.0 * NoteListConstant::leftOffsetX;
        double titleRectHeight = fmRectTitle.height() + NoteListConstant::topOffsetY + yOffsets;

        //日期的矩形范围
        double dateRectPosX = rowPosX + NoteListConstant::leftOffsetX;
        double dateRectPosY =
            rowPosY + fmRectTitle.height() + NoteListConstant::topOffsetY + yOffsets;
        double dateRectWidth = rowWidth - 2.0 * NoteListConstant::leftOffsetX;
        double dateRectHeight = fmRectDate.height() + NoteListConstant::titleDateSpace;

        //简介内容的矩形范围
        double contentRectPosX = rowPosX + NoteListConstant::leftOffsetX;
        double contentRectPosY = rowPosY + fmRectTitle.height() + fmRectDate.height()
                                 + NoteListConstant::topOffsetY + yOffsets;
        double contentRectWidth = rowWidth - 2.0 * NoteListConstant::leftOffsetX;
        double contentRectHeight = fmRectContent.height() + NoteListConstant::dateDescSpace;

        //文件夹名字的的矩形范围
        double folderNameRectPosX = 0;
        double folderNameRectPosY = 0;
        double folderNameRectWidth = 0;
        double folderNameRectHeight = 0;

        //如果当前列表是来自allnots，添加文件夹矩形
        if (m_isInAllNotes)
        {
            folderNameRectPosX = rowPosX + NoteListConstant::leftOffsetX + 20;
            folderNameRectPosY = rowPosY + fmRectContent.height() + fmRectTitle.height()
                                 + fmRectDate.height() + NoteListConstant::topOffsetY + yOffsets;
            folderNameRectWidth = rowWidth - 2.0 * NoteListConstant::leftOffsetX;
            folderNameRectHeight = fmRectParentName.height() + NoteListConstant::descFolderSpace;
        }

        //定义一个绘制函数
        auto drawStr = [&bufferPainter](double posX, double posY, double width, double height,
                                        QColor color, const QFont &font, const QString &str) {
            QRectF rect(posX, posY, width, height);
            bufferPainter.setPen(color);
            bufferPainter.setFont(font);
            bufferPainter.drawText(rect, Qt::AlignBottom, str);
        };


        //给定的文本缩略处理
        title = fmTitle.elidedText(title, Qt::ElideRight, int(titleRectWidth));
        content = fmContent.elidedText(content, Qt::ElideRight, int(titleRectWidth));
        //绘制标题与日期
        drawStr(titleRectPosX, titleRectPosY, titleRectWidth, titleRectHeight, m_titleColor,
                titleFont, title);
        drawStr(dateRectPosX, dateRectPosY, dateRectWidth, dateRectHeight, m_dateColor, m_dateFont,
                date);

        //如果是在allnotes文件夹下
        if (m_isInAllNotes)
        {
            //还要绘制图标
            bufferPainter.drawImage(QRect(rowPosX + NoteListConstant::leftOffsetX,
                                          folderNameRectPosY + NoteListConstant::descFolderSpace,
                                          16, 16),
                                    m_folderIcon);
            //绘制所属的文件夹名字
            drawStr(folderNameRectPosX, folderNameRectPosY, folderNameRectWidth,
                    folderNameRectHeight, m_contentColor, titleFont, parentName);
        }


        //绘制内容
        drawStr(contentRectPosX, contentRectPosY, contentRectWidth, contentRectHeight,
                m_contentColor, titleFont, content);
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        int rowHeight;

        //如果当前节点在索引列表中
        if (m_animatedIndexes.contains(index))
        {
            //不是移入状态，设置不同的比例
            if (m_state != NoteListState::MoveIn)
            {
                double rowRate = m_timeLine->currentFrame() / (m_maxFrame * 1.0);
                rowHeight = bufferSize.height() * rowRate;
            }
            else
            {
                double rowRate = 1.0 - m_timeLine->currentFrame() / (m_maxFrame * 1.0);
                rowHeight = bufferSize.height() * rowRate;
            }
        }
        else
        {
            rowHeight = option.rect.height();
        }

        //是移入状态
        if (m_state == NoteListState::MoveIn)
        {
            //当前节点是置顶区或普通区的第一个笔记，绘制矩形范围
            if (model && model->hasPinnedNote()
                && (model->isFirstPinnedNote(index) || model->isFirstUnpinnedNote(index)))
            {
                painter->drawPixmap(QRect{ option.rect.x(),
                                          option.rect.y() + bufferSize.height() - rowHeight + 25,
                                          option.rect.width(), rowHeight },
                                    buffer,
                                    QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(),
                                          rowHeight });
            }
            else
            {
                painter->drawPixmap(QRect{ option.rect.x(),
                                          option.rect.y() + bufferSize.height() - rowHeight,
                                          option.rect.width(), rowHeight },
                                    buffer,
                                    QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(),
                                          rowHeight });
            }
        }

        else
        {
            if (model && model->hasPinnedNote()
                && (model->isFirstPinnedNote(index) || model->isFirstUnpinnedNote(index)))
            {
                painter->drawPixmap(QRect{ option.rect.x(), option.rect.y() + 25,
                                          option.rect.width(), rowHeight },
                                    buffer,
                                    QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(),
                                          rowHeight });
            }
            else
            {
                painter->drawPixmap(option.rect, buffer,
                                    QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(),
                                          rowHeight });
            }
        }

        //模型有置顶的笔记，要设置抬头的矩形绘制位置
        if (model && model->hasPinnedNote())
        {
            //当前节点是第一个笔记
            if (model->isFirstPinnedNote(index))
            {
                QRect headerRect(option.rect.x() + NoteListConstant::leftOffsetX / 2,
                                 option.rect.y(),
                                 option.rect.width() - NoteListConstant::leftOffsetX / 2, 25);
#ifdef __APPLE__
                int iconPointSizeOffset = 0;
#else
                int iconPointSizeOffset = -4;
#endif

                //设置展开折叠图标
//                painter->setFont(FontLoader::getInstance().loadFont("Font Awesome 6 Free Solid", "",
//                                                                    14 + iconPointSizeOffset));
                painter->setFont(FontLoader::getInstance().getFont());
                painter->setPen(QColor(68, 138, 201));
                if (m_view->isPinnedNotesCollapsed())
                {
                    painter->drawText(QRect(headerRect.right() - 25, headerRect.y() + 5, 16, 16),
                                      u8"\uf054"); // fa-chevron-right u8"\U000002C5  u8"\uf054"
                }
                else
                {
                    painter->drawText(QRect(headerRect.right() - 25, headerRect.y() + 5, 16, 16),
                                      u8"\uf078"); // fa-chevron-down  u8"\uf078"
                }
                painter->setPen(m_contentColor);
                painter->setFont(m_headerFont);
                painter->drawText(headerRect, Qt::AlignLeft | Qt::AlignVCenter, "Pinned");
            }
            //有置顶的笔记且当前节点是第一个普通区的笔记
            else if (model->hasPinnedNote() && model->isFirstUnpinnedNote(index))
            {
                //设置抬头区域
                QRect headerRect(option.rect.x() + NoteListConstant::leftOffsetX / 2,
                                 option.rect.y() + fifthYOffset,
                                 option.rect.width() - NoteListConstant::leftOffsetX / 2, 25);
                painter->setPen(m_contentColor);
                painter->setFont(m_headerFont);
                painter->drawText(headerRect, Qt::AlignLeft | Qt::AlignVCenter, "Notes");
            }
        }
    }

    //当前节点不在索引列表中
    else
    {
        //标题
        QString title{ index.data(NoteListModel::NoteFullTitle).toString() };
        QFont titleFont =
            m_view->selectionModel()->isSelected(index) ? m_titleSelectedFont : m_titleFont;
        QFontMetrics fmTitle(titleFont);
        QRect fmRectTitle = fmTitle.boundingRect(title);

        //日期
        QString date =
            parseDateTime(index.data(NoteListModel::NoteLastModificationDateTime).toDateTime());
        QFontMetrics fmDate(m_dateFont);
        QRect fmRectDate = fmDate.boundingRect(date);

        //父节点名
        QString parentName{ index.data(NoteListModel::NoteParentName).toString() };
        QFontMetrics fmParentName(titleFont);
        QRect fmRectParentName = fmParentName.boundingRect(parentName);

        //简介内容
        QString content{ index.data(NoteListModel::NoteContent).toString() };
        content = NoteEditorLogic::getSecondLine(content);
        QFontMetrics fmContent(titleFont);
        QRect fmRectContent = fmContent.boundingRect(content);

        double rowPosX = option.rect.x();
        double rowPosY = option.rect.y();
        auto model = dynamic_cast<NoteListModel *>(m_view->model());
        int fifthYOffset = 0;

        //置顶区未折叠，当前节点是第一个普通区的节点
        if (model && model->hasPinnedNote() && !m_view->isPinnedNotesCollapsed()
            && model->isFirstUnpinnedNote(index))
        {
            fifthYOffset = NoteListConstant::lastPinnedToUnpinnedHeader;
        }
        if (model)
        {
            //当前节点是第一个指定的笔记，添加抬头绘制区域
            if (model->isFirstPinnedNote(index))
            {
                QRect headerRect(rowPosX + NoteListConstant::leftOffsetX / 2, rowPosY,
                                 option.rect.width() - NoteListConstant::leftOffsetX / 2, 25);
#ifdef __APPLE__
                int iconPointSizeOffset = 0;
#else
                int iconPointSizeOffset = -4;
#endif


//                painter->setFont(FontLoader::getInstance().loadFont("Font Awesome 6 Free Solid", "",
//                                                                    14 + iconPointSizeOffset));
                painter->setFont(FontLoader::getInstance().getFont());
                painter->setPen(QColor(68, 138, 201));

                //折叠展开图标
                if (m_view->isPinnedNotesCollapsed())
                {
                    painter->drawText(QRect(headerRect.right() - 25, headerRect.y() + 5, 16, 16),
                                      u8"\uf054"); // fa-chevron-right
                }
                else
                {
                    painter->drawText(QRect(headerRect.right() - 25, headerRect.y() + 5, 16, 16),
                                      u8"\uf078"); // fa-chevron-down
                }
                //置顶抬头的标题
                painter->setPen(m_contentColor);
                painter->setFont(m_headerFont);
                painter->drawText(headerRect, Qt::AlignLeft | Qt::AlignVCenter, "Pinned");
                rowPosY += 25;
            }

            //有置顶笔记且当前节点是第一个普通区笔记
            else if (model->hasPinnedNote() && model->isFirstUnpinnedNote(index))
            {
                //绘制普通区的抬头位置
                rowPosY += fifthYOffset;
                QRect headerRect(rowPosX + NoteListConstant::leftOffsetX / 2, rowPosY,
                                 option.rect.width() - NoteListConstant::leftOffsetX / 2, 25);
                painter->setPen(m_contentColor);
                painter->setFont(m_headerFont);
                painter->drawText(headerRect, Qt::AlignLeft | Qt::AlignVCenter, "Notes");
                rowPosY += 25;
            }
        }


        //置顶区折叠且当前笔记是置顶的，直接退出
        if (m_view->isPinnedNotesCollapsed())
        {
            auto isPinned = index.data(NoteListModel::NoteIsPinned).value<bool>();
            if (isPinned)
            {
                return;
            }
        }


        //不同情况偏移量
        double rowWidth = option.rect.width();
        int secondYOffset = 0;
        if (index.row() > 0)
        {
            secondYOffset = NoteListConstant::nextNoteOffset;
        }
        int thirdYOffset = 0;
        if (model && model->isFirstPinnedNote(index))
        {
            thirdYOffset = NoteListConstant::pinnedHeaderToNoteSpace;
        }
        int fourthYOffset = 0;
        if (model && model->isFirstUnpinnedNote(index))
        {
            fourthYOffset = NoteListConstant::unpinnedHeaderToNoteSpace;
        }

        int yOffsets = secondYOffset + thirdYOffset + fourthYOffset;


        //不同内容的绘制的位置
        double titleRectPosX = rowPosX + NoteListConstant::leftOffsetX;
        double titleRectPosY = rowPosY;
        double titleRectWidth = rowWidth - 2.0 * NoteListConstant::leftOffsetX;
        double titleRectHeight = fmRectTitle.height() + NoteListConstant::topOffsetY + yOffsets;

        double dateRectPosX = rowPosX + NoteListConstant::leftOffsetX;
        double dateRectPosY =
            rowPosY + fmRectTitle.height() + NoteListConstant::topOffsetY + yOffsets;
        double dateRectWidth = rowWidth - 2.0 * NoteListConstant::leftOffsetX;
        double dateRectHeight = fmRectDate.height() + NoteListConstant::titleDateSpace;

        double contentRectPosX = rowPosX + NoteListConstant::leftOffsetX;
        double contentRectPosY = rowPosY + fmRectTitle.height() + fmRectDate.height()
                                 + NoteListConstant::topOffsetY + yOffsets;
        double contentRectWidth = rowWidth - 2.0 * NoteListConstant::leftOffsetX;
        double contentRectHeight = fmRectContent.height() + NoteListConstant::dateDescSpace;

        double folderNameRectPosX = 0;
        double folderNameRectPosY = 0;
        double folderNameRectWidth = 0;
        double folderNameRectHeight = 0;

        //父节点是allnotes，绘制文件夹区域
        if (isInAllNotes())
        {
            folderNameRectPosX = rowPosX + NoteListConstant::leftOffsetX + 20;
            folderNameRectPosY = rowPosY + fmRectContent.height() + fmRectTitle.height()
                                 + fmRectDate.height() + NoteListConstant::topOffsetY + yOffsets;
            folderNameRectWidth = rowWidth - 2.0 * NoteListConstant::leftOffsetX;
            folderNameRectHeight = fmRectParentName.height() + NoteListConstant::descFolderSpace;
        }
        auto drawStr = [painter](double posX, double posY, double width, double height,
                                 QColor color, const QFont &font, const QString &str) {
            QRectF rect(posX, posY, width, height);
            painter->setPen(color);
            painter->setFont(font);
            painter->drawText(rect, Qt::AlignBottom, str);
        };


        //真正的绘制
        // draw title & date
        title = fmTitle.elidedText(title, Qt::ElideRight, int(titleRectWidth));
        content = fmContent.elidedText(content, Qt::ElideRight, int(titleRectWidth));
        drawStr(titleRectPosX, titleRectPosY, titleRectWidth, titleRectHeight, m_titleColor,
                titleFont, title);
        drawStr(dateRectPosX, dateRectPosY, dateRectWidth, dateRectHeight, m_dateColor, m_dateFont,
                date);

        if (isInAllNotes())
        {
            painter->drawImage(QRect(rowPosX + NoteListConstant::leftOffsetX,
                                     folderNameRectPosY + NoteListConstant::descFolderSpace, 16,
                                     16),
                               m_folderIcon);
            drawStr(folderNameRectPosX, folderNameRectPosY, folderNameRectWidth,
                    folderNameRectHeight, m_contentColor, titleFont, parentName);
        }
        drawStr(contentRectPosX, contentRectPosY, contentRectWidth, contentRectHeight,
                m_contentColor, titleFont, content);
    }


}

void NoteListDelegate::paintSeparator(QPainter *painter, QRect rect, const QModelIndex &index) const
{
    Q_UNUSED(index);

    painter->setPen(QPen(m_separatorColor));
    const int leftOffsetX = NoteListConstant::leftOffsetX;
    int posX1 = rect.x() + leftOffsetX;
    int posX2 = rect.x() + rect.width() - leftOffsetX - 1;
    int posY = rect.y() + rect.height();

    painter->drawLine(QPoint(posX1, posY), QPoint(posX2, posY));
}

QString NoteListDelegate::parseDateTime(const QDateTime &dateTime) const
{
    QLocale usLocale(QLocale("en_US"));  //设置本地位置

    auto currDateTime = QDateTime::currentDateTime(); //当前时间


    //根据要绘制的日期与当前日期相比较，选择绘制文本内容

    if (dateTime.date() == currDateTime.date())     //就是现在
    {
        return usLocale.toString(dateTime.time(), "h:mm A");
    }
    else if (dateTime.daysTo(currDateTime) == 1)    //相隔一天
    {
        return "Yesterday";
    }
    else if (dateTime.daysTo(currDateTime) >= 2 && dateTime.daysTo(currDateTime) <= 7)//相隔一周内
    {
        return usLocale.toString(dateTime.date(), "dddd");
    }

    //更久远的就绘制月日年
    return dateTime.date().toString("M/d/yy");
}

void NoteListDelegate::updateSizeMap(int id, QSize sz, const QModelIndex &index)
{
    //更新某个节点的大小
    szMap[id] = sz;
    emit sizeHintChanged(index);
}

void NoteListDelegate::editorDestroyed(int id, const QModelIndex &index)
{
    //移除编辑器
    szMap.remove(id);
    emit sizeHintChanged(index);
}

void NoteListDelegate::setHoveredIndex(const QModelIndex &hoveredIndex)
{
    m_hoveredIndex = hoveredIndex;
}

void NoteListDelegate::setRowRightOffset(int rowRightOffset)
{
    m_rowRightOffset = rowRightOffset;
}

QTimeLine::State NoteListDelegate::animationState()
{
    return m_timeLine->state();
}

void NoteListDelegate::setAnimationDuration(const int duration)
{
    m_timeLine->setDuration(duration);
}

void NoteListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //当前节点不是活动节点
    if (!m_animatedIndexes.contains(index))
    {
        return;
    }

    //置顶笔记是折叠的并且数据模型中的这个节点也是第一个置顶笔记
    if (m_view->isPinnedNotesCollapsed())
    {
        auto model = dynamic_cast<NoteListModel *>(m_view->model());
        auto isPinned = index.data(NoteListModel::NoteIsPinned).value<bool>();
        if (isPinned)
        {
            if (model && (!model->isFirstPinnedNote(index)))
            {
                return;
            }
        }
    }

    painter->setRenderHint(QPainter::Antialiasing);  //抗锯齿


    //设置绘制区域的右侧编译
    QStyleOptionViewItem opt = option;
    opt.rect.setWidth(option.rect.width() - m_rowRightOffset);

    int currentFrame = m_timeLine->currentFrame();
    double rate = (currentFrame / (m_maxFrame * 1.0));
    double height = m_rowHeight * rate;


    //分析当前列表的状态
    switch (m_state)
    {
        //插入，移出，移出，设置背景颜色
    case NoteListState::Insert:
    case NoteListState::Remove:
    case NoteListState::MoveOut:
        if (m_animatedIndexes.contains(index))
        {
            opt.rect.setHeight(int(height));
            opt.backgroundBrush.setColor(m_notActiveColor);
        }
        break;
    case NoteListState::MoveIn:
        break;
    case NoteListState::Normal:
        break;
    }


    //绘制背景、主体
    paintBackground(painter, opt, index);
    paintLabels(painter, option, index);
}

QSize NoteListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    QSize result; // = QStyledItemDelegate::sizeHint(option, index);
    result.setWidth(option.rect.width());
    //获取当前索引对应的笔记
    auto model = dynamic_cast<NoteListModel *>(m_view->model());
    const auto &note = model->getNote(index);
    auto id = note.id();


//节点列表中不包含当前节点，若map中有，就设置它的高度
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    if ((!m_animatedIndexes.contains(index)))
    {
#else
    if (m_view->isPersistentEditorOpen(index) && (!m_animatedIndexes.contains(index)))
    {
#endif


        if (szMap.contains(id))
        {
            result.setHeight(szMap[id].height());
            return result;
        }
    }
    int rowHeight = 70;
    //rowHeight = m_rowHeight;  //如果有tag就加，此项目没有


    //若节点列表包含当前节点
    if (m_animatedIndexes.contains(index))
    {
        //移入状态
        if (m_state == NoteListState::MoveIn)
        {
            result.setHeight(rowHeight);
        }
        //其他状态，设置一个比例
        else
        {
            double rate = m_timeLine->currentFrame() / (m_maxFrame * 1.0);
            double height = rowHeight * rate;
            result.setHeight(int(height));
        }
    }
    else
    {
        result.setHeight(rowHeight);
    }


    //当前列表式来自allnotes文件夹
    if (m_isInAllNotes)
    {
        result.setHeight(result.height() + 20);
    }

    //置顶区折叠了
    if (m_view->isPinnedNotesCollapsed())
    {
        //根据当前笔记是否折叠，设置高度
        auto isPinned = note.isPinnedNote();
        if (isPinned)
        {
            if (model && model->isFirstPinnedNote(index))
            {
                result.setHeight(25);
                return result;
            }
            else
            {
                result.setHeight(0);
                return result;
            }
        }
        //当前笔记没有折叠，但此笔记是第一个普通区的笔记
        else if (model->hasPinnedNote() && model->isFirstUnpinnedNote(index))
        {
            result.setHeight(result.height() + 25);  //因为要绘制Notes分割线
        }
    }
    //置顶区未折叠
    else
    {
        //模型有置顶的笔记，并且当前节点是置顶区或者普通区的第一个笔记
        if (model->hasPinnedNote()
            && (model->isFirstPinnedNote(index) || model->isFirstUnpinnedNote(index)))
        {
            result.setHeight(result.height() + 25); //也要+25，设置为分隔符的高度
        }
    }


    //所有可能的偏移量相加
    int secondYOffset = 0;
    if (index.row() > 0)
    {
        secondYOffset = NoteListConstant::nextNoteOffset;               //和下个笔记的间隔
    }
    int thirdYOffset = 0;
    if (model && model->isFirstPinnedNote(index))
    {
        thirdYOffset = NoteListConstant::pinnedHeaderToNoteSpace;       //置顶的抬头到第一个置顶笔记的间隔
    }
    int fourthYOffset = 0;
    if (model && model->isFirstUnpinnedNote(index))
    {
        fourthYOffset = NoteListConstant::unpinnedHeaderToNoteSpace;    //普通区的抬头到第一个普通笔记的间隔
    }
    int fifthYOffset = 0;
    //有置顶笔记且置顶区未折叠且当前节点是第一个普通区的笔记
    if (model && model->hasPinnedNote() && !m_view->isPinnedNotesCollapsed()
        && model->isFirstUnpinnedNote(index))
    {
        fifthYOffset = NoteListConstant::lastPinnedToUnpinnedHeader;    //置顶区的末尾到非指定抬头的间隔
    }

    int yOffsets = secondYOffset + thirdYOffset + fourthYOffset + fifthYOffset;

    //如果是来自allnotes文件夹,调整高度
    if (m_isInAllNotes)
    {
        result.setHeight(result.height() - 2 + NoteListConstant::lastElSepSpace + yOffsets);
    }
    else
    {
        result.setHeight(result.height() - 10 + NoteListConstant::lastElSepSpace + yOffsets);
    }


    return result;
}

QWidget *NoteListDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return nullptr;
    }

    //置顶区折叠且当前节点是折叠状态并且不是第一个置顶节点就退出
    auto model = dynamic_cast<NoteListModel *>(m_view->model());
    if (m_view->isPinnedNotesCollapsed())
    {
        auto isPinned = index.data(NoteListModel::NoteIsPinned).value<bool>();
        if (isPinned)
        {
            if (model && (!model->isFirstPinnedNote(index)))
            {
                return nullptr;
            }
        }
    }

    //获取编辑器
    auto w = new NoteListDelegateEditor(this, m_view, option, index, parent);
    w->setTheme(m_theme);

    //设置相关参数后返回这个widget
    connect(this, &NoteListDelegate::themeChanged, w, &NoteListDelegateEditor::setTheme);
    connect(w, &NoteListDelegateEditor::updateSizeHint, this, &NoteListDelegate::updateSizeMap);
    connect(w, &NoteListDelegateEditor::nearDestroyed, this, &NoteListDelegate::editorDestroyed);
    w->recalculateSize();

    return w;
}

QSize NoteListDelegate::bufferSizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize result = QStyledItemDelegate::sizeHint(option, index);
    result.setWidth(option.rect.width());
    auto id = index.data(NoteListModel::NoteID).toInt();
    //bool isHaveTags = index.data(NoteListModel::NoteTagsList).value<QSet<int>>().size() > 0;
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    if (!m_animatedIndexes.contains(index))
    {
#else
    if (m_view->isPersistentEditorOpen(index) && (!m_animatedIndexes.contains(index)))
    {
#endif
        if (szMap.contains(id))
        {
            result.setHeight(szMap[id].height());
            return result;
        }
    }

    int rowHeight = 70;
    //rowHeight = m_rowHeight; //有tag就加


    result.setHeight(rowHeight);

    if (m_isInAllNotes)
    {
        result.setHeight(result.height() + 20);
    }

    auto model = dynamic_cast<NoteListModel *>(m_view->model());
    //    if (model) {
    //        if (model->hasPinnedNote() &&
    //                (model->isFirstPinnedNote(index) || model->isFirstUnpinnedNote(index))) {
    //            result.setHeight(result.height() + 25);
    //        }
    //    }


    if (m_view->isPinnedNotesCollapsed())
    {
        auto isPinned = index.data(NoteListModel::NoteIsPinned).value<bool>();
        if (isPinned)
        {
            if (model && model->isFirstPinnedNote(index))
            {
                result.setHeight(25);
                return result;
            }
            else
            {
                result.setHeight(0);
                return result;
            }
        }
    }


    int secondYOffset = 0;
    if (index.row() > 0)
    {
        secondYOffset = NoteListConstant::nextNoteOffset;
    }
    int thirdYOffset = 0;
    if (model && model->isFirstPinnedNote(index))
    {
        thirdYOffset = NoteListConstant::pinnedHeaderToNoteSpace;
    }
    int fourthYOffset = 0;
    if (model && model->isFirstUnpinnedNote(index))
    {
        fourthYOffset = NoteListConstant::unpinnedHeaderToNoteSpace;
    }
    //    int fifthYOffset = 0;
    //    if (model && model->hasPinnedNote() && !m_view->isPinnedNotesCollapsed()
    //            && model->isFirstUnpinnedNote(index))
    //{
    //        fifthYOffset = NoteListConstant::lastPinnedToUnpinnedHeader;
    //    }


    int yOffsets = secondYOffset + thirdYOffset + fourthYOffset; // + fifthYOffset;

    if (m_isInAllNotes)
    {
        result.setHeight(result.height() - 2 + NoteListConstant::lastElSepSpace + yOffsets);
    }
    else
    {
        result.setHeight(result.height() - 10 + NoteListConstant::lastElSepSpace + yOffsets);
    }

    return result;
}
