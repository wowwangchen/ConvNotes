#include "dbmanager.h"

DBManager::DBManager(QObject *parent) : QObject(parent)
{

}

NodePath DBManager::getNodeAbsolutePath(int nodeId)
{
    //查询指定id的绝对路径
    QSqlQuery query(m_db);
    query.prepare("SELECT absolute_path FROM node_table WHERE id = :id");
    query.bindValue(":id", nodeId);
    bool status = query.exec();
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError() << query.isValid();
    }

    query.last();  //只返回一条记录的查询，等价于next
    auto absolutePath = query.value(0).toString();

    return absolutePath;
}

NodeData DBManager::getNode(int nodeId)
{
    //查询
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT)"
                  R"("id",)"
                  R"("title",)"
                  R"("creation_date",)"
                  R"("modification_date",)"
                  R"("deletion_date",)"
                  R"("content",)"
                  R"("node_type",)"
                  R"("parent_id",)"
                  R"("relative_position",)"
                  R"("scrollbar_position",)"
                  R"("absolute_path", )"
                  R"("is_pinned_note", )"
                  R"("relative_position_an", )"
                  R"("child_notes_count" )"
                  R"(FROM node_table WHERE id=:id LIMIT 1;)");
    query.bindValue(":id", nodeId);
    bool status = query.exec();

    if (status)
    {
        //设置参数
        query.next();
        NodeData node;
        node.setId(query.value(0).toInt());
        node.setFullTitle(query.value(1).toString());
        node.setCreationDateTime(QDateTime::fromMSecsSinceEpoch(query.value(2).toLongLong()));
        node.setLastModificationDateTime(
            QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong()));
        node.setDeletionDateTime(QDateTime::fromMSecsSinceEpoch(query.value(4).toLongLong()));
        node.setContent(query.value(5).toString());
        node.setNodeType(static_cast<NodeData::Type>(query.value(6).toInt()));
        node.setParentId(query.value(7).toInt());
        node.setRelativePosition(query.value(8).toInt());
        node.setScrollBarPosition(query.value(9).toInt());
        node.setAbsolutePath(query.value(10).toString());
        node.setIsPinnedNote(static_cast<bool>(query.value(11).toInt()));
        node.setRelativePosAN(query.value(13).toInt());
        node.setRelativePosAN(query.value(14).toInt());

        return node;
    }

    else
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    qDebug() << "Can't find node with id" << nodeId;
    return NodeData();
}

void DBManager::moveFolderToTrash(const NodeData &node)
{
    //查询节点对应的数据库中的id，其中路径是包含这个节点的路径即可(即包括子节点)
    QSqlQuery query(m_db);
    QString parentPath = node.absolutePath() + PATH_SEPARATOR;
    query.prepare(R"(SELECT id FROM "node_table" )"
                  R"(WHERE absolute_path like (:path_expr) || '%' AND node_type = (:node_type);)");
    query.bindValue(QStringLiteral(":path_expr"), parentPath);
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Note));
    bool status = query.exec();
    QSet<int> childIds;
    if (status)
    {
        while (query.next())
        {
            childIds.insert(query.value(0).toInt());
        }
    }
    else
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();

    //将节点加入垃圾桶节点
    auto trashFolder = getNode(SpecialNodeID::TrashFolder);
    for (const auto &id : childIds)
    {
        moveNode(id, trashFolder);
    }


    //从节点表中删除节点的子节点
    query.prepare(R"(DELETE FROM "node_table" )"
                  R"(WHERE absolute_path like (:path_expr) || '%' AND node_type = (:node_type);)");
    query.bindValue(QStringLiteral(":path_expr"), parentPath);
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Folder));
    status = query.exec();
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();

    //从表中删除这个节点
    query.prepare(R"(DELETE FROM "node_table" )"
                  R"(WHERE absolute_path like (:path_expr) AND node_type = (:node_type);)");
    query.bindValue(QStringLiteral(":path_expr"), node.absolutePath());
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Folder));
    status = query.exec();
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

FolderListType DBManager::getFolderList()
{

    //获取id和文件夹名称的map并返回
    QMap<int, QString> result;
    QSqlQuery query(m_db);
    query.prepare(
        R"(SELECT "id", "title" FROM node_table WHERE id > 0 AND node_type = :node_type;)");
    query.bindValue(":node_type", NodeData::Folder);
    bool status = query.exec();
    if (status)
    {
        while (query.next())
        {
            result[query.value(0).toInt()] = query.value(1).toString();
        }
    }
    else
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }

    return result;
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
    //查询所有文件件类型的节点的id、绝对路径
    QSqlQuery query;
    bool status;
    QMap<int, QString> folderIds;
    query.prepare(R"(SELECT id, absolute_path )"
                  R"(FROM node_table WHERE node_type=:node_type;)");
    query.bindValue(":node_type", static_cast<int>(NodeData::Type::Folder));
    status = query.exec();
    if (status)
    {
        while (query.next())
        {
            auto id = query.value(0).toInt();
            if (id != SpecialNodeID::RootFolder) //除根节点外
            {
                folderIds[id] = query.value(1).toString();
            }
        }
    }
    else
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }

    //遍历之前查找的所有结果
    for (const auto &id : folderIds.keys())
    {
        //查询当前遍历的id的子笔记的数量
        query.prepare(R"(SELECT count(*) FROM node_table )"
                      R"(WHERE node_type = (:node_type) AND parent_id = (:parent_id);)");
        query.bindValue(QStringLiteral(":parent_id"), id);
        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Note));
        status = query.exec();
        int childNotesCount = 0;
        if (status)
        {
            query.next();
            childNotesCount = query.value(0).toInt();
        }
        else
        {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.clear();

        //更新笔记数量
        query.prepare(QStringLiteral("UPDATE node_table SET child_notes_count = :child_notes_count "
                                     "WHERE id = :id"));
        query.bindValue(QStringLiteral(":id"), id);
        query.bindValue(QStringLiteral(":child_notes_count"), childNotesCount);
        status = query.exec();
        if (!status)
        {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }

        //发送更新文件夹子笔记数量的信号
        emit childNotesCountUpdatedFolder(id, folderIds[id], childNotesCount);
    }


    recalculateChildNotesCountAllNotes();
}

void DBManager::recalculateChildNotesCountAllNotes()
{
    //查询所有不在垃圾桶节点的子节点数量
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT count(*) FROM node_table )"
                  R"(WHERE node_type = (:node_type) AND parent_id != (:parent_id);)");
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Note));
    query.bindValue(QStringLiteral(":parent_id"), static_cast<int>(SpecialNodeID::TrashFolder));
    bool status = query.exec();
    int childNotesCount = 0;
    if (status)
    {
        query.next();
        childNotesCount = query.value(0).toInt();
    }
    else
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();


    //更新到根节点的子节点数量
    query.prepare(QStringLiteral("UPDATE node_table SET child_notes_count = :child_notes_count "
                                 "WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), SpecialNodeID::RootFolder);
    query.bindValue(QStringLiteral(":child_notes_count"), childNotesCount);
    status = query.exec();
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }

    //子节点数量改变的信号
    emit childNotesCountUpdatedFolder(SpecialNodeID::RootFolder,
                                      getNodeAbsolutePath(SpecialNodeID::RootFolder).path(),
                                      childNotesCount);
}

