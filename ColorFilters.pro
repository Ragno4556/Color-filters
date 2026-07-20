QT += widgets

CONFIG += c++17

SOURCES += \
    cpp/ColorController.cpp \
    cpp/ConfigManager.cpp \
    cpp/INIManager.cpp \
    cpp/Monitor.cpp \
    cpp/MonitorManager.cpp \
    cpp/main.cpp \
    cpp/mainwindow.cpp

HEADERS += \
    h/ColorController.h \
    h/ConfigManager.h \
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

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    form/style.qss \
    resources/Apps-colors-icon_31805.ico

RESOURCES += \
    resources.qrc

win32 {
    LIBS += -lgdi32 -luser32
}