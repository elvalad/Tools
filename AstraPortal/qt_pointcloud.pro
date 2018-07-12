#-------------------------------------------------
#
# Project created by QtCreator 2017-03-29T11:39:43
#
#-------------------------------------------------

QT       += core gui opengl widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = astraportal
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += orbbec_main.cpp\
    orbbec_main_window.cpp \
    orbbec_pointcloud_gl_widget.cpp \
    orbbec_sensors.cpp \
    orbbec_depth_label.cpp

HEADERS  += orbbec_main_window.h \
    orbbec_pointcloud_gl_widget.h \
    orbbec_util.h \
    orbbec_common.h \
    orbbec_sensors.h \
    orbbec_depth_label.h

FORMS    += orbbec_main_window.ui

win32 {
    win32: LIBS += opengl32.lib
    win32: LIBS += -L$$PWD/3rdparty/windows/OpenNI2/Lib/ -lOpenNI2

    INCLUDEPATH += $$PWD/3rdparty/windows/OpenNI2/Include
    DEPENDPATH += $$PWD/3rdparty/windows/OpenNI2/Include
}

linux {
    linux: LIBS += -lGL -lGLEW
    linux: LIBS += -L$$PWD/3rdparty/linux/OpenNI2/Redist/ -lOpenNI2

    INCLUDEPATH += $$PWD/3rdparty/linux/OpenNI2/Include
    DEPENDPATH += $$PWD/3rdparty/linux/OpenNI2/Include
}
