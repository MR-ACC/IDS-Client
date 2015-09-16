#-------------------------------------------------
#
# Project created by QtCreator 2015-08-31T09:11:34
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ids_server
TEMPLATE = app


SOURCES += main.cpp\
        idsserver.cpp \
    ../common/displaycfgdialog.cpp \
    ../common/netcfgdialog.cpp

HEADERS  += idsserver.h \
    ../common/displaycfgdialog.h \
    ../common/netcfgdialog.h

FORMS    += idsserver.ui \
    ../common/displaycfgdialog.ui \
    ../common/netcfgdialog.ui

ROOT_DIR = $$system(pwd)/../../
ROOT_LIB_DIR = $$ROOT_DIR/lib
ROOT_INC_DIR = $$ROOT_DIR/inc

INCLUDEPATH += $$ROOT_INC_DIR $$ROOT_INC_DIR/glib-2.0 $$ROOT_LIB_DIR/glib-2.0/include
LIBS += -L $$ROOT_LIB_DIR -lids_fw -lmodules_core  -lmodules_app_ex -Wl,-rpath,$$ROOT_LIB_DIR

RESOURCES += \
    image.qrc
