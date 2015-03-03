/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QHeaderView>
#include <QMainWindow>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>
#include "../ext/qcustomplot/qcustomplot.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *main_area;
    QVBoxLayout *verticalLayout;
    QCustomPlot *chart_surface;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        //if (MainWindow->objectName().isEmpty())
        //    MainWindow->setObjectName(QString::fromUtf8("MainWindow"));

        //MainWindow->resize(548, 420);
        MainWindow->setContentsMargins(0, 0, 0, 0);

        //centralWidget = new QWidget(MainWindow);
        // //centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        //verticalLayout = new QVBoxLayout(centralWidget);
        main_area = new QWidget(MainWindow);
        verticalLayout = new QVBoxLayout(main_area);

        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        //verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetMaximumSize);
        //verticalLayout->setContentsMargins(0, 0, 0, 0);

        chart_surface = new QCustomPlot(main_area);
        chart_surface->setObjectName(QString::fromUtf8("chart_surface"));

        verticalLayout->addWidget(chart_surface);

        MainWindow->setCentralWidget(main_area);


        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        MainWindow->showMaximized();

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        //MainWindow->setWindowTitle(QApplication::translate("MainWindow", "QCustomPlot plot examples", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TG_Main_Window: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
