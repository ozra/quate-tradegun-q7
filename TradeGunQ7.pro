#
#  QCustomPlot Plot Examples
#

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = tradegun-q7
TEMPLATE = app

SOURCES += src/main.cpp\
           src/mainwindow.cpp \
           ext/qcustomplot/qcustomplot.cpp

HEADERS  += src/mainwindow.h \
            ext/qcustomplot/qcustomplot.h \
    src/mainwindow_ui.h

FORMS    +=

