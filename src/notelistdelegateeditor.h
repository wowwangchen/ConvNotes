#ifndef NOTELISTDELEGATEEDITOR_H
#define NOTELISTDELEGATEEDITOR_H

#include <QObject>
#include<QWidget>
#include"notelistdelegate.h"
#include"mylistview.h"


//列表的不同内容的间隔
struct NoteListConstant
{
    static constexpr int leftOffsetX = 20;
    static constexpr int topOffsetY = 7; // space on top of title

    static constexpr int titleDateSpace = 4; // space between title and date
    static constexpr int dateDescSpace = 12; // space between date and description
    static constexpr int descFolderSpace = 7; // space between description and folder name
    static constexpr int lastElSepSpace = 12; // space between the last element and the separator
    static constexpr int nextNoteOffset =
        0; // space between the separator and the next note underneath it
    static constexpr int pinnedHeaderToNoteSpace =
        0; // space between Pinned label to the pinned list
    static constexpr int unpinnedHeaderToNoteSpace =
        0; // space between Notes label and the normal notes list
    static constexpr int lastPinnedToUnpinnedHeader =
        5; // space between the last pinned note to Notes label
};


//笔记列表界面的项的代理编辑器
class NoteListDelegateEditor : public QWidget
{
    Q_OBJECT
public:
    explicit NoteListDelegateEditor(const NoteListDelegate *delegate, myListView *view,
                                    const QStyleOptionViewItem &option, const QModelIndex &index,
                                    QWidget *parent = nullptr);
        ~NoteListDelegateEditor();
    void setRowRightOffset(int rowRightOffset);
    void setActive(bool isActive);
    void recalculateSize();
    void setScrollBarPos(int pos);
    int getScrollBarPos();
    bool underMouseC() const;
    QPixmap renderToPixmap();





public slots:
    void setTheme(Theme::Value theme);



signals:
    void updateSizeHint(int id, const QSize &sz, const QModelIndex &index);
    void nearDestroyed(int id, const QModelIndex &index);





private:
    void paintBackground(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const;
    void paintLabels(QPainter *painter, const QStyleOptionViewItem &option,
                     const QModelIndex &index) const;
    void paintSeparator(QPainter *painter, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const;
    QString parseDateTime(const QDateTime &dateTime) const;



protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent *event) override;

    virtual void enterEvent(QEvent *event) override;


    virtual void leaveEvent(QEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;





private:
    const NoteListDelegate *m_delegate;
    QStyleOptionViewItem m_option;
    int m_id;
    myListView *m_view;


    QString m_displayFont;
    QFont m_titleFont;
    QFont m_titleSelectedFont;
    QFont m_dateFont;
    QFont m_headerFont;
    QColor m_titleColor;
    QColor m_dateColor;
    QColor m_contentColor;
    QColor m_ActiveColor;
    QColor m_notActiveColor;
    QColor m_hoverColor;
    QColor m_applicationInactiveColor;
    QColor m_separatorColor;
    QColor m_defaultColor;
    int m_rowHeight;
    int m_rowRightOffset;
    bool m_isActive;
    QImage m_folderIcon;
    Theme::Value m_theme;
    bool m_containsMouse;
    QModelIndex m_animatedIndex;

};

#endif // NOTELISTDELEGATEEDITOR_H
