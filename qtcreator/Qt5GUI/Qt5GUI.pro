#-------------------------------------------------
#
# Project created by QtCreator 2014-06-28T01:17:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# our GUI libraries have a specific naming format (libui-NAME.so)
TARGET = ui-qt5
TEMPLATE = lib

DEFINES += QT5GUI_LIBRARY


#---------------------
# BEGIN customizations
#---------------------
QMAKE_CXX = clang++
QMAKE_CXXFLAGS += \
	-std=c++11 \
	-Wall \
	-include build_config.h
DEFINES += _DEBUG USING_LIBCONFIG
DEPENDPATH += ../../src
INCLUDEPATH += ../../src
MOC_DIR = ../../src/Qt5GUI/generated
UI_DIR = ../../src/Qt5GUI/generated
RCC_DIR = ../../src/Qt5GUI/generated

#>>> libconfig
DEFINES += USING_LIBCONFIG
contains(DEFINES,USING_LIBCONFIG){
	INCLUDEPATH += ../../third-party/libconfig
	LIBS += -L../../third-party/libconfig/lib/ -lconfig++
}
#<<<

# Warning: assumes build directory sbi-all = '$project/qtcreator'
# library -> '$project/lib/linux_x86-64/debug/libui-qt5.so'
# objects -> '$project/qtcreator/Qt5GUI/debug/obj/xxx.o'
CONFIG(debug, debug|release) {
	DESTDIR = ../../lib/linux_x86-64/debug
	OBJECTS_DIR = debug/obj
	# Dependencies
	LIBS += -L../../lib/linux_x86-64/debug/ \
		-lapi
} else {
	DESTDIR = ../../lib/linux_x86-64/release
	OBJECTS_DIR = release/obj
	# Dependencies
	LIBS += -L../../lib/linux_x86-64/release/ \
		-lapi
}
#-------------------
# END customizations
#-------------------


SOURCES += ../../src/Qt5GUI/AboutDialog.cc \
    ../../src/Qt5GUI/InterfacesLoadDialog.cc \
    ../../src/Qt5GUI/InterfacesUnloadDialog.cc \
    ../../src/Qt5GUI/library.cc \
    ../../src/Qt5GUI/ModulesLoadDialog.cc \
    ../../src/Qt5GUI/ModulesUnloadDialog.cc \
    ../../src/Qt5GUI/UI.cc

HEADERS += ../../src/Qt5GUI/AboutDialog.h \
    ../../src/Qt5GUI/InterfacesLoadDialog.h \
    ../../src/Qt5GUI/InterfacesUnloadDialog.h \
    ../../src/Qt5GUI/library.h \
    ../../src/Qt5GUI/ModulesLoadDialog.h \
    ../../src/Qt5GUI/ModulesUnloadDialog.h \
    ../../src/Qt5GUI/UI.h \
    ../../src/Qt5GUI/ui_status.h

FORMS += ../../src/Qt5GUI/forms/AboutDialog.ui \
    ../../src/Qt5GUI/forms/InterfacesLoadDialog.ui \
    ../../src/Qt5GUI/forms/InterfacesUnloadDialog.ui \
    ../../src/Qt5GUI/forms/ModulesLoadDialog.ui \
    ../../src/Qt5GUI/forms/ModulesUnloadDialog.ui \
    ../../src/Qt5GUI/forms/UI.ui

RESOURCES += \
    ../../resources/sbi.qrc
