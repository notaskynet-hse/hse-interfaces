QT += core gui widgets serialport

CONFIG += c++17

TARGET = terminal
TEMPLATE = app

INCLUDEPATH += \
    src \
    src/formats \
    src/history

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/history/datahistory.cpp \
    src/history/historyview.cpp \
    src/formats/formatregistry.cpp \
    src/formats/asciiformat.cpp \
    src/formats/numericformat.cpp

HEADERS += \
    src/mainwindow.h \
    src/history/datahistory.h \
    src/history/historyview.h \
    src/formats/dataformat.h \
    src/formats/formatregistry.h \
    src/formats/asciiformat.h \
    src/formats/numericformat.h \
    src/formats/formatvalidator.h