void DBManager::recalculateChildNotesCountFolder(int folderId)
{
    //查询节点类型是笔记，父节点是给定id的节点
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT count(*) FROM node_table )"
                  R"(WHERE node_type = (:node_type) AND parent_id = (:parent_id);)");
    query.bindValue(QStringLiteral(":parent_id"), folderId);
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Note));
    bool status = query.exec();
    int childNotesCount = 0;
    if (status)
    {
        query.next();
        childNotesCount = query.value(0).toInt();
    }
    else
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();


    //更新
    query.prepare(QStringLiteral("UPDATE node_table SET child_notes_count = :child_notes_count "
                                 "WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), folderId);
    query.bindValue(QStringLiteral(":child_notes_count"), childNotesCount);
    status = query.exec();
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }


    //发送更新完成信号
    emit childNotesCountUpdatedFolder(folderId, getNodeAbsolutePath(folderId).path(),
                                      childNotesCount);
}

int DBManager::nextAvailableNodeId()
{
    QSqlQuery query(m_db);

    query.prepare("SELECT value FROM metadata WHERE key = :key");
    query.bindValue(":key", "next_node_id");
    if (!query.exec())
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.last();   //最新的一个节点
    int nodeId = query.value(0).toInt();
    return nodeId;
}

void DBManager::onNotesListInFolderRequested(int parentID, bool isRecursive, bool newNote, int scrollToId)
{
    QVector<NodeData> nodeList;
    QSqlQuery query(m_db);

    //如果父节点是根节点
    if (parentID == SpecialNodeID::RootFolder)
    {
        //查询是笔记节点但父节点不是垃圾桶的节点
        query.prepare(R"(SELECT )"
                      R"("id",)"
                      R"("title",)"
                      R"("creation_date",)"
                      R"("modification_date",)"
                      R"("deletion_date",)"
                      R"("content",)"
                      R"("node_type",)"
                      R"("parent_id",)"
                      R"("relative_position",)"
                      R"("scrollbar_position",)"
                      R"("absolute_path", )"
                      R"("is_pinned_note", )"
                      R"("relative_position_an", )"
                      R"("child_notes_count" )"
                      R"(FROM node_table )"
                      R"(WHERE node_type = (:node_type) AND parent_id != (:parent_id);)");
        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Note));
        query.bindValue(QStringLiteral(":parent_id"), static_cast<int>(SpecialNodeID::TrashFolder));


        bool status = query.exec();
        if (status)
        {
            //遍历所有结果
            while (query.next())
            {
                NodeData node;
                node.setId(query.value(0).toInt());
                node.setFullTitle(query.value(1).toString());
                node.setCreationDateTime(
                    QDateTime::fromMSecsSinceEpoch(query.value(2).toLongLong()));
                node.setLastModificationDateTime(
                    QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong()));
                node.setDeletionDateTime(
                    QDateTime::fromMSecsSinceEpoch(query.value(4).toLongLong()));
                node.setContent(query.value(5).toString());
                node.setNodeType(static_cast<NodeData::Type>(query.value(6).toInt()));
                node.setParentId(query.value(7).toInt());
                node.setRelativePosition(query.value(8).toInt());
                node.setScrollBarPosition(query.value(9).toInt());
                node.setAbsolutePath(query.value(10).toString());
                node.setIsPinnedNote(static_cast<bool>(query.value(11).toInt()));
                node.setRelativePosAN(query.value(13).toInt());
                node.setChildNotesCount(query.value(14).toInt());
                auto p = getNode(node.parentId());
                node.setParentName(p.fullTitle());

                nodeList.append(node);
            }
        }
        else
        {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
    }

    //如果不是递归的
    else if (isRecursive==false)
    {
        //查找所有父节点是指定节点且为笔记类型的节点
        query.prepare(R"(SELECT )"
                      R"("id",)"
                      R"("title",)"
                      R"("creation_date",)"
                      R"("modification_date",)"
                      R"("deletion_date",)"
                      R"("content",)"
                      R"("node_type",)"
                      R"("parent_id",)"
                      R"("relative_position", )"
                      R"("scrollbar_position",)"
                      R"("absolute_path", )"
                      R"("is_pinned_note", )"
                      R"("relative_position_an", )"
                      R"("child_notes_count" )"
                      R"(FROM node_table )"
                      R"(WHERE parent_id = (:parent_id) AND node_type = (:node_type);)");
        query.bindValue(QStringLiteral(":parent_id"), parentID);
        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Note));

        bool status = query.exec();
        if (status)
        {
            //遍历所有结果然后添加
            while (query.next())
            {
                NodeData node;
                node.setId(query.value(0).toInt());
                node.setFullTitle(query.value(1).toString());
                node.setCreationDateTime(
                    QDateTime::fromMSecsSinceEpoch(query.value(2).toLongLong()));
                node.setLastModificationDateTime(
                    QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong()));
                node.setDeletionDateTime(
                    QDateTime::fromMSecsSinceEpoch(query.value(4).toLongLong()));
                node.setContent(query.value(5).toString());
                node.setNodeType(static_cast<NodeData::Type>(query.value(6).toInt()));
                node.setParentId(query.value(7).toInt());
                node.setRelativePosition(query.value(8).toInt());
                node.setScrollBarPosition(query.value(9).toInt());
                node.setAbsolutePath(query.value(10).toString());
                node.setIsPinnedNote(static_cast<bool>(query.value(11).toInt()));
                node.setRelativePosAN(query.value(13).toInt());
                node.setChildNotesCount(query.value(14).toInt());

                nodeList.append(node);
            }
        }
        else
        {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
    }

    //遍历所有子节点
    else
    {
        auto parentPath = getNodeAbsolutePath(parentID).path() + PATH_SEPARATOR;
        query.prepare(
            R"(SELECT )"
            R"("id",)"
            R"("title",)"
            R"("creation_date",)"
            R"("modification_date",)"
            R"("deletion_date",)"
            R"("content",)"
            R"("node_type",)"
            R"("parent_id",)"
            R"("relative_position", )"
            R"("scrollbar_position",)"
            R"("absolute_path", )"
            R"("is_pinned_note", )"
            R"("relative_position_an", )"
            R"("child_notes_count" )"
            R"(FROM node_table )"
            R"(WHERE absolute_path like (:path_expr) || '%' AND node_type = (:node_type);)");
        query.bindValue(QStringLiteral(":path_expr"), parentPath);
        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Note));

        bool status = query.exec();
        if (status) {
            while (query.next()) {
                NodeData node;
                node.setId(query.value(0).toInt());
                node.setFullTitle(query.value(1).toString());
                node.setCreationDateTime(
                    QDateTime::fromMSecsSinceEpoch(query.value(2).toLongLong()));
                node.setLastModificationDateTime(
                    QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong()));
                node.setDeletionDateTime(
                    QDateTime::fromMSecsSinceEpoch(query.value(4).toLongLong()));
                node.setContent(query.value(5).toString());
                node.setNodeType(static_cast<NodeData::Type>(query.value(6).toInt()));
                node.setParentId(query.value(7).toInt());
                node.setRelativePosition(query.value(8).toInt());
                node.setScrollBarPosition(query.value(9).toInt());
                node.setAbsolutePath(query.value(10).toString());
                node.setIsPinnedNote(static_cast<bool>(query.value(11).toInt()));
                node.setRelativePosAN(query.value(13).toInt());
                node.setChildNotesCount(query.value(14).toInt());

                nodeList.append(node);
            }
        }
        else
        {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
    }


    ListViewInfo inf;
    inf.isInSearch = false;
    inf.isInTag = false;
    inf.parentFolderId = parentID;
    inf.currentNotesId = { SpecialNodeID::InvalidNodeId };
    inf.needCreateNewNote = newNote;
    inf.scrollToId = scrollToId;
    //根据创建时间排序
    std::sort(nodeList.begin(), nodeList.end(), [](const NodeData &a, const NodeData &b) -> bool {
        return a.lastModificationdateTime() > b.lastModificationdateTime();
    });

    emit notesListReceived(nodeList, inf);
}

