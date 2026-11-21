QT += core gui quick quickcontrols2 testlib dbus network
CONFIG += c++17 testcase
TARGET = gist_tests
TEMPLATE = app

INCLUDEPATH += ../src ../core/include
LIBS += -L$$PWD/../core/target/release -lgist_core -lssl -lcrypto
QMAKE_RPATHDIR += $$PWD/../core/target/release

HEADERS += ../src/backend.h ../src/theme.h ../src/translator.h
SOURCES += gist_tests.cpp ../src/backend.cpp ../src/theme.cpp ../src/translator.cpp
