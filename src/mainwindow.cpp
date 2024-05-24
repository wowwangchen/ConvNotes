#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    m_listView(nullptr),
    m_treeView(nullptr),
    m_treeModel(new myTreeViewModel(this)),
    m_treeViewLogic(nullptr),
    m_dbManager(nullptr),

    m_newNoteButton(nullptr),
    m_dotsButton(nullptr),
    m_searchButton(nullptr),
    m_switchToTextViewButton(nullptr),
    m_switchToKanbanViewButton(nullptr),
    m_globalSettingsButton(nullptr),
    m_searchEdit(nullptr),
    m_textEdit(nullptr),
    m_editorDateLabel(nullptr)


{
    ui->setupUi(this);
    setMouseTracking(true);
    ui->iconPackageLabel->installEventFilter(this);  //组件必须安装事件过滤器，主窗口才能接收到事件


    setupMainWindow();
    setDataBase();
    setupModelView();
    initWindow();
    initConnect();

    connect(this, &MainWindow::requestNodesTree, m_dbManager, &DBManager::onNodeTagTreeRequested,
            Qt::BlockingQueuedConnection);
    emit requestNodesTree();
}

MainWindow::~MainWindow()
{
    delete ui;
    m_dbThread->quit();
    m_dbThread->wait();
    delete m_dbThread;
}

void MainWindow::setupMainWindow()
{
    //setupModelView();

    //各种按钮的设置
    m_newNoteButton = ui->addFileButton;
    m_dotsButton = ui->moreSelectButton;
    m_globalSettingsButton = ui->settingButton;
    m_searchEdit = ui->searchLineText;
    m_textEdit = ui->mainTextEdit;
    m_editorDateLabel = ui->lastChangeDateLabel;
    m_switchToTextViewButton = ui->textShowButton;
    m_switchToKanbanViewButton = ui->listShowButton;

    m_newNoteButton->setToolTip(tr("Create New Note"));
    m_dotsButton->setToolTip(tr("Open Editor Settings"));
    m_globalSettingsButton->setToolTip(tr("Open App Settings"));
    m_switchToTextViewButton->setToolTip("Switch To Text View");
    m_switchToKanbanViewButton->setToolTip("Switch To Kanban View");
}

void MainWindow::setupModelView()
{
    m_listView=ui->filesListView;
    //文件夹树形结构
    m_treeView = static_cast<myTreeView*>(ui->allPackageTreeView);
    m_treeView->setModel(m_treeModel);
    m_treeViewLogic = new myTreeViewLogic(m_treeView, m_treeModel, m_dbManager, m_listView, this);

}

void MainWindow::initWindow()
{
    QIcon temp_icon;
    QString ss = QStringLiteral("QPushButton { "
                                "  border: none; "
                                "  padding: 0px; "
                                "  outline: none;"
                                "}");

    QString iconColorss=QStringLiteral("min-width: 24px; "
                                       "min-height: 24px;"
                                       "max-width:24px; "
                                       "max-height: 24px;"
                                       "border-radius: 12px;"
                                       "border:1px;"
                                       "background:%1;"
                                       ).arg("red");



    //窗口分割比例
    ui->splitter->setStretchFactor(0, 5);
    ui->splitter->setStretchFactor(1, 6);
    ui->splitter->setStretchFactor(2, 18);


    temp_icon=QIcon(":/image/setting.png");
    ui->settingButton->setIcon(temp_icon);
    ui->settingButton->setIconSize(ui->settingButton->size());
    ui->settingButton->setStyleSheet(ss);

    temp_icon=QIcon(":/image/search.png");
    ui->searchButton->setIcon(temp_icon);
    ui->searchButton->setIconSize(ui->searchButton->size());
    ui->searchButton->setStyleSheet(ss);

    temp_icon=QIcon(":/image/addNote.png");
    ui->addFileButton->setIcon(temp_icon);
    ui->addFileButton->setIconSize(ui->addFileButton->size());
    ui->addFileButton->setStyleSheet(ss);

    temp_icon=QIcon(":/image/moreSelect.png");
    ui->moreSelectButton->setIcon(temp_icon);
    ui->moreSelectButton->setIconSize(ui->moreSelectButton->size());
    ui->moreSelectButton->setStyleSheet(ss);

    temp_icon=QIcon(":/image/text.png");
    ui->textShowButton->setIcon(temp_icon);
    ui->textShowButton->setIconSize(ui->textShowButton->size());
    ui->textShowButton->setStyleSheet(ss);

    temp_icon=QIcon(":/image/noteList_unuse.png");
    ui->listShowButton->setIcon(temp_icon);
    ui->listShowButton->setIconSize(ui->listShowButton->size());
    ui->listShowButton->setStyleSheet(ss);



    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString formattedDateTime = currentDateTime.toString("yyyy, MM d, h:mm AP");//MM d, yyyy, h:mm AP
    ui->lastChangeDateLabel->setText(formattedDateTime);

    ui->searchLineText->setPlaceholderText("Search");
    QString str = QString::fromUtf8(u8"\U0001F5D1");


    ui->iconPackageLabel->setStyleSheet(iconColorss);

}

