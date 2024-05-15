QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets widgets-private

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    mytreeview.cpp \
    mytreeviewmodel.cpp \
    nodepath.cpp

HEADERS += \
    mainwindow.h \
    mytreeview.h \
    mytreeviewmodel.h \
    nodepath.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
#INCLUDEPATH += F:/Qt/5.12.9/msvc2017_64/include


RESOURCES += \
    source.qrc