void DBManager::onOpenDBManagerRequested(const QString &path, bool doCreate)
{
    this->open(path, doCreate);
}

void DBManager::onCreateUpdateRequestedNoteContent(const NodeData &note)
{
    //不是笔记类型(是文件夹类型)
    if (note.nodeType() != NodeData::Note)
    {
        qDebug() << "Wrong node type";
        return;
    }

    bool exists = isNodeExist(note);    //看这个节点是否存在

    if (exists)
    {
        updateNoteContent(note);
    }
    else
    {
        addNode(note);
    }
}

void DBManager::onImportNotesRequested(const QString &fileName)
{
    //打开文件
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << __FUNCTION__ << __LINE__ << "fail to open file";
        return;
    }
    auto magic_header = file.read(16);
    file.close();

    //如果是这个类型的数据库，就打开
    if (QString::fromUtf8(magic_header).startsWith(QStringLiteral("SQLite format 3")))
    {
        qDebug() << __FUNCTION__;
        auto outside_db = QSqlDatabase::addDatabase("QSQLITE", OUTSIDE_DATABASE_NAME);
        outside_db.setDatabaseName(fileName);
        if (!outside_db.open())
        {
            qDebug() << __FUNCTION__ << "Error: connection with database fail";
            return;
        }
        else
        {
            qDebug() << __FUNCTION__ << "Database: connection ok";
        }

        QMap<int, int> folderIdMap, noteIdMap;


        //查询文件夹
        {
            folderIdMap[SpecialNodeID::RootFolder] = SpecialNodeID::RootFolder;
            folderIdMap[SpecialNodeID::TrashFolder] = SpecialNodeID::TrashFolder;
            folderIdMap[SpecialNodeID::DefaultNotesFolder] = SpecialNodeID::DefaultNotesFolder;
            QSqlQuery out_qr(outside_db);
            //查询文件夹类型的节点
            out_qr.prepare(R"(SELECT)"
                           R"("id",)"
                           R"("title",)"
                           R"("creation_date",)"
                           R"("modification_date",)"
                           R"("deletion_date",)"
                           R"("content",)"
                           R"("node_type",)"
                           R"("parent_id",)"
                           R"("relative_position",)"
                           R"("absolute_path" )"
                           R"(FROM node_table WHERE node_type=:node_type;)");
            out_qr.bindValue(":node_type", static_cast<int>(NodeData::Type::Folder));


            bool status = out_qr.exec();
            QMap<int, NodeData> nodeList;
            if (status)
            {
                //根据结果一个个获取参数，然后组装成nodedata加入到nodelist
                while (out_qr.next())
                {
                    NodeData node;
                    node.setId(out_qr.value(0).toInt());
                    node.setFullTitle(out_qr.value(1).toString());
                    node.setCreationDateTime(
                        QDateTime::fromMSecsSinceEpoch(out_qr.value(2).toLongLong()));
                    node.setLastModificationDateTime(
                        QDateTime::fromMSecsSinceEpoch(out_qr.value(3).toLongLong()));
                    node.setDeletionDateTime(
                        QDateTime::fromMSecsSinceEpoch(out_qr.value(4).toLongLong()));
                    node.setContent(out_qr.value(5).toString());
                    node.setNodeType(static_cast<NodeData::Type>(out_qr.value(6).toInt()));
                    node.setParentId(out_qr.value(7).toInt());
                    node.setRelativePosition(out_qr.value(8).toInt());
                    node.setAbsolutePath(out_qr.value(9).toString());
                    nodeList[node.id()] = node;
                }



                //查找或创建文件夹，并把id保存在foldermap中
                auto matchFolderFunc = [&](int id, const auto &matchFolderFunctor)
                {
                    if (!nodeList.contains(id))   //nodelist中不包含这个id
                    {
                        qDebug() << __FUNCTION__ << __LINE__ << "node not found";
                        return;
                    }
                    if (folderIdMap.contains(id)) //已经处理过了
                    {
                        return;
                    }


                    auto node = nodeList[id]; //获取这个节点

                    //看文件夹map是否包含这节点的父节点
                    if (folderIdMap.contains(node.parentId()))
                    {
                        QSqlQuery qr(m_db);
                        //查询数据库中是否包含以下条件的id
                        qr.prepare(
                            R"(SELECT "id" FROM node_table WHERE title = :title AND node_type = :node_type AND parent_id = :parent_id;)");
                        qr.bindValue(QStringLiteral(":title"), node.fullTitle());
                        qr.bindValue(QStringLiteral(":node_type"),
                                     static_cast<int>(NodeData::Folder));
                        qr.bindValue(QStringLiteral(":parent_id"), folderIdMap[node.parentId()]);

                        //将包含上述条件的id的查询结果添加到ids
                        QVector<int> ids;
                        if (qr.exec())
                        {
                            while (qr.next())
                            {
                                ids.append(qr.value(0).toInt());
                            }
                        }
                        else
                        {
                            qDebug() << __FUNCTION__ << __LINE__ << qr.lastError();
                        }
                        qr.finish();

                        //不为空，则将第一个添加到map
                        if (!ids.isEmpty())
                        {
                            folderIdMap[id] = ids[0];
                        }
                        else
                        {
                            node.setParentId(folderIdMap[node.parentId()]);
                            int newId = addNode(node);  //添加节点
                            folderIdMap[id] = newId;
                        }

                    //不包含父节点
                    }
                    else
                    {
                        //将这个节点的路径分割，再来查找
                        auto prL = NodePath(node.absolutePath()).separate();
                        for (const auto &pr : qAsConst(prL))
                        {
                            matchFolderFunctor(pr.toInt(), matchFolderFunctor);
                        }
                    }
                };


                struct Folder
                {
                    int id;
                    int parentId;
                    std::vector<Folder *> children;
                };

                Folder *rootFolder = nullptr;
                QHash<int, Folder> needImportFolderMap;


                for (const auto &node : qAsConst(nodeList))
                {
                    Folder f;
                    f.id = node.id();
                    f.parentId = node.parentId();
                    needImportFolderMap[node.id()] = f;
                    if (f.id == SpecialNodeID::RootFolder)
                    {
                        rootFolder = &needImportFolderMap[node.id()];
                    }
                }

                //如果根节点不为空
                if (rootFolder)
                {
                    for (const auto &folder : qAsConst(needImportFolderMap))
                    {
                        if (folder.id != SpecialNodeID::RootFolder)
                        {
                            needImportFolderMap[folder.parentId].children.push_back(
                                &needImportFolderMap[folder.id]);
                        }
                    }


                    //根据节点相对位置进行排序
                    auto sortChildFunc = [&](Folder *f)
                    {
                        std::sort(f->children.begin(), f->children.end(),
                                  [&](Folder *f1, Folder *f2) {
                                      return nodeList[f1->id].relativePosition()
                                             < nodeList[f2->id].relativePosition();
                                  });
                    };
                    //递归地对整个文件夹树排序
                    auto sortFunc = [&](Folder *f, const auto &sf) -> void
                    {
                        sortChildFunc(f);
                        for (auto c : f->children)
                        {
                            sf(c, sf);
                        }
                    };
                    //调用函数
                    sortFunc(rootFolder, sortFunc);


                    //递归地调用之前定义的matchfunc处理节点
                    auto matchFunc = [&](Folder *f, const auto &mf) -> void
                    {
                        matchFolderFunc(f->id, matchFolderFunc);
                        for (auto c : f->children)
                        {
                            mf(c, mf);
                        }
                    };
                    matchFunc(rootFolder, matchFunc);

                }

                //根节点为空
                else
                {
                    qDebug() << __FUNCTION__ << "Error while keeping folder position";
                    for (const auto &node : qAsConst(nodeList))
                    {
                        matchFolderFunc(node.id(), matchFolderFunc);
                    }
                }
            }
            else
            {
                qDebug() << __FUNCTION__ << __LINE__ << out_qr.lastError();
            }
        }



        //查询笔记
        {
            QSqlQuery out_qr(outside_db);
            out_qr.prepare(R"(SELECT)"
                           R"("id",)"
                           R"("title",)"
                           R"("creation_date",)"
                           R"("modification_date",)"
                           R"("deletion_date",)"
                           R"("content",)"
                           R"("node_type",)"
                           R"("parent_id",)"
                           R"("relative_position",)"
                           R"("scrollbar_position",)"
                           R"("absolute_path", )"
                           R"("is_pinned_note", )"
                           R"("relative_position_an" )"
                           R"(FROM node_table WHERE node_type=:node_type;)");

            out_qr.bindValue(":node_type", static_cast<int>(NodeData::Type::Note));
            bool status = out_qr.exec();

            QVector<NodeData> nodeList;
            QMap<int, std::pair<NodeData, int>> parents;


            for (const auto &id : qAsConst(folderIdMap))
            {
                if (parents.contains(id))
                {
                    continue;
                }
                auto node = getNode(id);
                auto rel = nextAvailablePosition(id, NodeData::Note);
                parents[id] = std::make_pair(node, rel);
            }


            //查询结果添加到nodelist
            if (status)
            {
                while (out_qr.next())
                {
                    NodeData node;
                    node.setId(out_qr.value(0).toInt());
                    node.setFullTitle(out_qr.value(1).toString());
                    node.setCreationDateTime(
                        QDateTime::fromMSecsSinceEpoch(out_qr.value(2).toLongLong()));
                    node.setLastModificationDateTime(
                        QDateTime::fromMSecsSinceEpoch(out_qr.value(3).toLongLong()));
                    node.setDeletionDateTime(
                        QDateTime::fromMSecsSinceEpoch(out_qr.value(4).toLongLong()));
                    node.setContent(out_qr.value(5).toString());
                    node.setNodeType(static_cast<NodeData::Type>(out_qr.value(6).toInt()));
                    node.setParentId(out_qr.value(7).toInt());
                    node.setRelativePosition(out_qr.value(8).toInt());
                    node.setScrollBarPosition(out_qr.value(9).toInt());
                    node.setAbsolutePath(out_qr.value(10).toString());
                    node.setIsPinnedNote(static_cast<bool>(out_qr.value(11).toInt()));
                    node.setRelativePosAN(out_qr.value(13).toInt());
                    node.setChildNotesCount(out_qr.value(14).toInt());

                    nodeList.append(node);
                }

                //开启事务
                m_db.transaction();
                auto nodeId = nextAvailableNodeId();
                //将节点添加到数据库中
                for (auto node : qAsConst(nodeList))
                {
                    if (folderIdMap.contains(node.parentId())
                        && parents.contains(folderIdMap[node.parentId()]))
                    {
                        auto parentId = folderIdMap[node.parentId()];
                        auto parent = parents[parentId].first;
                        auto oldId = node.id();
                        node.setId(nodeId);
                        node.setRelativePosition(parents[parentId].second);
                        node.setAbsolutePath(parent.absolutePath() + PATH_SEPARATOR
                                             + QString::number(nodeId));
                        node.setParentId(parentId);
                        addNodePreComputed(node);   //添加
                        ++nodeId;
                        parents[parentId].second = node.relativePosition() + 1;
                        noteIdMap[oldId] = node.id();
                    }
                    else
                    {
                        qDebug() << __FUNCTION__ << __LINE__ << "can't find parent for note";
                    }
                }

                //更新数据库
                QSqlQuery query(m_db);
                query.prepare(
                    R"(UPDATE "metadata" SET "value"=:value WHERE "key"='next_node_id';)");
                query.bindValue(":value", nodeId + 1);
                if (!query.exec())
                {
                    qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
                }
                m_db.commit();

            }
            else
            {
                qDebug() << __FUNCTION__ << __LINE__ << out_qr.lastError();
            }
        }

        {
            outside_db.close();
            outside_db = QSqlDatabase::database();
        }
    }
    //不是sqlite的数据库
    else
    {
        auto noteList = readOldNBK(fileName);
        if (noteList.isEmpty())
        {
            emit showErrorMessage(tr("Invalid file"), "Please select a valid notes export file");
        }
        else
        {
            auto defaultNoteFolder = getNode(SpecialNodeID::DefaultNotesFolder);
            int nodeId = nextAvailableNodeId();
            int notePos = nextAvailablePosition(defaultNoteFolder.id(), NodeData::Note);
            QString parentAbsPath = defaultNoteFolder.absolutePath();


            m_db.transaction();
            for (auto &note : noteList)
            {
                note.setId(nodeId);
                note.setRelativePosition(notePos);
                note.setAbsolutePath(parentAbsPath + PATH_SEPARATOR + QString::number(nodeId));
                note.setNodeType(NodeData::Note);
                note.setParentId(SpecialNodeID::DefaultNotesFolder);
                note.setParentName("Notes");
                note.setIsTempNote(false);
                addNodePreComputed(note);  //这里添加到数据库
                ++nodeId;
                ++notePos;
            }


            QSqlQuery query(m_db);
            query.prepare(R"(UPDATE "metadata" SET "value"=:value WHERE "key"='next_node_id';)");
            query.bindValue(":value", nodeId + 1);
            if (!query.exec())
            {
                qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
            }

            m_db.commit();
        }
    }

    //改变了，重新计算子节点数量
    recalculateChildNotesCount();
}

