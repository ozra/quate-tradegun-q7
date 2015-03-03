#
#  QCustomPlot Plot Examples
#

TEMPLATE = app
TARGET = tradegun-q7

# CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS +=  -std=c++11

QT      = core
QT      += gui
QT      += network

lessThan(QT_MAJOR_VERSION, 5): error("requires Qt 5")
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport


SOURCES +=\
           ext/qcustomplot/qcustomplot.cpp \
    src/tradegun_study.cc \
    src/tg_main_window.cc \
    src/main.cc

HEADERS  += \
            ext/qcustomplot/qcustomplot.h \
    src/tradegun_study.hh \
    src/tg_main_window.hh \
    src/tg_main_window_ui.hh

# FORMS    +=

