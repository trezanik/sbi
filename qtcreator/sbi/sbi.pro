TARGET = sbi
TEMPLATE = app
CONFIG -= qt


#---------------------
# BEGIN customizations
#---------------------
QMAKE_CXX = clang++
QMAKE_CXXFLAGS += \
	-std=c++11 \
	-Wall
#QMAKE_CXXFLAGS_DEBUG
#QMAKE_CXXFLAGS_RELEASE
DEFINES += _DEBUG
DEPENDPATH += ../../src
INCLUDEPATH += ../../src
LIBS += -lapi -lpthread -ldl

#>>> libconfig @todo : remove this dependency of a child; definition source???
DEFINES += USING_LIBCONFIG
contains(DEFINES,USING_LIBCONFIG){
	INCLUDEPATH += ../../third-party/libconfig
	LIBS += -L../../third-party/libconfig/lib/ -lconfig++
}
#<<<

# Warning: assumes build directory sbi-all = '$project/qtcreator'
# binary -> '$project/bin/linux_x86-64/debug/sbi'
# objects -> '$project/qtcreator/sbi/debug/obj/xxx.o'
CONFIG(debug, debug|release) {
	DESTDIR = ../../bin/linux_x86-64/debug
	OBJECTS_DIR = debug/obj
	# Dependencies
	LIBS += -L../../lib/linux_x86-64/debug/
} else {
	DESTDIR = ../../bin/linux_x86-64/release
	OBJECTS_DIR = release/obj
	# Dependencies
	LIBS += -L../../lib/linux_x86-64/release/
}
#-------------------
# END customizations
#-------------------



SOURCES += ../../src/sbi/app.cc \
    ../../src/sbi/getopt.cc \
    ../../src/sbi/main.cc


HEADERS += ../../src/sbi/app.h \
    ../../src/sbi/getopt.h