void DBManager::onRestoreNotesRequested(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << __FUNCTION__ << __LINE__ << "fail to open file";
        return;
    }
    auto magic_header = file.read(16);
    file.close();

    //如果是sqlite数据库
    if (QString::fromUtf8(magic_header).startsWith(QStringLiteral("SQLite format 3")))
    {
        //关闭数据库连接
        {
            m_db.close();
            m_db = QSqlDatabase::database();
        }
        //删除已有数据库文件
        QSqlDatabase::removeDatabase(DEFAULT_DATABASE_NAME);
        QFile::remove(m_dbpath);
        //打开指定位置的数据库
        if (!QFile::copy(fileName, m_dbpath))
        {
            qDebug() << __FUNCTION__ << "Can't import notes";
        };
        open(m_dbpath, false);
    }

    else
    {
        auto noteList = readOldNBK(fileName);
        if (noteList.isEmpty())//是否读取成功
        {
            emit showErrorMessage(tr("Invalid file"), "Please select a valid notes export file");
        }
        else
        {
            //创建新的数据库和表
            {
                m_db.close();
                m_db = QSqlDatabase::database();
            }
            QSqlDatabase::removeDatabase(DEFAULT_DATABASE_NAME);
            QFile::remove(m_dbpath);
            open(m_dbpath, true);

            //找到默认的笔记文件夹节点
            auto defaultNoteFolder = getNode(SpecialNodeID::DefaultNotesFolder);
            int nodeId = nextAvailableNodeId();
            int notePos = nextAvailablePosition(defaultNoteFolder.id(), NodeData::Note);
            QString parentAbsPath = defaultNoteFolder.absolutePath();

            //将list中的节点添加到数据库
            m_db.transaction();
            for (auto &note : noteList)
            {
                note.setId(nodeId);
                note.setRelativePosition(notePos);
                note.setAbsolutePath(parentAbsPath + PATH_SEPARATOR + QString::number(nodeId));
                note.setNodeType(NodeData::Note);
                note.setParentId(defaultNoteFolder.id());
                note.setParentName("Notes");
                note.setIsTempNote(false);
                addNodePreComputed(note);
                ++nodeId;
                ++notePos;
            }

            QSqlQuery query(m_db);
            query.prepare(R"(UPDATE "metadata" SET "value"=:value WHERE "key"='next_node_id';)");
            query.bindValue(":value", nodeId + 1);
            if (!query.exec())
            {
                qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
            }

            m_db.commit();
        }
    }

    recalculateChildNotesCount();
}

