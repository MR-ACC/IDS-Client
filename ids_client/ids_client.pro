#-------------------------------------------------
#
# Project created by QtCreator 2015-09-15T11:20:52
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ids_client
TEMPLATE = app


SOURCES += main.cpp\
        idsclient.cpp \
    ../common/displaycfgdialog.cpp \
    ../common/netcfgdialog.cpp \
    ../common/chncfgdialog.cpp \
    upgradedialog.cpp \
    upgradethread.cpp \
    ../common/layoutcfgdialog.cpp \
    stitchdialog.cpp \
    layoutswitchdialog.cpp

HEADERS  += idsclient.h \
    ../common/displaycfgdialog.h \
    ../common/netcfgdialog.h \
    ../common/chncfgdialog.h \
    upgradedialog.h \
    upgradethread.h \
    ../common/layoutcfgdialog.h \
    stitchdialog.h \
    layoutswitchdialog.h \
    ../common/idsutil.h

FORMS    += idsclient.ui \
    ../common/displaycfgdialog.ui \
    ../common/netcfgdialog.ui \
    ../common/chncfgdialog.ui \
    upgradedialog.ui \
    ../common/layoutcfgdialog.ui \
    stitchdialog.ui \
    layoutswitchdialog.ui

RC_FILE += logo.rc

unix {
ROOT_DIR = $$system(pwd)/../../
ROOT_LIB_DIR = $$ROOT_DIR/lib
ROOT_INC_DIR = $$ROOT_DIR/inc

INCLUDEPATH += $$ROOT_INC_DIR $$ROOT_INC_DIR/glib-2.0 $$ROOT_LIB_DIR/glib-2.0/include
LIBS += -L $$ROOT_LIB_DIR -lids_fw  -lmodules_core  -Wl,-rpath,$$ROOT_LIB_DIR
}

win32 {
ROOT_DIR = $$PWD/../..
ROOT_LIB_DIR = $$ROOT_DIR/win32_build/
ROOT_INC_DIR = $$ROOT_DIR/inc

INCLUDEPATH += $$ROOT_INC_DIR $$ROOT_DIR/winlib/win32/include/glib-2.0 $$ROOT_DIR/winlib/win32/lib/glib-2.0/include
LIBS += -L$$ROOT_LIB_DIR -lids_fw -lmodules_core  -Wl,-rpath,$$ROOT_LIB_DIR
}
