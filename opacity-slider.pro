QT += core gui widgets dbus
CONFIG += c++17
CONFIG -= app_bundle

TARGET = ghosty
TEMPLATE = app

isEmpty(PREFIX): PREFIX = /usr/local
isEmpty(BINDIR): BINDIR = $$PREFIX/bin
isEmpty(DATADIR): DATADIR = $$PREFIX/share/ghosty

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += "GHOSTY_DATA_DIR=\\\"$$DATADIR/assets\\\""

SOURCES += \
    src/main.cpp \
    src/controller.cpp \
    src/icon.cpp \
    src/tray.cpp

HEADERS += \
    src/controller.h \
    src/icon.h \
    src/tray.h

target.path = $$BINDIR
INSTALLS += target

assets.files = assets/*
assets.path = $$DATADIR/assets
INSTALLS += assets

desktop.files = packaging/ghosty.desktop
desktop.path = $$PREFIX/share/applications
INSTALLS += desktop

autostart.files = packaging/ghosty-autostart.desktop
autostart.path = /etc/xdg/autostart
INSTALLS += autostart

icon_22.files = packaging/icons/22x22/apps/ghosty.png
icon_22.path = $$PREFIX/share/icons/hicolor/22x22/apps
INSTALLS += icon_22

icon_32.files = packaging/icons/32x32/apps/ghosty.png
icon_32.path = $$PREFIX/share/icons/hicolor/32x32/apps
INSTALLS += icon_32

icon_48.files = packaging/icons/48x48/apps/ghosty.png
icon_48.path = $$PREFIX/share/icons/hicolor/48x48/apps
INSTALLS += icon_48

icon_64.files = packaging/icons/64x64/apps/ghosty.png
icon_64.path = $$PREFIX/share/icons/hicolor/64x64/apps
INSTALLS += icon_64

icon_128.files = packaging/icons/128x128/apps/ghosty.png
icon_128.path = $$PREFIX/share/icons/hicolor/128x128/apps
INSTALLS += icon_128

icon_256.files = packaging/icons/256x256/apps/ghosty.png
icon_256.path = $$PREFIX/share/icons/hicolor/256x256/apps
INSTALLS += icon_256

icon_scalable.files = packaging/icons/scalable/apps/ghosty.svg
icon_scalable.path = $$PREFIX/share/icons/hicolor/scalable/apps
INSTALLS += icon_scalable