void DBManager::onExportNotesRequested(const QString &fileName)
{
    //锁
    QSqlQuery query(m_db);
    query.prepare("BEGIN IMMEDIATE;");
    bool status = query.exec();
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    QFile::remove(fileName);//确保导出前文件是空的

    //将数据库文件导出到file中
    if (!QFile::copy(m_dbpath, fileName))
    {
        qDebug() << __FUNCTION__ << "Can't export notes";
    };

    //回滚
    query.prepare("ROLLBACK;");
    status = query.exec();
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}


void DBManager::onChangeDatabasePathRequested(const QString &newPath)
{
    {
        m_db.commit();
        m_db.close();
        m_db = QSqlDatabase::database();
    }
    QFile::rename(m_dbpath, newPath);
    open(newPath, false);
}

void DBManager::increaseChildNotesCountFolder(int folderId)
{
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT child_notes_count, absolute_path  FROM "node_table" WHERE id=:id)");
    query.bindValue(QStringLiteral(":id"), folderId);

    bool status = query.exec();
    int childNotesCount = 0;
    QString absPath;
    //获取文件夹的位置
    if (status)
    {
        query.next();
        childNotesCount = query.value(0).toInt();  //子节点数
        absPath = query.value(1).toString();       //绝对路径
    }
    else
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        return;
    }
    query.clear();



    //更新
    childNotesCount += 1;  //子节点数+1

    query.prepare(QStringLiteral("UPDATE node_table SET child_notes_count = :child_notes_count "
                                 "WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), folderId);
    query.bindValue(QStringLiteral(":child_notes_count"), childNotesCount);
    status = query.exec();
    if (!status) //不成功
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }



    //发送信号，文件夹子节点更新，让model等其他对象更新
    emit childNotesCountUpdatedFolder(folderId, absPath, childNotesCount);

}

void DBManager::decreaseChildNotesCountFolder(int folderId)
{

}

bool DBManager::isNodeExist(const NodeData &node)
{
    QSqlQuery query(m_db);

    //判断id是否存在,在数据库中查询，根据id因为id唯一
    int id = node.id();
    QString queryStr =      //select exists表示是否存在
        QStringLiteral("SELECT EXISTS(SELECT 1 FROM node_table WHERE id = :id LIMIT 1 )");
    query.prepare(queryStr);
    query.bindValue(":id", id);
    bool status = query.exec();
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError() << query.isValid();
    }

    query.next();
    return query.value(0).toInt() == 1; //是否为1，因为查询如果有的话返回1
}

