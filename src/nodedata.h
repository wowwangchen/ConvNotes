#ifndef NODEDATA_H
#define NODEDATA_H

#include<QObject>
#include<QDateTime>
#include<QDataStream>
#include<QSet>

//特殊的节点的ID
namespace SpecialNodeID
{
enum Value {
    InvalidNodeId = -1,
    RootFolder = 0,
    TrashFolder = 1,
    DefaultNotesFolder = 2,
};
}


//包含一个节点项的所有数据
class NodeData
{
public:
    enum Type { Note = 0, Folder };
    NodeData();
    ~NodeData();
    int id() const;
    void setId(int id);

    QString fullTitle() const;
    void setFullTitle(const QString &fullTitle);

    QDateTime lastModificationdateTime() const;
    void setLastModificationDateTime(const QDateTime &lastModificationdateTime);

    QDateTime creationDateTime() const;
    void setCreationDateTime(const QDateTime &creationDateTime);

    QDateTime deletionDateTime() const;
    void setDeletionDateTime(const QDateTime &deletionDateTime);

    QString content() const;
    void setContent(const QString &content);

    bool isModified() const;
    void setModified(bool isModified);

    bool isSelected() const;
    void setSelected(bool isSelected);

    int scrollBarPosition() const;
    void setScrollBarPosition(int scrollBarPosition);


    NodeData::Type nodeType() const;
    void setNodeType(NodeData::Type newNodeType);

    int parentId() const;
    void setParentId(int newParentId);

    int relativePosition() const;
    void setRelativePosition(int newRelativePosition);

    const QString &absolutePath() const;
    void setAbsolutePath(const QString &newAbsolutePath);

    //const QSet<int> &tagIds() const;
    //void setTagIds(const QSet<int> &newTagIds);

    bool isTempNote() const;
    void setIsTempNote(bool newIsTempNote);

    const QString &parentName() const;
    void setParentName(const QString &newParentName);

    bool isPinnedNote() const;
    void setIsPinnedNote(bool newIsPinnedNote);

    //int tagListScrollBarPos() const;
    //void setTagListScrollBarPos(int newTagListScrollBarPos);

    int relativePosAN() const;
    void setRelativePosAN(int newRelativePosAN);

    int childNotesCount() const;
    void setChildNotesCount(int newChildCount);

private:
    int             m_id;                           //节点id
    QString         m_fullTitle;                    //全称
    QDateTime       m_lastModificationDateTime;    //上次修改日期
    QDateTime       m_creationDateTime;             //创造日期
    QDateTime       m_deletionDateTime;             //删除日期
    QString         m_content;                      //内容
    bool            m_isModified;                   //是否被修改了
    bool            m_isSelected;                   //是否被选中
    int             m_scrollBarPosition;            //滚轮位置
    NodeData::Type  m_nodeType;                     //节点类型
    int             m_parentId;                     //父节点id
    int             m_relativePosition;             //相对位置
    QString         m_absolutePath;                 //绝对路径
    bool            m_isTempNote;                   //是否为临时笔记
    QString         m_parentName;                   //父节点名称
    bool            m_isPinnedNote;                 //是否为置顶
    //QSet<int>       m_tagIds;
    //int             m_tagListScrollBarPos;
    int             m_relativePosAN;
    int             m_childNotesCount;              //子节点(包含子节点的子节点...)数量
};


//声明自定义类型的元类型，当使用QVarient时可以支持NodeData类型
Q_DECLARE_METATYPE(NodeData)


//用于解析流，将内容正确地赋值给NodeData
QDataStream &operator>>(QDataStream &stream, NodeData &nodeData);
QDataStream &operator>>(QDataStream &stream, NodeData *&nodeData);

#endif // NODEDATA_H
