#-------------------------------------------------
#
# Project created by QtCreator 2018-09-10T10:41:01
#
#-------------------------------------------------

QT       += core gui widgets printsupport
QT += network

TARGET = UDPLogger
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11 \
        c++14
        debug
INCLUDEPATH += \
    src \
    includes

SOURCES += \
    src/udp.cpp \
    src/changegraphdialog.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/plot.cpp \
    src/plots.cpp \
    src/plotscontextmenu.cpp \
    src/qcustomplot.cpp \
    src/settingsdialog.cpp \
    src/signals.cpp

HEADERS += \
    includes/changegraphdialog.h \
    includes/mainwindow.h \
    includes/plot.h \
    includes/plots.h \
    includes/plotscontextmenu.h \
    includes/qcustomplot.h \
    includes/settingsdialog.h \
    includes/signals.h \
    includes/udp.h

FORMS += \
    forms/changegraphdialog.ui \
    forms/mainwindow.ui \
    forms/settingsdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
