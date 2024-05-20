#include "dbmanager.h"

DBManager::DBManager(QObject *parent) : QObject(parent)
{

}

NodePath DBManager::getNodeAbsolutePath(int nodeId)
{
    return NodePath("");
}

NodeData DBManager::getNode(int nodeId)
{
    return NodeData{};
}

void DBManager::moveFolderToTrash(const NodeData &node)
{

}

FolderListType DBManager::getFolderList()
{
    return FolderListType{};
}

void DBManager::open(const QString &path, bool doCreate)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", DEFAULT_DATABASE_NAME);
    m_dbpath = path;
    m_db.setDatabaseName(path);
    if (!m_db.open())
    {
        qDebug() << "Error: connection with database fail";
    }
    else
    {
        qDebug() << "Database: connection ok";
    }

    if (doCreate)
    {
        createTables();
    }
    recalculateChildNotesCount();
}

void DBManager::createTables()
{
    //开始事务，原子操作
    m_db.transaction();
    QSqlQuery query(m_db);

    //节点的表
    QString nodeTable = R"(CREATE TABLE "node_table" ()"                            //表名
                        R"(    "id"	INTEGER NOT NULL,)"                             //id
                        R"(    "title"	TEXT,)"                                     //显示标题
                        R"(    "creation_date"	INTEGER NOT NULL DEFAULT 0,)"       //创建日期
                        R"(    "modification_date"	INTEGER NOT NULL DEFAULT 0,)"   //上次修改日期
                        R"(    "deletion_date"	INTEGER NOT NULL DEFAULT 0,)"       //删除的日期
                        R"(    "content"	TEXT,)"                                 //笔记内容
                        R"(    "node_type"	INTEGER NOT NULL,)"                     //节点类型
                        R"(    "parent_id"	INTEGER NOT NULL,)"                     //父节点id
                        R"(    "relative_position"	INTEGER NOT NULL,)"             //相对位置
                        R"(    "scrollbar_position"	INTEGER NOT NULL,)"             //滚动条位置
                        R"(    "absolute_path"	TEXT NOT NULL,)"                    //绝对路径
                        R"(    "is_pinned_note"	INTEGER NOT NULL DEFAULT 0,)"       //是否置顶
                        R"(    "relative_position_an"	INTEGER NOT NULL,)"         //排序中的相对位置
                        R"(    "child_notes_count"	INTEGER NOT NULL)"              //子节点数量
                        R"();)";
    auto status = query.exec(nodeTable);
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();


    //标签的关系的表
    QString tagRelationship = R"(CREATE TABLE "tag_relationship" ()"
                              R"(    "node_id"	INTEGER NOT NULL,)"
                              R"(    "tag_id"	INTEGER NOT NULL,)"
                              R"(    UNIQUE(node_id, tag_id))"
                              R"();)";
    status = query.exec(tagRelationship);
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();


    //标签的表
    QString tagTable = R"(CREATE TABLE "tag_table" ()"
                       R"(    "id"	INTEGER NOT NULL,)"
                       R"(    "name"	TEXT NOT NULL,)"
                       R"(    "color"	TEXT NOT NULL,)"
                       R"(    "child_notes_count"	INTEGER NOT NULL,)"
                       R"(    "relative_position"	INTEGER NOT NULL)"
                       R"();)";
    status = query.exec(tagTable);
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();


    //键值对，元数据
    QString metadata = R"(CREATE TABLE "metadata" ()"
                       R"(    "key"	TEXT NOT NULL,)"
                       R"(    "value"	INTEGER NOT NULL)"
                       R"();)";
    status = query.exec(metadata);
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();


    //插入
    query.prepare(R"(INSERT INTO "metadata"("key","value") VALUES (:key, :value);)");

    //代表查询语句中的占位符, :key  :value
    query.bindValue(":key", "next_node_id");
    query.bindValue(":value", 0);
    status = query.exec();
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }

    query.bindValue(":key", "next_tag_id");
    query.bindValue(":value", 0);
    status = query.exec();
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();


    //默认的根节点，垃圾桶，笔记文件夹
    NodeData rootFolder; // id 0
    rootFolder.setNodeType(NodeData::Folder);           //类型
    QDateTime noteDate = QDateTime::currentDateTime();  //事件
    rootFolder.setCreationDateTime(noteDate);
    rootFolder.setLastModificationDateTime(noteDate);
    rootFolder.setFullTitle(QStringLiteral("/"));       //标题
    rootFolder.setParentId(-1);                         //父节点
    addNode(rootFolder);                                //添加节点

    NodeData trashFolder; // id 1
    trashFolder.setNodeType(NodeData::Folder);
    trashFolder.setCreationDateTime(noteDate);
    trashFolder.setLastModificationDateTime(noteDate);
    trashFolder.setFullTitle(QStringLiteral("Trash"));
    trashFolder.setParentId(0);
    addNode(trashFolder);

    NodeData notesFolder; // id 2
    notesFolder.setNodeType(NodeData::Folder);
    notesFolder.setCreationDateTime(noteDate);
    notesFolder.setLastModificationDateTime(noteDate);
    notesFolder.setFullTitle(QStringLiteral("Notes"));
    notesFolder.setParentId(0);
    addNode(notesFolder);

    //结束事务
    m_db.commit();

}

