QT += core gui widgets serialport printsupport

CONFIG += c++17

TARGET = monitor
TEMPLATE = app

INCLUDEPATH += \
    src \
    qcustomplot

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/sampleparser.cpp \
    src/adcplot.cpp \
    qcustomplot/qcustomplot.cpp

HEADERS += \
    src/mainwindow.h \
    src/sampleparser.h \
    src/adcplot.h \
    src/adc.h \
    qcustomplot/qcustomplot.h
