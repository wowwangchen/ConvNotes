#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QMessageBox>
#include<QDateTime>
#include<QColorDialog>
#include<QMouseEvent>
#include<QObject>
#include<QSettings>
#include<QFile>
#include<QEvent>
#include<QThread>
#include<QDir>
#include<QFileInfo>
#include<QDebug>
#include<QMetaType>
#include"mytreeview.h"
#include"mytreeviewlogic.h"
#include"mytreeviewmodel.h"
#include"customdocument.h"
#include "codetranslate.h"
#include"notelistmodel.h"
#include"mylistviewlogic.h"
#include"noteeditorlogic.h"
#include"customdocument.h"
#include"custommarkdownhighlighter.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


//qRegisterMetaType< QVector<NodeData> >("QVector<NodeData>");
//整个项目的主界面
//分为treeView，listView，textEdit
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum class ShadowType { Linear = 0, Radial };

    //阴影方向
    enum class ShadowSide
    {
        Left = 0,
        Right,
        Top,
        Bottom,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    //拉伸方向
    enum class StretchSide
    {
        None = 0,
        Left,
        Right,
        Top,
        Bottom,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    //注册到元对象系统中
    Q_ENUM(ShadowType)
    Q_ENUM(ShadowSide)
    Q_ENUM(StretchSide)


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    void    setupSearchEdit();                      //初始化搜索相关的内容
    void    setupMainWindow();                      //初始化主界面
    void    initWindow();                           //初始化窗口
    void    setDataBase();                          //初始化数据库
    void    initConnect();                          //初始化信号与槽
    void    setupModelView();                       //初始化模型视图
    void    setColorDialogSS(QColorDialog *dialog); //设置stylesheet
    void    initializeSettingsDatabase();           //初始化程序设置数据库
    void    createNewNote();                        //新建笔记


public slots:
    void settingButton_clicked();
    void setTheme(Theme::Value theme);

signals:
    void requestOpenDBManager(const QString &path, bool doCreate);
    void requestNodesTree();


protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual bool eventFilter(QObject *obj, QEvent *event) override;


private slots:
    void on_searchButton_clicked();
    void onNewNoteButtonClicked();


private:
    Ui::MainWindow *ui;

    //以下全是代指ui界面中的控件
    myListView*             m_listView;                     //文件列表结构
    NoteListModel*          m_listModel;                    //列表的model
    myListViewLogic*        m_listViewLogic;                //列表结构的所有逻辑统一

    myTreeView*             m_treeView;                     //代指文件夹展示treeView
    myTreeViewModel*        m_treeModel;                    //ui->treeView的数据模型
    myTreeViewLogic*        m_treeViewLogic;                //MVD和数据库逻辑统一的类

    QPushButton*            m_newNoteButton;                //代指新建文件按钮
    QPushButton*            m_dotsButton;                   //代指更多选项按钮
    QPushButton*            m_switchToTextViewButton;       //代指选择文本视图按钮
    QPushButton*            m_switchToKanbanViewButton;     //代指看板视图按钮
    QPushButton*            m_globalSettingsButton;         //代表设置按钮
    QLineEdit*              m_searchEdit;                   //代表搜索编辑行                   
    QLabel*                 m_editorDateLabel;              //代表日期编辑标签

    QSettings*              m_settingsDatabase;             //应用程序配置的数据库
    QSettings*              m_localLicenseData;             //本地许可证的数据
    DBManager*              m_dbManager;                    //数据库
    QThread*                m_dbThread;                     //处理数据库的线程


    QString                 m_displayFont;                  //展示的内容的字体
    Theme::Value            m_currentTheme;                 //当前主题
    QColor                  m_currentEditorTextColor;       //当前编辑的内容的颜色
    QMenu                   m_mainMenu;                     //菜单
    QToolButton*            m_searchButton;                 //搜索按钮
    QToolButton*            m_clearButton;                  //当输入内容后要出来的一个按钮

    NoteEditorLogic*        m_noteEditorLogic;              //笔记编写的操控类
    CustomDocument*         m_textEdit;                     //代表文本编辑框


};
#endif // MAINWINDOW_H