QVector<NodeData> DBManager::getAllFolders()
{
    QVector<NodeData> nodeList;

    //查询文件夹类型的节点的数据
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT)"
                  R"("id",)"
                  R"("title",)"
                  R"("creation_date",)"
                  R"("modification_date",)"
                  R"("deletion_date",)"
                  R"("content",)"
                  R"("node_type",)"
                  R"("parent_id",)"
                  R"("relative_position",)"
                  R"("absolute_path", )"
                  R"("child_notes_count" )"
                  R"(FROM node_table WHERE node_type=:node_type;)");
    query.bindValue(":node_type", static_cast<int>(NodeData::Type::Folder));


    bool status = query.exec();
    if (status)
    {
        //一直查询，设置参数，然后返回节点
        while (query.next())
        {
            NodeData node;
            node.setId(query.value(0).toInt());
            node.setFullTitle(query.value(1).toString());
            node.setCreationDateTime(QDateTime::fromMSecsSinceEpoch(query.value(2).toLongLong()));
            node.setLastModificationDateTime(
                QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong()));
            node.setDeletionDateTime(QDateTime::fromMSecsSinceEpoch(query.value(4).toLongLong()));
            node.setContent(query.value(5).toString());
            node.setNodeType(static_cast<NodeData::Type>(query.value(6).toInt()));
            node.setParentId(query.value(7).toInt());
            node.setRelativePosition(query.value(8).toInt());
            node.setAbsolutePath(query.value(9).toString());
            node.setChildNotesCount(query.value(10).toInt());
            //添加
            nodeList.append(node);
        }
    }
    else
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    //返回所查的列表
    return nodeList;
}

bool DBManager::updateNoteContent(const NodeData &note)
{
    QSqlQuery query(m_db);
    QString emptyStr;

    int id = note.id();
    //节点不合法直接返回
    if (id == SpecialNodeID::InvalidNodeId)
    {
        qDebug() << "Invalid Note ID";
        return false;
    }

    //节点的参数
    qint64 epochTimeDateModified = note.lastModificationdateTime().toMSecsSinceEpoch();
    QString content = note.content().replace(QChar('\x0'), emptyStr);
    QString fullTitle = note.fullTitle().replace(QChar('\x0'), emptyStr);

    //更新节点
    query.prepare(QStringLiteral(
        "UPDATE node_table SET modification_date = :modification_date, content = :content, "
        "title = :title, scrollbar_position = :scrollbar_position WHERE id = :id AND node_type "
        "= :node_type;"));
    query.bindValue(QStringLiteral(":modification_date"), epochTimeDateModified);
    query.bindValue(QStringLiteral(":content"), content);
    query.bindValue(QStringLiteral(":title"), fullTitle);
    query.bindValue(QStringLiteral(":scrollbar_position"), note.scrollBarPosition());
    query.bindValue(QStringLiteral(":id"), id);
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Note));

    if (!query.exec())
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }

    return (query.numRowsAffected() == 1);  //返回是否更新成功
}

QList<NodeData> DBManager::readOldNBK(const QString &fileName)
{
    QList<NodeData> noteList;

    //根据传入的文件名，获取这个文件的数据流
    {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_5_12);

        try
        {
            in >> noteList; //输入到list中
        }
        catch (...)
        {
            // Any exception deserializing will result in an empty note list and  the user will be
            // notified
        }
        file.close();
    }

    //如果list为空，那么不用普通的nodedata，而是创建一个nodedata指针类型的list
    if (noteList.isEmpty())
    {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_5_12);

        QList<NodeData *> nl;

        try
        {
            in >> nl;
        }
        catch (...)
        {
            // Any exception deserializing will result in an empty note list and  the user will be
            // notified
        }

        if (!nl.isEmpty()) //不为空，就从nl中一个个传到之前list中
        {
            for (const auto &n : qAsConst(nl))
            {
                noteList.append(*n);
            }
        }
        qDeleteAll(nl); //释放内从
        file.close();
    }

    return noteList;
}

int DBManager::nextAvailablePosition(int parentId, NodeData::Type nodeType)
{
    QSqlQuery query(m_db);
    int relationalPosition = 0;

    //查询父节点的所有子节点的相对位置
    if (parentId != -1)
    {
        query.prepare(R"(SELECT relative_position FROM "node_table" )"
                      R"(WHERE parent_id = :parent_id AND node_type = :node_type;)");
        query.bindValue(":parent_id", parentId);
        query.bindValue(":node_type", static_cast<int>(nodeType));
        bool status = query.exec();
        if (status)
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

    return relationalPosition;
}

int DBManager::addNodePreComputed(const NodeData &node)
{
    QSqlQuery query(m_db);
    QString emptyStr;

    //相关信息
    qint64 epochTimeDateCreated = node.creationDateTime().toMSecsSinceEpoch();
    QString content = node.content().replace("'", "''").replace(QChar('\x0'), emptyStr);
    QString fullTitle = node.fullTitle().replace("'", "''").replace(QChar('\x0'), emptyStr);

    qint64 epochTimeDateLastModified = node.lastModificationdateTime().isNull()
                                           ? epochTimeDateCreated
                                           : node.lastModificationdateTime().toMSecsSinceEpoch();

    int relationalPosition = node.relativePosition();
    int nodeId = node.id();
    QString absolutePath = node.absolutePath();

    //插入新的信息
    QString queryStr =
        R"(INSERT INTO "node_table" )"
        R"(("id", "title", "creation_date", "modification_date", "deletion_date", "content", "node_type", "parent_id", "relative_position", "scrollbar_position", "absolute_path", "is_pinned_note", "relative_position_an", "child_notes_count") )"
        R"(VALUES (:id, :title, :creation_date, :modification_date, :deletion_date, :content, :node_type, :parent_id, :relative_position, :scrollbar_position, :absolute_path, :is_pinned_note, :relative_position_an, :child_notes_count);)";

    //绑定相关参数
    query.prepare(queryStr);
    query.bindValue(":id", nodeId);
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

    //真正实施插入
    bool status = query.exec();
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError() << query.isValid();
    }
    query.finish();

    return nodeId;
}

