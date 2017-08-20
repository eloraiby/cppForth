TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
#  -fno-non-call-exceptions -fno-use-cxa-get-exception-ptr
QMAKE_CXXFLAGS  += -D_HAS_EXCEPTION=0 -fno-rtti -fno-exceptions -fno-use-cxa-atexit -ffunction-sections -fdata-sections -fno-common -DBUILDING_STATIC
QMAKE_LFLAGS += -Wl,--gc-sections #-static -static-libgcc

QMAKE_LINK  = gcc

SOURCES += main.cpp \
    primitives.cpp \
    base.cpp \
    streams.cpp \
    mingw_fix.c \
    terminal.cpp \
    vm.cpp

HEADERS += \
    forth.hpp \
    hash_map.hpp \
    base.hpp \
    string.hpp \
    vector.hpp \
    intrusive-ptr.hpp \
    vm.hpp

DISTFILES += \
    bootstrap.f
