/***************************************************************************
**                                                                        **
**  QCustomPlot, an easy to use, modern plotting widget for Qt            **
**  Copyright (C) 2011, 2012, 2013, 2014 Emanuel Eichhammer               **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Emanuel Eichhammer                                   **
**  Website/Contact: http://www.qcustomplot.com/                          **
**             Date: 11.10.14                                             **
**          Version: 1.3.0-beta                                           **
****************************************************************************/

/************************************************************************************************************
**                                                                                                         **
**  This is the example code for QCustomPlot.                                                              **
**                                                                                                         **
**  It demonstrates basic and some advanced capabilities of the widget. The interesting code is inside     **
**  the "setup(...)Demo" functions of TG_Main_Window.                                                          **
**                                                                                                         **
**  In order to see a demo in action, call the respective "setup(...)Demo" function inside the             **
**  TG_Main_Window constructor. Alternatively you may call setupDemo(i) where i is the index of the demo       **
**  you want (for those, see TG_Main_Window constructor comments). All other functions here are merely a       **
**  way to easily create screenshots of all demos for the website. I.e. a timer is set to successively     **
**  setup all the demos and make a screenshot of the window area and save it in the ./screenshots          **
**  directory.                                                                                             **
**                                                                                                         **
*************************************************************************************************************/

#ifndef TG_Main_Window_H
#define TG_Main_Window_H

#include <QMainWindow>
#include <QTimer>

// // // // // // // //
// For data fetching
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

//#include <QWebSettings>

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>
#include <QVariantMap>
// // // // // // // //

#include "../ext/qcustomplot/qcustomplot.h" // the header file of QCustomPlot. Don't forget to add it to your project, if you use an IDE, so it gets compiled.



namespace Ui {
class TG_Main_Window;
}

class TG_Main_Window : public QMainWindow
{
    Q_OBJECT

public:
    explicit TG_Main_Window(QWidget *parent = 0);
    ~TG_Main_Window();

    void setupTGQ7(QCustomPlot *customPlot);

    void select_pane(int ix);

private slots:
    void screen_shot();

    void reload_data();
    void httpReplyHandler(QNetworkReply *);
    void send_panic_signal();
    void send_save_command();

    void toggle_legend();
    void cycle_pane();
    void cycle_pane_reverse();

    void step_up();
    void step_down();
    void zoom_in_value();
    void zoom_out_value();
    void auto_zoom_value();

    void step_left();
    void step_right();
    void jump_left();
    void jump_right();
    void zoom_in_time();
    void zoom_out_time();

private:
    Ui::TG_Main_Window *ui;
    //QString demoName;
    QTimer dataTimer;
    //QCPItemTracer *itemDemoPhaseTracer;
    //int currentDemoIndex;
    bool automatic_value_zoom = true;
    bool chart_study_established_ = false;
    int selected_pane_ix_;
    QCPAxisRect * selected_pane_;
};

#endif // TG_Main_Window_H
