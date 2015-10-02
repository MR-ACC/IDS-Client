#-------------------------------------------------
#
# Project created by QtCreator 2015-08-31T09:11:34
#
#-------------------------------------------------

QT       += core gui network opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ids_server
TEMPLATE = app


SOURCES += main.cpp\
        idsserver.cpp \
    ../common/displaycfgdialog.cpp \
    ../common/netcfgdialog.cpp \
    ../common/chncfgdialog.cpp \
    videowidget.cpp \
    ../common/layoutcfgdialog.cpp

HEADERS  += idsserver.h \
    ../common/displaycfgdialog.h \
    ../common/netcfgdialog.h \
    ../common/chncfgdialog.h \
    videowidget.h \
    ../common/layoutcfgdialog.h

FORMS    += idsserver.ui \
    ../common/displaycfgdialog.ui \
    ../common/netcfgdialog.ui \
    ../common/chncfgdialog.ui \
    ../common/layoutcfgdialog.ui

ROOT_DIR = $$system(pwd)/../../
ROOT_LIB_DIR = $$ROOT_DIR/lib
ROOT_INC_DIR = $$ROOT_DIR/inc

INCLUDEPATH += $$ROOT_INC_DIR $$ROOT_INC_DIR/glib-2.0 $$ROOT_LIB_DIR/glib-2.0/include
#LIBS += -L $$ROOT_LIB_DIR -lids_fw -lmodules_core  -lmodules_app_ex -Wl,-rpath,$$ROOT_LIB_DIR
LIBS += -L $$ROOT_LIB_DIR -lids_fw -lmodules_core  -lmodules_app_ex -Wl,-rpath,$$ROOT_LIB_DIR -lGL -lGLU -lglut -L/usr/local/lib -lopencv_cudabgsegm -lopencv_cudaobjdetect -lopencv_cudastereo -lopencv_shape -lopencv_stitching -lopencv_cudafeatures2d -lopencv_superres -lopencv_cudacodec -lopencv_videostab -lopencv_cudaoptflow -lopencv_cudalegacy -lopencv_calib3d -lopencv_features2d -lopencv_objdetect -lopencv_highgui -lopencv_videoio -lopencv_photo -lopencv_imgcodecs -lopencv_cudawarping -lopencv_cudaimgproc -lopencv_cudafilters -lopencv_video -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_cudaarithm -lopencv_viz -lopencv_core -lopencv_cudev -lopencv_hal

RESOURCES += \
    image.qrc

DISTFILES += \
    ../readme.txt