void DBManager::renameNode(int id, const QString &newName)
{
    QSqlQuery query(m_db);
    query.prepare(R"(UPDATE "node_table" SET "title"=:title WHERE "id"=:id;)");
    query.bindValue(":title", newName);
    query.bindValue(":id", id);
    if (!query.exec())
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

void DBManager::moveNode(int nodeId, const NodeData &target)
{
    //目标节点不是文件件类型
    if (target.nodeType() != NodeData::Folder)
    {
        qDebug() << "moveNode target is not folder" << target.id();
        return;
    }
    QSqlQuery query(m_db);
    auto node = getNode(nodeId);

    //新的绝对路径
    QString newAbsolutePath = target.absolutePath() + PATH_SEPARATOR + QString::number(nodeId);

    //目标节点是垃圾桶类型
    if (target.id() == SpecialNodeID::TrashFolder)
    {
        qint64 deletionTime = QDateTime::currentMSecsSinceEpoch();
        query.prepare(QStringLiteral(
            "UPDATE node_table SET parent_id = :parent_id, absolute_path = :absolute_path, "
            "is_pinned_note = :is_pinned_note, deletion_date = :deletion_date "
            "WHERE id = :id;"));
        query.bindValue(QStringLiteral(":parent_id"), target.id());
        query.bindValue(QStringLiteral(":absolute_path"), newAbsolutePath);
        query.bindValue(QStringLiteral(":is_pinned_note"), false);
        query.bindValue(QStringLiteral(":deletion_date"), deletionTime);
        query.bindValue(QStringLiteral(":id"), nodeId);
    }
    //否则移动节点
    else
    {
        query.prepare(QStringLiteral(
            "UPDATE node_table SET parent_id = :parent_id, absolute_path = :absolute_path "
            "WHERE id = :id;"));
        query.bindValue(QStringLiteral(":parent_id"), target.id());
        query.bindValue(QStringLiteral(":absolute_path"), newAbsolutePath);
        query.bindValue(QStringLiteral(":id"), nodeId);
    }
    bool status = query.exec();
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError() << query.lastQuery();
    }

    //如果节点是文件夹类型，也要移动子节点
    if (node.nodeType() == NodeData::Folder)
    {
        QString oldAbsolutePath = node.absolutePath();
        QMap<int, QString> children; //子节点,id与绝对路径
        query.clear();
        query.prepare(R"(SELECT id, absolute_path FROM "node_table" )"
                      R"(WHERE absolute_path like (:path_expr) || '%';)");
        query.bindValue(QStringLiteral(":path_expr"), oldAbsolutePath);
        if (query.exec())
        {
            while (query.next())
            {
                if (query.value(0).toInt() != node.id())
                {
                    children[query.value(0).toInt()] = query.value(1).toString();
                }
            }
        }
        else
        {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }

        //遍历子节点，更新位置
        for (const auto id : children.keys())
        {
            QString oldP = children[id];
            QString newP = oldP.replace(oldP.indexOf(oldAbsolutePath), oldAbsolutePath.size(),
                                        newAbsolutePath);
            //要移入垃圾桶节点
            if (target.id() == SpecialNodeID::TrashFolder)
            {
                qint64 deletionTime = QDateTime::currentMSecsSinceEpoch();
                query.prepare(QStringLiteral(
                    "UPDATE node_table SET absolute_path = :absolute_path, is_pinned_note = "
                    ":is_pinned_note, deletion_date = :deletion_date "
                    "WHERE id = :id;"));
                query.bindValue(QStringLiteral(":absolute_path"), newP);
                query.bindValue(QStringLiteral(":id"), id);
                query.bindValue(QStringLiteral(":is_pinned_note"), false);
                query.bindValue(QStringLiteral(":deletion_date"), deletionTime);
            }
            //移动到其他文件夹
            else
            {
                query.prepare(QStringLiteral("UPDATE node_table SET absolute_path = :absolute_path "
                                             "WHERE id = :id;"));
                query.bindValue(QStringLiteral(":absolute_path"), newP);
                query.bindValue(QStringLiteral(":id"), id);
            }
            if (!query.exec())
            {
                qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
            }
        }

        //更新子节点数
        recalculateChildNotesCount();
    }
}

void DBManager::updateRelPosNode(int nodeId, int relPos)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE node_table SET relative_position = :relative_position "
                                 "WHERE id = :id;"));
    query.bindValue(QStringLiteral(":relative_position"), relPos);
    query.bindValue(QStringLiteral(":id"), nodeId);
    bool status = query.exec();
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

void DBManager::searchForNotes(const QString &keyword, const ListViewInfo &inf)
{
    //节点列表
    QVector<NodeData> nodeList;

    QSqlQuery query(m_db);
    //寻找带有关键字的节点 //
    if (!inf.isInTag && inf.parentFolderId == SpecialNodeID::RootFolder)
    {
        query.prepare(R"(SELECT )"
                      R"("id",)"
                      R"("title",)"
                      R"("creation_date",)"
                      R"("modification_date",)"
                      R"("deletion_date",)"
                      R"("content",)"
                      R"("node_type",)"
                      R"("parent_id",)"
                      R"("relative_position", )"
                      R"("scrollbar_position",)"
                      R"("absolute_path", )"
                      R"("is_pinned_note", )"
                      R"("relative_position_an", )"
                      R"("child_notes_count" )"
                      R"(FROM node_table )"
                      R"(WHERE node_type = (:node_type) AND parent_id != (:parent_id) )"
                      R"(AND content like  '%' || (:search_expr) || '%';)");
        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Note));
        query.bindValue(QStringLiteral(":parent_id"), static_cast<int>(SpecialNodeID::TrashFolder));
        query.bindValue(QStringLiteral(":search_expr"), keyword);

        bool status = query.exec();
        if (status)
        {
            //遍历所有结果，添加到nodelist中
            while (query.next())
            {
                NodeData node;
                node.setId(query.value(0).toInt());
                node.setFullTitle(query.value(1).toString());
                node.setCreationDateTime(
                    QDateTime::fromMSecsSinceEpoch(query.value(2).toLongLong()));
                node.setLastModificationDateTime(
                    QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong()));
                node.setDeletionDateTime(
                    QDateTime::fromMSecsSinceEpoch(query.value(4).toLongLong()));
                node.setContent(query.value(5).toString());
                node.setNodeType(static_cast<NodeData::Type>(query.value(6).toInt()));
                node.setParentId(query.value(7).toInt());
                node.setRelativePosition(query.value(8).toInt());
                node.setScrollBarPosition(query.value(9).toInt());
                node.setAbsolutePath(query.value(10).toString());
                node.setIsPinnedNote(static_cast<bool>(query.value(11).toInt()));
                node.setRelativePosAN(query.value(13).toInt());
                node.setChildNotesCount(query.value(14).toInt());

                auto p = getNode(node.parentId());
                node.setParentName(p.fullTitle());
                nodeList.append(node);
            }
        }
        else
        {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
    }


    else if (!inf.isInTag)
    {
        query.prepare(R"(SELECT )"
                      R"("id",)"
                      R"("title",)"
                      R"("creation_date",)"
                      R"("modification_date",)"
                      R"("deletion_date",)"
                      R"("content",)"
                      R"("node_type",)"
                      R"("parent_id",)"
                      R"("relative_position", )"
                      R"("scrollbar_position",)"
                      R"("absolute_path", )"
                      R"("is_pinned_note", )"
                      R"("relative_position_an", )"
                      R"("child_notes_count" )"
                      R"(FROM node_table )"
                      R"(WHERE node_type = (:node_type) AND parent_id == (:parent_id) )"
                      R"(AND content like  '%' || (:search_expr) || '%';)");
        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Note));
        query.bindValue(QStringLiteral(":parent_id"), static_cast<int>(inf.parentFolderId));
        query.bindValue(QStringLiteral(":search_expr"), keyword);

        bool status = query.exec();
        if (status)
        {
            while (query.next())
            {
                NodeData node;
                node.setId(query.value(0).toInt());
                node.setFullTitle(query.value(1).toString());
                node.setCreationDateTime(
                    QDateTime::fromMSecsSinceEpoch(query.value(2).toLongLong()));
                node.setLastModificationDateTime(
                    QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong()));
                node.setDeletionDateTime(
                    QDateTime::fromMSecsSinceEpoch(query.value(4).toLongLong()));
                node.setContent(query.value(5).toString());
                node.setNodeType(static_cast<NodeData::Type>(query.value(6).toInt()));
                node.setParentId(query.value(7).toInt());
                node.setRelativePosition(query.value(8).toInt());
                node.setScrollBarPosition(query.value(9).toInt());
                node.setAbsolutePath(query.value(10).toString());
                node.setIsPinnedNote(static_cast<bool>(query.value(11).toInt()));
                node.setRelativePosAN(query.value(13).toInt());
                node.setChildNotesCount(query.value(14).toInt());

                auto p = getNode(node.parentId());
                node.setParentName(p.fullTitle());
                nodeList.append(node);
            }
        }
        else
        {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
    }


    else if (inf.isInTag)
    {
        QVector<QSet<int>> nds;
        for (const auto &tagId : inf.currentTagList)
        {
            QSet<int> nd;
            query.prepare(R"(SELECT "node_id" FROM tag_relationship WHERE tag_id = :tag_id;)");
            query.bindValue(QStringLiteral(":tag_id"), tagId);
            bool status = query.exec();
            if (status)
            {
                while (query.next())
                {
                    nd.insert(query.value(0).toInt());
                }
            }
            else
            {
                qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
            }
            nds.append(nd);
        }
        if (nds.size() == 0)
        {
            emit notesListReceived(nodeList, inf);
            return;
        }
        QSet<int> noteIds;
        int nds_id = 0;
        for (int i = 1; i < nds.size(); ++i)
        {
            if (nds[i].size() < nds[nds_id].size())
            {
                nds_id = i;
            }
        }
        for (const int id : qAsConst(nds[nds_id]))
        {
            bool ok = true;
            for (int i = 0; i < nds.size(); ++i)
            {
                if (i == nds_id)
                {
                    continue;
                }
                if (!nds[i].contains(id))
                {
                    ok = false;
                }
            }
            if (ok)
            {
                noteIds.insert(id);
            }
        }
        for (const auto &id : noteIds)
        {
            NodeData node = getNode(id);
            if (node.id() != SpecialNodeID::InvalidNodeId && node.nodeType() == NodeData::Note
                && node.content().contains(keyword))
            {
                nodeList.append(node);
            }
            else
            {
                qDebug() << __FUNCTION__ << "Note with id" << id << "is not valid";
            }
        }
    }
    else
    {
        qDebug() << __FUNCTION__ << __LINE__ << "not supported";
    }
    ListViewInfo _inf = inf;
    _inf.isInSearch = true;
    std::sort(nodeList.begin(), nodeList.end(), [](const NodeData &a, const NodeData &b) -> bool {
        return a.lastModificationdateTime() > b.lastModificationdateTime();
    });


    emit notesListReceived(nodeList, _inf);
}

