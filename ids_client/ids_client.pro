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
    ../common/netcfgdialog.cpp

HEADERS  += idsclient.h \
    ../common/displaycfgdialog.h \
    ../common/netcfgdialog.h

FORMS    += idsclient.ui \
    ../common/displaycfgdialog.ui \
    ../common/netcfgdialog.ui

ROOT_DIR = $$system(pwd)/../../
ROOT_LIB_DIR = $$ROOT_DIR/lib
ROOT_INC_DIR = $$ROOT_DIR/inc

INCLUDEPATH += $$ROOT_INC_DIR $$ROOT_INC_DIR/glib-2.0 $$ROOT_LIB_DIR/glib-2.0/include
LIBS += -L $$ROOT_LIB_DIR -lids_fw -lmodules_app_ex -lmodules_core  -Wl,-rpath,$$ROOT_LIB_DIR
