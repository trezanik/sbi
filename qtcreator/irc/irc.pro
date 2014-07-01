TEMPLATE = lib
CONFIG += plugin
CONFIG -= qt


#---------------------
# BEGIN customizations
#---------------------
QMAKE_CXX = clang++
QMAKE_CXXFLAGS += \
	-std=c++11 \
	-Wall \
	-Wno-unused-label \
	-Wno-unused-value \
	-Wno-unused-variable \
	-Wno-ignored-qualifiers \
	-Wno-write-strings \
	-include build_config.h
DEFINES += _DEBUG USING_BOOST_NET
DEPENDPATH += ../../src
INCLUDEPATH += ../../src

#>>> clang
contains(QMAKE_CXX,clang) {
	QMAKE_CXXFLAGS += -Wno-c++11-compat-deprecated-writable-strings
}
#<<<
#>>> boost
contains(DEFINES,USING_BOOST_NET) {
	INCLUDEPATH += ../../third-party/boost
	# boost triggers this a lot
	QMAKE_CXXFLAGS += -Wno-unused-local-typedefs
}
#<<<

# Warning: assumes build directory sbi-all = '$project/qtcreator'
# library -> '$project/lib/linux_x86-64/debug/libirc.so'
# objects -> '$project/qtcreator/irc/debug/obj/xxx.o'
CONFIG(debug, debug|release) {
	DESTDIR = ../../lib/linux_x86-64/debug
	OBJECTS_DIR = debug/obj
} else {
	DESTDIR = ../../lib/linux_x86-64/release
	OBJECTS_DIR = release/obj
}
#-------------------
# END customizations
#-------------------


SOURCES += ../../src/irc/interface.cc \
    ../../src/irc/IrcChannel.cc \
    ../../src/irc/IrcConnection.cc \
    ../../src/irc/IrcEngine.cc \
    ../../src/irc/IrcFactory.cc \
    ../../src/irc/IrcNetwork.cc \
    ../../src/irc/IrcObject.cc \
    ../../src/irc/IrcParser.cc \
    ../../src/irc/IrcPool.cc \
    ../../src/irc/IrcUser.cc \
    ../../src/irc/nethelper.cc \
    ../../src/irc/win32.cc

HEADERS += ../../src/irc/config_structs.h \
    ../../src/irc/irc_channel_modes.h \
    ../../src/irc/irc_status.h \
    ../../src/irc/irc_structs.h \
    ../../src/irc/irc_user_modes.h \
    ../../src/irc/IrcChannel.h \
    ../../src/irc/IrcConnection.h \
    ../../src/irc/ircd_bahamut.h \
    ../../src/irc/ircd_ngircd.h \
    ../../src/irc/IrcEngine.h \
    ../../src/irc/IrcFactory.h \
    ../../src/irc/IrcListener.h \
    ../../src/irc/IrcNetwork.h \
    ../../src/irc/IrcObject.h \
    ../../src/irc/IrcParser.h \
    ../../src/irc/IrcPool.h \
    ../../src/irc/IrcUser.h \
    ../../src/irc/live_structs.h \
    ../../src/irc/nethelper.h \
    ../../src/irc/rfc1459.h \
    ../../src/irc/rfc2812.h
