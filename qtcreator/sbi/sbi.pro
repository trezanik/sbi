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
DEFINES += _DEBUG USING_JSON_SPIRIT_RPC USING_BOOST_NET
DEPENDPATH += ../../src
INCLUDEPATH += ../../src
LIBS += -lapi -lpthread -ldl

#>>> libconfig
DEFINES += USING_LIBCONFIG
contains(DEFINES,USING_LIBCONFIG){
	INCLUDEPATH += ../../third-party/libconfig
	LIBS += -L../../third-party/libconfig/lib/ -lconfig++
}
#<<<
#>>> JSON Spirit
contains(DEFINES,USING_JSON_SPIRIT_RPC){
	INCLUDEPATH += ../../third-party/json_spirit
	# json_spirit->boost dependency
	INCLUDEPATH += ../../third-party/boost
	LIBS += -L../../third-party/boost/lib/ -lboost_system
	# boost->openssl dependency
	INCLUDEPATH += ../../third-party/openssl
	LIBS += -L../../third-party/openssl/lib/ -lssl -lcrypto
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
