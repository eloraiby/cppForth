TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS  += -fno-rtti -fno-exceptions -fno-non-call-exceptions -fno-use-cxa-atexit -ffunction-sections -fdata-sections -fno-common -DBUILDING_STATIC
QMAKE_LFLAGS += -static-libgcc -static-libstdc++ -Wl,--gc-sections

SOURCES += main.cpp \
    forth.cpp \
    primitives.cpp \
    base.cpp

HEADERS += \
    forth.hpp \
    hash_map.hpp \
    base.hpp \
    string.hpp \
    vector.hpp \
    intrusive-ptr.hpp