void MainWindow::setDataBase()
{
    m_settingsDatabase =
        new QSettings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("myAwesomeness"),
                      QStringLiteral("mySettings"), this);


#if !defined(PRO_VERSION)

#  if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    m_localLicenseData =
        new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                      QStringLiteral("myAwesomeness"), QStringLiteral(".mynotesLicenseData"), this);
#  else
    m_localLicenseData =
        new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                      QStringLiteral("myAwesomeness"), QStringLiteral("mynotesLicenseData"), this);
#  endif

#endif

    m_settingsDatabase->setFallbacksEnabled(false);
    bool needMigrateFromV1_5_0 = false;
    if (m_settingsDatabase->value(QStringLiteral("version"), "NULL") == "NULL")
    {
        needMigrateFromV1_5_0 = true;
    }
    auto versionString = m_settingsDatabase->value(QStringLiteral("version")).toString();
    auto major = versionString.split(".").first().toInt();
    if (major < 2)
    {
        needMigrateFromV1_5_0 = true;
    }
    initializeSettingsDatabase();


    bool doCreate = false;

    //文件消息类
    QFileInfo fi(m_settingsDatabase->fileName());
    QDir dir(fi.absolutePath());
    bool folderCreated = dir.mkpath(QStringLiteral("."));
    if (!folderCreated)
        qFatal("ERROR: Can't create settings folder : %s",
               dir.absolutePath().toStdString().c_str());
    QString defaultDBPath = dir.path() + QDir::separator() + QStringLiteral("notes.db");

    QString noteDBFilePath =
        m_settingsDatabase->value(QStringLiteral("noteDBFilePath"), QString()).toString();
    if (noteDBFilePath.isEmpty())
    {
        noteDBFilePath = defaultDBPath;
    }

    QFileInfo noteDBFilePathInf(noteDBFilePath);
    QFileInfo defaultDBPathInf(defaultDBPath);

    if ((!noteDBFilePathInf.exists()) && (defaultDBPathInf.exists()))
    {
        QDir().mkpath(noteDBFilePathInf.absolutePath());
        QFile defaultDBFile(defaultDBPath);
        defaultDBFile.rename(noteDBFilePath);
    }
    if (QFile::exists(noteDBFilePath) && needMigrateFromV1_5_0)
    {
        {
            auto m_db = QSqlDatabase::addDatabase("QSQLITE", DEFAULT_DATABASE_NAME);
            m_db.setDatabaseName(noteDBFilePath);
            if (m_db.open())
            {
                QSqlQuery query(m_db);
                if (query.exec("SELECT name FROM sqlite_master WHERE type='table' AND "
                               "name='tag_table';"))
                {
                    if (query.next() && query.value(0).toString() == "tag_table")
                    {
                        needMigrateFromV1_5_0 = false;
                    }
                }
                m_db.close();
            }
            m_db = QSqlDatabase::database();
        }
        QSqlDatabase::removeDatabase(DEFAULT_DATABASE_NAME);
    }
    if (!QFile::exists(noteDBFilePath))
    {
        QFile noteDBFile(noteDBFilePath);
        if (!noteDBFile.open(QIODevice::WriteOnly))
            qFatal("ERROR : Can't create database file");

        noteDBFile.close();
        doCreate = true;
        needMigrateFromV1_5_0 = false;
    }
    else if (needMigrateFromV1_5_0)
    {
        QFile noteDBFile(noteDBFilePath);
        noteDBFile.rename(dir.path() + QDir::separator() + QStringLiteral("oldNotes.db"));
        noteDBFile.setFileName(noteDBFilePath);
        if (!noteDBFile.open(QIODevice::WriteOnly))
            qFatal("ERROR : Can't create database file");

        noteDBFile.close();
        doCreate = true;
    }


    if (needMigrateFromV1_5_0)
    {
        m_settingsDatabase->setValue(QStringLiteral("version"), qApp->applicationVersion());
    }



    m_dbManager = new DBManager;
    m_dbThread = new QThread;
    m_dbThread->setObjectName(QStringLiteral("dbThread"));
    m_dbManager->moveToThread(m_dbThread);
    connect(m_dbThread, &QThread::started, this, [=]() {
        setTheme(m_currentTheme);

//        if (needMigrateFromV1_5_0)
//        {
//            emit requestMigrateNotesFromV1_5_0(dir.path() + QDir::separator()
//                                               + QStringLiteral("oldNotes.db"));
//        }
    });
    connect(this, &MainWindow::requestOpenDBManager, m_dbManager,
            &DBManager::onOpenDBManagerRequested, Qt::QueuedConnection);
    emit requestOpenDBManager(noteDBFilePath, doCreate);


