TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -fdata-sections -ffunction-sections -fno-exceptions -std=c++11
QMAKE_LFLAGS += -static -static-libgcc -static-libstdc++ -Wl,--gc-sections

SOURCES += main.cpp \
    forth.cpp \
    primitives.cpp \
    base.cpp

HEADERS += \
    forth.hpp \
    hash_map.hpp \
    base.hpp \
    string.hpp \
    vector.hpp
