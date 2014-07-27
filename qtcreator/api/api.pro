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
	-include build_config.h
DEFINES += _DEBUG USING_LIBCONFIG USING_JSON_SPIRIT_RPC USING_BOOST_NET
DEPENDPATH += ../../src
INCLUDEPATH += ../../src

# for shm_open
LIBS += -lrt

#>>> libconfig
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
	# boost->openssl dependency
	INCLUDEPATH += ../../third-party/openssl
	LIBS += -L../../third-party/openssl/lib/ -lssl -lcrypto
}
#<<<

# Warning: assumes build directory sbi-all = '$project/qtcreator'
# library -> '$project/lib/linux_x86-64/debug/libapi.so'
# objects -> '$project/qtcreator/api/debug/obj/xxx.o'
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


SOURCES += ../../src/api/Allocator.cc \
    ../../src/api/Configuration.cc \
    ../../src/api/crash_handler.cc \
    ../../src/api/interfaces.cc \
    ../../src/api/Log.cc \
    ../../src/api/Runtime.cc \
    ../../src/api/sync_event.cc \
    ../../src/api/utils.cc \
    ../../src/api/utils_linux.cc \
    ../../src/api/utils_win.cc \
    ../../src/api/rpc_commands.cc \
    ../../src/api/RpcServer.cc \
    ../../src/api/RpcTable.cc \
    ../../src/api/JsonRpc.cc

HEADERS += ../../src/api/Allocator.h \
    ../../src/api/char_helper.h \
    ../../src/api/compiler.h \
    ../../src/api/Configuration.h \
    ../../src/api/crash_handler.h \
    ../../src/api/definitions.h \
    ../../src/api/interface.h \
    ../../src/api/interface_status.h \
    ../../src/api/interfaces.h \
    ../../src/api/Log.h \
    ../../src/api/modules.h \
    ../../src/api/Runtime.h \
    ../../src/api/sync_event.h \
    ../../src/api/Terminal.h \
    ../../src/api/types.h \
    ../../src/api/utils.h \
    ../../src/api/utils_linux.h \
    ../../src/api/utils_win.h \
    ../../src/api/version.h \
    ../../src/api/rpc_commands.h \
    ../../src/api/rpc_status.h \
    ../../src/api/RpcCommand.h \
    ../../src/api/RpcServer.h \
    ../../src/api/RpcTable.h \
    ../../src/api/JsonRpc.h
