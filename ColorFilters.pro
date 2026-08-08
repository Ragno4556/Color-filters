QT += widgets

TARGET = ColorFilters
VERSION = 1.0.0

CONFIG += c++17 warn_on
DEFINES += NOMINMAX WIN32_LEAN_AND_MEAN

SOURCES += \
    cpp/ColorController.cpp \
    cpp/INIManager.cpp \
    cpp/Monitor.cpp \
    cpp/MonitorManager.cpp \
    cpp/main.cpp \
    cpp/mainwindow.cpp

HEADERS += \
    h/ColorController.h \
    h/FilterSettings.h \
    h/INIManager.h \
    h/Monitor.h \
    h/MonitorManager.h \
    h/RGB.h \
    h/ini.h \
    h/mainwindow.h

FORMS += \
    form/mainwindow.ui

INCLUDEPATH += h

RESOURCES += \
    resources.qrc

win32 {
    LIBS += -lgdi32 -luser32
}
