QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets widgets-private

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    allnotebuttontreedelegateeditor.cpp \
    defaultnotefolderdelegateeditor.cpp \
    foldertreedelegateeditor.cpp \
    fontloader.cpp \
    main.cpp \
    mainwindow.cpp \
    mytreeview.cpp \
    mytreeviewdelegate.cpp \
    mytreeviewmodel.cpp \
    nodedata.cpp \
    nodepath.cpp \
    nodetreeitem.cpp \
    pushbuttontype.cpp \
    trashbuttondelegateeditor.cpp

HEADERS += \
    allnotebuttontreedelegateeditor.h \
    defaultnotefolderdelegateeditor.h \
    foldertreedelegateeditor.h \
    fontloader.h \
    mainwindow.h \
    mytreeview.h \
    mytreeviewdelegate.h \
    mytreeviewmodel.h \
    nodedata.h \
    nodepath.h \
    nodetreeitem.h \
    pushbuttontype.h \
    trashbuttondelegateeditor.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
#INCLUDEPATH += F:/Qt/5.12.9/msvc2017_64/include


RESOURCES += \
    source.qrc
