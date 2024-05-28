#include "notelistdelegate.h"



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
    if (animationState() != QTimeLine::NotRunning)
    {
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
            animationQueue.push_back(qMakePair(ids, NewState));
        }
    }
    else
    {
        setStateI(NewState, indexes);
    }
}

void NoteListDelegate::setStateI(NoteListState NewState, const QModelIndexList &indexes)
{

}

void NoteListDelegate::setActive(bool isActive)
{

}

void NoteListDelegate::setHoveredIndex(const QModelIndex &hoveredIndex)
{

}

void NoteListDelegate::setRowRightOffset(int rowRightOffset)
{

}

QTimeLine::State NoteListDelegate::animationState()
{

}
