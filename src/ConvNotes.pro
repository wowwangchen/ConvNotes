QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets widgets-private

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    allnotebuttontreedelegateeditor.cpp \
    customapplicationstyle.cpp \
    customdocument.cpp \
    custommarkdownhighlighter.cpp \
    dbmanager.cpp \
    defaultnotefolderdelegateeditor.cpp \
    foldertreedelegateeditor.cpp \
    fontloader.cpp \
    labeledittype.cpp \
    main.cpp \
    mainwindow.cpp \
    markdownhighlighter.cpp \
    morebutton.cpp \
    mylistview.cpp \
    mylistviewlogic.cpp \
    mytreeview.cpp \
    mytreeviewdelegate.cpp \
    mytreeviewlogic.cpp \
    mytreeviewmodel.cpp \
    nodedata.cpp \
    nodepath.cpp \
    nodetreeitem.cpp \
    noteeditorlogic.cpp \
    notelistdelegate.cpp \
    notelistdelegateeditor.cpp \
    notelistmodel.cpp \
    pushbuttontype.cpp \
    qownlanguagedata.cpp \
    trashbuttondelegateeditor.cpp

HEADERS += \
    allnotebuttontreedelegateeditor.h \
    codetranslate.h \
    customapplicationstyle.h \
    customdocument.h \
    custommarkdownhighlighter.h \
    dbmanager.h \
    defaultnotefolderdelegateeditor.h \
    foldertreedelegateeditor.h \
    fontloader.h \
    labeledittype.h \
    mainwindow.h \
    markdownhighlighter.h \
    morebutton.h \
    mylistview.h \
    mylistviewlogic.h \
    mytreeview.h \
    mytreeviewdelegate.h \
    mytreeviewlogic.h \
    mytreeviewmodel.h \
    nodedata.h \
    nodepath.h \
    nodetreeitem.h \
    noteeditorlogic.h \
    notelistdelegate.h \
    notelistdelegateeditor.h \
    notelistmodel.h \
    pushbuttontype.h \
    qownlanguagedata.h \
    trashbuttondelegateeditor.h

FORMS += \
    mainwindow.ui \
    morebutton.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
#INCLUDEPATH += F:/Qt/5.12.9/msvc2017_64/include


RESOURCES += \
    source.qrc