void DBManager::recalculateChildNotesCount()
{

}

int DBManager::nextAvailableNodeId()
{
    return 0;
}

void DBManager::increaseChildNotesCountFolder(int folderId)
{
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT child_notes_count, absolute_path  FROM "node_table" WHERE id=:id)");
    query.bindValue(QStringLiteral(":id"), folderId);
    bool status = query.exec();
    int childNotesCount = 0;
    QString absPath;
    if (status)
    {
        query.next();
        childNotesCount = query.value(0).toInt();
        absPath = query.value(1).toString();
    }
    else
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        return;
    }
    query.clear();
    childNotesCount += 1;

    query.prepare(QStringLiteral("UPDATE node_table SET child_notes_count = :child_notes_count "
                                 "WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), folderId);
    query.bindValue(QStringLiteral(":child_notes_count"), childNotesCount);
    status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    emit childNotesCountUpdatedFolder(folderId, absPath, childNotesCount);
}

void DBManager::decreaseChildNotesCountFolder(int folderId)
{

}

void DBManager::renameNode(int id, const QString &newName)
{

}

void DBManager::moveNode(int nodeId, const NodeData &target)
{

}

void DBManager::updateRelPosNode(int nodeId, int relPos)
{

}

int DBManager::addNode(const NodeData &node)
{
    QSqlQuery query(m_db);
    QString emptyStr;

    qint64 epochTimeDateCreated = node.creationDateTime().toMSecsSinceEpoch();  //UTC时间
    QString content = node.content().replace("'", "''").replace(QChar('\x0'), emptyStr);//前者替换为后者
    QString fullTitle = node.fullTitle().replace("'", "''").replace(QChar('\x0'), emptyStr);

    qint64 epochTimeDateLastModified = node.lastModificationdateTime().isNull()
                                           ? epochTimeDateCreated
                                           : node.lastModificationdateTime().toMSecsSinceEpoch();

    int relationalPosition = 0;
    //不是根节点
    if (node.parentId() != -1)
    {
        //查询这个节点的相对位置，设置为最大的可用位置

        query.prepare(R"(SELECT relative_position FROM "node_table" )"
                      R"(WHERE parent_id = :parent_id AND node_type = :node_type;)");
        query.bindValue(":parent_id", node.parentId());
        query.bindValue(":node_type", static_cast<int>(node.nodeType()));
        bool status = query.exec();
        if (status) //有结果
        {
            while (query.next())
            {
                if (relationalPosition <= query.value(0).toInt())
                {
                    relationalPosition = query.value(0).toInt() + 1;
                }
            }
        }
        else
        {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.finish();
    }



    int nodeId = nextAvailableNodeId(); //下一个可获得的节点的id
    //获取节点父节点的绝对位置
    QString absolutePath;
    if (node.parentId() != -1)
    {
        absolutePath = getNodeAbsolutePath(node.parentId()).path();
    }
    //得到自己的绝对位置
    absolutePath += PATH_SEPARATOR + QString::number(nodeId);



    //插入一条记录
    QString queryStr =
        R"(INSERT INTO "node_table")"
        R"(("id", "title", "creation_date", "modification_date", "deletion_date", "content", "node_type", "parent_id", "relative_position", "scrollbar_position", "absolute_path", "is_pinned_note", "relative_position_an", "child_notes_count"))"
        R"(VALUES (:id, :title, :creation_date, :modification_date, :deletion_date, :content, :node_type, :parent_id, :relative_position, :scrollbar_position, :absolute_path, :is_pinned_note, :relative_position_an, :child_notes_count);)";

    query.prepare(queryStr); //准备数据库查询
    query.bindValue(":id", nodeId); //绑定查询语句中的的参数
    query.bindValue(":title", fullTitle);
    query.bindValue(":creation_date", epochTimeDateCreated);
    query.bindValue(":modification_date", epochTimeDateLastModified);
    if (node.deletionDateTime().isNull())
    {
        query.bindValue(":deletion_date", -1);
    }
    else
    {
        query.bindValue(":deletion_date", node.deletionDateTime().toMSecsSinceEpoch());
    }
    query.bindValue(":content", content);
    query.bindValue(":node_type", static_cast<int>(node.nodeType()));
    query.bindValue(":parent_id", node.parentId());
    query.bindValue(":relative_position", relationalPosition);
    query.bindValue(":scrollbar_position", node.scrollBarPosition());
    query.bindValue(":absolute_path", absolutePath);
    query.bindValue(":is_pinned_note", node.isPinnedNote() ? 1 : 0);
    query.bindValue(":relative_position_an", node.relativePosAN());
    query.bindValue(":child_notes_count", node.childNotesCount());
    //查询
    if (!query.exec()) //没有结果
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.finish();


    //更新
    query.prepare(R"(UPDATE "metadata" SET "value"=:value WHERE "key"='next_node_id';)");
    query.bindValue(":value", nodeId + 1);
    if (!query.exec())
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }

    //笔记节点，就更新父节点的子节点数
    if (node.nodeType() == NodeData::Note)
    {
        increaseChildNotesCountFolder(node.parentId());
        increaseChildNotesCountFolder(SpecialNodeID::RootFolder);
    }
    return nodeId;
}
