QT += core gui widgets dbus
CONFIG += c++17
CONFIG -= app_bundle

TARGET = opacity-slider
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    src/main.cpp \
    src/controller.cpp \
    src/icon.cpp \
    src/tray.cpp

HEADERS += \
    src/controller.h \
    src/icon.h \
    src/tray.h