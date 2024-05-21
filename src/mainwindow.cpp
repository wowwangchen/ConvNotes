#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    m_treeView(nullptr),
    m_treeModel(new myTreeViewModel(this)),
    m_treeViewLogic(nullptr),
    m_listView(nullptr),

    //m_dbManager(nullptr),

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

    //m_dbManager=new DBManager;

    setupMainWindow();
    setupModelView();
    initWindow();
    initConnect();
}

MainWindow::~MainWindow()
{
    delete ui;
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

    //m_treeView->setModel(m_treeModel);
    //m_treeViewLogic = new myTreeViewLogic(m_treeView, m_treeModel, m_dbManager, m_listView, this);
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

    ui->iconPackageLabel->setStyleSheet(iconColorss);

    //m_treeModel=new myTreeViewModel;
    //m_treeDelegate=new myTreeViewDelegate(ui->allPackageTreeView);
    //ui->allPackageTreeView->setModel(m_treeModel);



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



void MainWindow::settingButton_clicked()
{
    QMessageBox::information(this,"no","no");
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


