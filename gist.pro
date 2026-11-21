QT += core gui qml quick quickcontrols2 dbus network

CONFIG += c++17 release
TARGET = gist
TEMPLATE = app

INCLUDEPATH += core/include
LIBS += -L$$PWD/core/target/release -lgist_core -lssl -lcrypto
QMAKE_RPATHDIR += $$PWD/core/target/release

HEADERS += \
    src/backend.h \
    src/theme.h \
    src/translator.h

SOURCES += \
    src/main.cpp \
    src/backend.cpp \
    src/theme.cpp \
    src/translator.cpp

RESOURCES += src/resources.qrc
