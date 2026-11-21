QT += core gui quick quickcontrols2 testlib dbus network
CONFIG += c++17 testcase
TARGET = gist_tests
TEMPLATE = app

INCLUDEPATH += ../src
HEADERS += ../src/backend.h ../src/theme.h ../src/translator.h
SOURCES += gist_tests.cpp ../src/backend.cpp ../src/theme.cpp ../src/translator.cpp