void DBManager::clearSearch(const ListViewInfo &inf)
{
    //父节点是根节点
    if (inf.parentFolderId == SpecialNodeID::RootFolder)
    {
        onNotesListInFolderRequested(inf.parentFolderId, true, inf.needCreateNewNote,
                                     inf.scrollToId);
    }
    else
    {
        onNotesListInFolderRequested(inf.parentFolderId, false, inf.needCreateNewNote,
                                     inf.scrollToId);
    }

}

void DBManager::updateRelPosPinnedNote(int nodeId, int relPos)
{
    QSqlQuery query(m_db);
    //更新指定节点的相对位置
    query.prepare(QStringLiteral("UPDATE node_table SET relative_position = :relative_position "
                                 "WHERE id = :id AND node_type=:node_type;"));
    query.bindValue(QStringLiteral(":relative_position"), relPos);
    query.bindValue(QStringLiteral(":id"), nodeId);
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Note));

    bool status = query.exec();
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

void DBManager::updateRelPosPinnedNoteAN(int nodeId, int relPos)
{
    QSqlQuery query(m_db);

    query.prepare(
        QStringLiteral("UPDATE node_table SET relative_position_an = :relative_position_an "
                       "WHERE id = :id AND node_type=:node_type;"));
    query.bindValue(QStringLiteral(":relative_position_an"), relPos);
    query.bindValue(QStringLiteral(":id"), nodeId);
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Note));
    bool status = query.exec();
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

void DBManager::setNoteIsPinned(int noteId, bool isPinned)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE node_table SET is_pinned_note = :is_pinned_note "
                                 "WHERE id = :id AND node_type=:node_type;"));
    query.bindValue(QStringLiteral(":is_pinned_note"), isPinned);
    query.bindValue(QStringLiteral(":id"), noteId);
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Note));
    bool status = query.exec();
    if (!status)
    {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

NodeData DBManager::getChildNotesCountFolder(int folderId)
{
    NodeData d;
    d.setNodeType(NodeData::Folder);
    d.setId(folderId);

    QSqlQuery query(m_db);
    query.prepare(R"(SELECT child_notes_count, absolute_path  FROM "node_table" WHERE id=:id)");
    query.bindValue(QStringLiteral(":id"), folderId);

    //获取这个节点
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
    }


    d.setChildNotesCount(childNotesCount);
    d.setAbsolutePath(absPath);
    return d;
}


void DBManager::removeNote(const NodeData &note)
{
    //这个节点的父节点是垃圾桶
    if (note.parentId() == SpecialNodeID::TrashFolder)
    {
        //删除
        QSqlQuery query(m_db);
        query.prepare(R"(DELETE FROM "node_table" )"
                      R"(WHERE id = (:id) AND node_type = (:node_type);)");
        query.bindValue(QStringLiteral(":id"), note.id());
        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Note));
        if (!query.exec())
        {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.clear();


        query.prepare(R"(DELETE FROM "tag_relationship" )"
                      R"(WHERE node_id = (:id);)");
        query.bindValue(QStringLiteral(":id"), note.id());
        if (!query.exec())
        {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        //如果这个节点是笔记，还要将父节点的子节点数-1
        if (note.nodeType() == NodeData::Note)
        {
            decreaseChildNotesCountFolder(SpecialNodeID::TrashFolder);
        }
    }
    //父节点不是文件夹，就将文件移入垃圾桶文件夹即可，不是真正的删除
    else
    {
        auto trashFolder = getNode(SpecialNodeID::TrashFolder);
        moveNode(note.id(), trashFolder);
    }
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