//    connect(this, &MainWindow::requestMigrateNotesFromV1_5_0, m_dbManager,
//            &DBManager::onMigrateNotesFrom1_5_0Requested, Qt::QueuedConnection);
    connect(m_dbThread, &QThread::finished, m_dbManager, &QObject::deleteLater);
    m_dbThread->start();
}

void MainWindow::initConnect()
{
    connect(ui->settingButton,&QPushButton::clicked,this,&MainWindow::settingButton_clicked);
    //connect(ui->allPackageTreeView,&myTreeView::renameFolderNameInDatabase,this,[=]{});
}

void MainWindow::setColorDialogSS(QColorDialog *dialog)
{
    dialog->setWindowTitle("选择颜色");
    QIcon icon(":/image/titleIcon.jpg");
    dialog->setWindowIcon(icon);
    //dialog->resize(QSize(1000, 700)); // 设置对话框的宽度为400，高度为300
}

void MainWindow::initializeSettingsDatabase()
{

}



void MainWindow::settingButton_clicked()
{
    QMessageBox::information(this,"no","no");
}

void MainWindow::setTheme(Theme::Value theme)
{

}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->iconPackageLabel && event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); // 事件转换
        if(mouseEvent->button() == Qt::LeftButton)
        {

            //选择一个颜色应用到文件图标label上
            QColorDialog colorDialog;
            setColorDialogSS(&colorDialog);
            QColor color;
            if(colorDialog.exec())
            {
                QColor temp_color = colorDialog.currentColor();
                if(temp_color.isValid()) color=temp_color;
                else return true;
            }
            else
            {
                return true;
            }


            QString iconColorss=QStringLiteral(  "min-width: 24px; "
                                                 "min-height: 24px;"
                                                 "max-width:24px; "
                                                 "max-height: 24px;"
                                                 "border-radius: 12px;"
                                                 "border:1px;"
                                                 "background:%1;"
                                                 ).arg(color.name());
            ui->iconPackageLabel->setStyleSheet(iconColorss);

            return true;
        }
    }
    return false;
}


