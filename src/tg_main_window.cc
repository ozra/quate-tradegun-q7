#include "tg_main_window.hh"
#include "tg_main_window_ui.hh"
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>
#include <QMetaEnum>
#include <QShortcut>
#include <algorithm>    // for std::numeric_limits<> (it's here yes? no net connection to verify..)
#include <typeinfo>


/*
*TODO*
- line width
- better scatter support
- click on pane, sets focus on it
- more intuitive scrolling / zooming (arrows MOVE, CTRL + arrows zoom.. or something..)
*/


/*
 *

    *TODO* - cross hair!

    connect(the-plot, SIGNAL(mousePress(QMouseEvent*)), SLOT(myMouseHandler(QMouseEvent*)));

    theClass::myMouseHandler(QMouseEvent* e) {
        QPoint p = e->pos();

        if (! e->keyBoardModifiers(Qt::ControlModifier eller Qt::Key_Control eller så)) {
            return;
        }

        if (!g_the_muddafuckin_tracer) {
            g_the_muddafuckin_tracer = new QCPItemTracer(the-plot);
        }
        QCPItemTracer* tracer = g_the_muddafuckin_tracer;

        tracer->setStyle(QCP::tsCrossHair);
        tracer->setGraph(the-graph-to-align-to);

        double io_key, io_val;
        time-pos = graph->the-axis->pixelsToCoords(p, io_key, io_val);
        tracer->setGraphKey(io_key);

    }

/ *
 *
 *  NOTES - 2014-12-22 - Oscar Campbell
 *
 * beforeReplot - is a good signal to touch up zooming etc after user move/scale operations - before actualizing it
 *
 *
 *
 */

// *TODO* *TEMP* *BAD*!!!!
QCPLayer* G__marker_layer = nullptr;


struct TG_Chart_Navigation_Ranges {
    double  visible_range_threshold;
    double  tick_step;
    int     sub_ticks;
    double  time_navigation_step;
};

// *TODO* *MEGAFOO* *TEMP*
QVector<TG_Chart_Navigation_Ranges>  G__navigation_ranges;



QColor invert_color(QColor color) {
    /*
    qreal h, s, v, a;
    color.toHsv().getHsvF(&h,&s,&v,&a);
    qDebug() << "invert_color:" << h << ", " << s << ", " << v << ", " << a << ", new v=" << (1.0 - v);
    v = 0.2 * v;
    s = 0.8 * s;
    return QColor::fromHsvF(h,s,v,a).toRgb();
    */

    qreal r, g, b, a;
    color.getRgbF(&r, &g, &b, &a);
    //return QColor::fromRgbF(1-r, 1-g, 1-b, a);
    return QColor::fromRgbF(1-g, 1-r, 1-b, a);
}



TG_Main_Window::TG_Main_Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TG_Main_Window)
{
    ui->setupUi(this);
    setGeometry(50, 50, 1142, 790);

    setupTGQ7(ui->chart_surface);
    setWindowTitle("TradeGunQ7");
    statusBar()->clearMessage();
    ui->chart_surface->replot();

}

TG_Main_Window::~TG_Main_Window()
{
    delete ui;
}









// Util funcs to retrieve axes more cleanly - 2015-01-16/ORC
inline QCPAxis * xaxis(QCPAxisRect * pane) {
    return pane->axis(QCPAxis::atBottom);
}

inline QCPAxis * yaxis(QCPAxisRect * pane) {
    return pane->axis(QCPAxis::atLeft);
}

inline QCPAxis * x2axis(QCPAxisRect * pane) {
    return pane->axis(QCPAxis::atTop);
}

inline QCPAxis * y2axis(QCPAxisRect * pane) {
    return pane->axis(QCPAxis::atRight);
}

void reconfigure_panes(QCustomPlot* chart, TG_Chart_Navigation_Ranges& nav_ranges) {
    auto panes = chart->axisRects();

    for (auto pane : panes) {
        //xaxis(pane)->setAutoTicks(false);
        //xaxis(pane)->setAutoSubTicks(false);
        xaxis(pane)->setAutoTickStep(false);
        xaxis(pane)->setTickStep(nav_ranges.tick_step);
        xaxis(pane)->setSubTickCount(nav_ranges.sub_ticks);

        // *TODO* for the y-axis we may want specifics for instance for "unit-normalized signals"
        yaxis(pane)->setAutoTickStep(true);
    }
}

void dual_connect_panes(QWidget * view, QCPAxis * axis1, QCPAxis * axis2) {
    view->connect(axis1, SIGNAL(rangeChanged(QCPRange)), axis2, SLOT(setRange(QCPRange)));
    view->connect(axis2, SIGNAL(rangeChanged(QCPRange)), axis1, SLOT(setRange(QCPRange)));
}

void set_pane_defaults(QCPAxisRect* pane, QCPMarginGroup* margin_group) {
    pane->setMarginGroup(QCP::msLeft|QCP::msRight, margin_group);
    pane->setMinimumMargins(QMargins(0,0,0,0));
    pane->setMargins(QMargins(0,0,0,0));
    pane->setAutoMargins(QCP::msLeft);
    //pane->setAntialiased(false);

    auto x_axis = xaxis(pane); //->axis(QCPAxis::atBottom);
    x_axis->setTickLabelType(QCPAxis::ltDateTime);
    x_axis->setDateTimeSpec(Qt::UTC);
    x_axis->setDateTimeFormat("dd. H:m:s");
    //x_axis->setDateTimeFormat("dd. MMM");
    x_axis->setVisible(true);
    x_axis->setTickLabels(false);
}

QCPAxisRect* add_chart_pane(QWidget * view, QCustomPlot* chart, int row_ix, int min_height, QCPMarginGroup* margin_group) {
    QColor bg_color = QColor::fromRgbF(0, 0, 0, 1);
    QPen bg_grid_color = QPen(QColor::fromRgbF(0.01, 0.01, 0.01, 0.5));
    QCPAxisRect* pane;

    if (row_ix == 0) {  // default pane, already available
        pane = chart->axisRect();
        set_pane_defaults(pane, margin_group);
        xaxis(pane)->setTickLabels(true);

    } else {
        pane = new QCPAxisRect(chart);
        set_pane_defaults(pane, margin_group);
        chart->plotLayout()->addElement(row_ix, 0, pane);
        dual_connect_panes(view, xaxis(pane), chart->xAxis);
    }

    pane->setMinimumSize(50, min_height);
    pane->setBackground(bg_color);

    QCPAxis* axis;
    axis = xaxis(pane);
    axis->setBasePen(bg_grid_color);
    axis->setTickPen(bg_grid_color);
    axis->setSubTickPen(bg_grid_color);
    axis->setSelectedBasePen(bg_grid_color);
    axis->setSelectedTickPen(bg_grid_color);
    axis->setSelectedSubTickPen(bg_grid_color);

    axis = yaxis(pane);
    axis->setBasePen(bg_grid_color);
    axis->setTickPen(bg_grid_color);
    axis->setSubTickPen(bg_grid_color);
    axis->setSelectedBasePen(bg_grid_color);
    axis->setSelectedTickPen(bg_grid_color);
    axis->setSelectedSubTickPen(bg_grid_color);

    return pane;
}

double rnd(double max) {
    return (qrand() / (double) RAND_MAX) * max;
}

inline void add_drawing_arrow(QCustomPlot *chart, QCPAxisRect* pane, double sx, double sy, double ex, double ey, const QPen & pen) {
    auto *arrow = new QCPItemLine(chart);

    // *TODO*

    //auto lay = pane->layer();   // chart->layer(0);
    arrow->setLayer(G__marker_layer);
    arrow->setPen(pen);

    arrow->setClipAxisRect(pane);
    //arrow->setClipToAxisRect(false);
    arrow->start->setAxisRect(pane);
    arrow->start->setAxes(xaxis(pane), yaxis(pane));
    arrow->start->setCoords(sx, sy); // setParentAnchor(textLabel->bottom);
    arrow->end->setAxisRect(pane);
    arrow->end->setAxes(xaxis(pane), yaxis(pane));
    arrow->end->setCoords(ex, ey);
    arrow->setHead(QCPLineEnding::esSpikeArrow);

    chart->addItem(arrow);

}
/*
void addDrawLayer(QCustomPlot *chart, QCPAxisRect* pane, QVector<double> times, QVector<double> values) {
    auto i = rnd(10);
    auto len = times.size();

    //QPen npen(QColor(50, 60, 85));
    QPen xpen(QColor(50, 90, 55));
    //QPen rpen(QColor(90, 60, 55));

    while(i < len) {
        int end = i + rnd(1) + 30;
        if (end >= len)
            break;

        auto ratio = (values[end] - values[i]) /  (times[end] - times[i]);
        auto duration = (times[end] - times[end-1]);
        double extend = (rnd(150) + 50) * duration;

        // add the arrow:
        add_drawing_arrow(chart, pane, times[i], values[i], times[end] + extend, values[end] + (ratio * 2 * extend), xpen);
        //add_drawing_arrow(chart, times[i], values[i], times[end] + extend, values[end] + (ratio * extend), npen);
        //add_drawing_arrow(chart, times[i], values[i], times[end] + extend, values[end] + (-ratio * extend), rpen);

        i += rnd(50);
    }
}
*/





QCPAbstractPlottable* setup_line_plot(QCustomPlot* chart, QCPAxisRect* pane, QJsonArray& series_conf) {
    qDebug() << "Linish plot";

    // *TODO* right-axis or screen-axis (no axis)
    auto x_axis = xaxis(pane);
    auto y_axis = yaxis(pane);
    auto color = invert_color(QColor(QString("#") + series_conf[2].toString()));
    int line_style = series_conf[4].toInt();
    int scatter_style = series_conf[0].toInt();
    int scatter_size = 5;

    QCPGraph* graph = new QCPGraph(x_axis, y_axis);
    chart->addPlottable(graph);

    QPen pen;
    pen.setColor(color); //QColor(qSin(foooo_counter*1+1.2)*80+80, qSin(foooo_counter*0.3+0)*80+80, qSin(foooo_counter*0.3+1.5)*80+80));
    graph->setPen(pen);
    graph->setName(series_conf[1].toString());

    graph->setLineStyle(
        line_style == 0 ? QCPGraph::lsNone :
        QCPGraph::lsLine
    );

    graph->setScatterStyle(QCPScatterStyle(
        scatter_style == 0 ? QCPScatterStyle::ssNone :
        scatter_style == 1 ? QCPScatterStyle::ssCircle :
        scatter_style == 2 ? QCPScatterStyle::ssDiamond :
        scatter_style == 3 ? QCPScatterStyle::ssSquare :
        scatter_style == 4 ? QCPScatterStyle::ssCross :
        scatter_style == 5 ? QCPScatterStyle::ssPlus :
        scatter_style == 6 ? QCPScatterStyle::ssStar :
        //scatter_style == 7 ? QCPScatterStyle::ssTriangle :
        //scatter_style == 8 ? QCPScatterStyle::ssTriangleInverted :
        //scatter_style == 9 ? QCPScatterStyle::ssCross :
        QCPScatterStyle::ssDisc
    , scatter_size
    ));

    return static_cast<QCPAbstractPlottable*>(graph);
}

QCPAbstractPlottable* setup_bar_plot(QCustomPlot* chart, QCPAxisRect* pane, QJsonArray& series_conf) {



    return setup_line_plot(chart, pane, series_conf);


    // *TODO*

    /*
    // create bottom axis rect for volume bar chart:
    //QCPAxisRect *pane = new QCPAxisRect(chart);
    pane->setMaximumSize(QSize(QWIDGETSIZE_MAX, 100));
    xaxis(pane)->setLayer("axes");
    xaxis(pane)->grid()->setLayer("grid");

    // bring bottom and main axis rect closer together:
    */
    chart->plotLayout()->setRowSpacing(0);
    pane->setAutoMargins(QCP::msLeft|QCP::msRight|QCP::msBottom);
    pane->setMargins(QMargins(0, 0, 0, 0));

    QCPBars* bars = new QCPBars(xaxis(pane), yaxis(pane));
    /*
      *TODO* continue here...
    chart->setAutoAddPlottableToLegend(false);
    chart->addPlottable(volumeUp);
    chart->addPlottable(volumeDown);
    bars->setWidth(3600 * 0.9);
    bars->setPen(Qt::NoPen);
    bars->setBrush(QColor(100, 180, 110));
    */

    // create two bar plottables, for positive (green) and negative (red) volume bars:
    /*
    QCPBars *volumeUp = new QCPBars(xaxis(pane), yaxis(pane));
    QCPBars *volumeDown = new QCPBars(xaxis(pane), yaxis(pane));
    auto n = times.size();
    for (int i = 0; i < n / 1; ++i) {
        int v = qrand() % 20000 + qrand() % 20000 + qrand() % 20000 - 10000 * 3;
        (v < 0 ? volumeDown : volumeUp)->addData(times[0] + 3600 * 1.0 * i, qAbs(v)); // add data to either volumeDown or volumeUp, depending on sign of v
    }
    chart->setAutoAddPlottableToLegend(false);
    chart->addPlottable(volumeUp);
    chart->addPlottable(volumeDown);
    volumeUp->setWidth(3600 * 0.9);
    volumeUp->setPen(Qt::NoPen);
    volumeUp->setBrush(QColor(100, 180, 110));
    volumeDown->setWidth(3600 * 0.9);
    volumeDown->setPen(Qt::NoPen);
    volumeDown->setBrush(QColor(180, 90, 90));
    */

    return dynamic_cast<QCPAbstractPlottable*>(bars);
}

QCPAbstractPlottable* setup_zone_plot(QCustomPlot* chart, QCPAxisRect* pane, QJsonArray& series_conf) {
    qDebug() << "Zone Plot";
    QCPAbstractPlottable* graph = nullptr;
    return graph;
}
QCPAbstractPlottable* setup_ohlc_plot(QCustomPlot* chart, QCPAxisRect* pane, QJsonArray& series_conf) {
    qDebug() << "OHLC Plot";
    //QCPAbstractPlottable* graph = nullptr;

    //QCPFinancialDataMap map;

    auto x_axis = xaxis(pane);
    auto y_axis = yaxis(pane);

    auto up_body_color = invert_color(QColor(QString("#") + series_conf[2].toString()));
    auto down_body_color= invert_color(QColor(QString("#") + series_conf[3].toString()));
    auto up_wick_color = invert_color(QColor(QString("#") + series_conf[4].toString()));
    auto down_wick_color= invert_color(QColor(QString("#") + series_conf[5].toString()));
    int line_width = series_conf[6].toInt();

    QCPFinancial* graph = new QCPFinancial(x_axis, y_axis);
    chart->addPlottable(graph);
    graph->setName(series_conf[1].toString());

    auto THE_CANDLE_DURATION = 60.0;    // *TODO* add to series plot conf
    graph->setWidth(THE_CANDLE_DURATION * 0.9);
    graph->setChartStyle(QCPFinancial::csCandlestick);
    graph->setTwoColored(true);
    graph->setBrushPositive(QBrush(up_body_color));
    graph->setBrushNegative(QBrush(down_body_color));
    graph->setPenPositive(QPen(up_wick_color));
    graph->setPenNegative(QPen(down_wick_color));

    return static_cast<QCPAbstractPlottable*>(graph);
}
QCPAbstractPlottable* setup_graphic_plot(QCustomPlot* chart, QCPAxisRect* pane, QJsonArray& series_conf) {
    qDebug() << "Arrow (marker / drawing / graphic).";
    delete G__marker_layer;
    auto plot_name = series_conf[1].toString();
    chart->addLayer(plot_name);
    G__marker_layer = chart->layer(plot_name);
    QCPAbstractPlottable* graph = nullptr;
    return graph;
}

void setup_plot_series(QCustomPlot* chart, QCPAxisRect* pane, QJsonObject& series_root) {
    QJsonArray series_conf = series_root["conf"].toArray();
    // *TODO* remove data - is done in another pass
    QJsonArray quant_data = series_root["data"].toArray();
    QCPAbstractPlottable* graph;
    qDebug() << "Sets up series" << series_conf[1].toString() << "\n";

    switch (series_conf[0].toInt()) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        graph = setup_line_plot(chart, pane, series_conf);
        break;
    case 7:
        graph = setup_zone_plot(chart, pane, series_conf);
        break;
    case 8:
        graph = setup_ohlc_plot(chart, pane, series_conf);
        break;
    case 9:
        qDebug() << "OHLCM not supported atm.";
        exit(1);
        break;
    case 10:
        graph = setup_bar_plot(chart, pane, series_conf);
        break;
    case 11:
        graph = setup_graphic_plot(chart, pane, series_conf);
        break;
    }

    if (graph) {
        graph->rescaleAxes(true);
    }
}
void setup_plot_pane(QWidget * view, QCustomPlot* chart, QCPMarginGroup* margin_group, int row_ix, const QJsonObject& pane_conf) {
    QJsonArray series = pane_conf["seriesCollection"].toArray();
    int min_height = pane_conf["minHeight"].toInt();
    int max_height = pane_conf["maxHeight"].toInt();
    auto pane = add_chart_pane(view, chart, row_ix, min_height, margin_group);

    if (max_height > 0) {
        pane->setMaximumSize(10000, max_height);    // *TODO* the 10000 part...
    }

    qDebug() << "Sets up pane" << pane_conf["name"].toString() << ", " << series.count() << ", " << series.size() << "\n";
    for (int series_ix = 0; series_ix < series.size(); ++series_ix) {
        QJsonObject serie_root = series[series_ix].toObject();
        setup_plot_series(chart, pane, serie_root);
    }
}
void setup_full_plot(QWidget * view, QCustomPlot* chart, const QJsonObject& plot_conf) {
    QJsonArray panes = plot_conf["panes"].toArray();
    qDebug() << "Sets up plot panes: " << panes.count() << ", " << panes.size() << "\n";

    // make axis rects' left side line up:
    auto *margin_group = new QCPMarginGroup(chart);
    chart->legend->setFont(QFont("Helvetica", 9));
    //chart->setMargins(QMargins(0,0,0,0));

    chart->setAntialiasedElements(QCP::aeNone);

    auto p1_x_axis = chart->xAxis;
    //p1_x_axis->setBasePen(Qt::NoPen);

    for (int pane_ix = 0; pane_ix < panes.count(); ++pane_ix) {
        setup_plot_pane(view, chart, margin_group, pane_ix, panes[pane_ix].toObject());
    }

    // // // //
    auto nav_ranges = plot_conf["navigation_ranges"].toArray();

    for (int nav_ix = 0; nav_ix < nav_ranges.count(); ++nav_ix) {
        auto nav_set = nav_ranges[nav_ix].toArray();
        G__navigation_ranges.push_back(TG_Chart_Navigation_Ranges{ nav_set[0].toDouble(), nav_set[1].toDouble(), nav_set[2].toInt(), nav_set[3].toDouble() } );
    }

    reconfigure_panes(chart, G__navigation_ranges[2]);

}

void renew_data_full_plot(QWidget * view, QCustomPlot* chart, const QJsonObject& plot_conf) {
    QJsonArray panes = plot_conf["panes"].toArray();

    qDebug() << "panes " << panes.count();

    for (int pane_ix = 0; pane_ix < panes.count(); ++pane_ix) {
        auto pane = chart->axisRect(pane_ix);
        QJsonArray series = panes[pane_ix].toObject()["seriesCollection"].toArray();

        qDebug() << "pane " << pane_ix;
        qDebug() << "series " << series.count();

        // *TODO* remove all drawings.. - do this a more elegant way please... (cache added ones in an array for each gfx-plot-"stream"...)
        //for (auto item : pane->items()) {
        //}

        int plottable_ix = 0;

        for (int series_ix = 0; series_ix < series.size(); ++series_ix) {
            qDebug() << "serie " << series_ix;
            const QJsonObject series_root = series[series_ix].toObject();
            const QJsonArray series_conf = series_root["conf"].toArray();
            const QJsonArray quant_data = series_root["data"].toArray();
            qDebug() << "serie name: \"" << series_conf[1].toString() << "\"";
            auto plot_type = series_conf[0].toInt();

            if (plot_type < 7     ||   plot_type == 10 /* *TODO* */  ) {   // One value plotting?
                // *TODO* - we want to EXPAND on existing data later on...
                auto graph = static_cast<QCPGraph*>(pane->plottables()[plottable_ix++]);
                graph->clearData();

                //graph->setData(times, values);
                int quant_ix = 0, len = quant_data.size();
                qDebug() << "quant samples " << quant_data.count();

                while(quant_ix < len) {
                    graph->addData(quant_data[quant_ix].toDouble() * 0.001, quant_data[quant_ix + 1].toDouble());
                    quant_ix += 2;
                }
            }
            else if (plot_type == 8) {
                auto financial = static_cast<QCPFinancial*>(pane->plottables()[plottable_ix++]);
                financial->clearData();

                int quant_ix = 0, len = quant_data.size();
                qDebug() << "quant samples " << quant_data.count();

                while(quant_ix < len) {
                    financial->addData(quant_data[quant_ix].toDouble() * 0.001, quant_data[quant_ix + 1].toDouble(), quant_data[quant_ix + 2].toDouble(), quant_data[quant_ix + 3].toDouble(), quant_data[quant_ix + 4].toDouble());
                    quant_ix += 5;
                }

            }
            else if (plot_type == 11) {
                int quant_ix = 0, len = quant_data.size();
                qDebug() << "quant samples " << quant_data.count();

                while(quant_ix < len) {
                    auto gfx_type = quant_data[quant_ix++].toInt();

                    //qDebug() << "GFX is of type " << gfx_type;

                    if (gfx_type == 1 || gfx_type == 2) {   // ARROW, or LINE
                        auto x = quant_data[quant_ix++].toDouble() * 0.001;
                        auto y = quant_data[quant_ix++].toDouble();
                        auto x2 = quant_data[quant_ix++].toDouble() * 0.001;
                        auto y2 = quant_data[quant_ix++].toDouble();

                        auto color = invert_color(QColor(QString("#") + quant_data[quant_ix++].toString()));

                        QPen pen;
                        pen.setColor(color); //QColor(qSin(foooo_counter*1+1.2)*80+80, qSin(foooo_counter*0.3+0)*80+80, qSin(foooo_counter*0.3+1.5)*80+80));

                        //qDebug() << "Arrow/Line" << x << ", " << y << ", " << x2 << ", " << y2  << ", " << color;
                        add_drawing_arrow(chart, pane, x, y, x2, y2, pen);
                    }
                }
            }
        }

    }
}

void TG_Main_Window::httpReplyHandler(QNetworkReply * reply) {
    //mNpcs.clear();

    auto body = reply->readAll();
    qDebug() << "Got reply from http:"; // << body << "\n";

    if (reply->error()) {
        qDebug() << "Got error reply from http:" << reply->error() << "\n";
        //do shit and alert about the error and then return
        return;
    }

    qDebug() << "char at 0 == >>" << (int)body[0] << "<< >>" << (int)body[1] << "<<" << "\n";
    QJsonParseError json_err;
    QJsonDocument json = QJsonDocument::fromJson(body, &json_err);

    if (json_err.error) {
        qDebug() << "JSON ERROR: " << json_err.errorString() << " at " << json_err.offset << "\n";
    }

    QJsonObject json_obj = json.object();
    qDebug() << "JSON" << json_obj.keys() << "\n";

    if (json_obj["version"].toDouble() != 0.42) {
        qDebug() << "ERROR: wrong version of data format! : " << json_obj["version"].toDouble();
        return;
    }

    if (json_obj["success"].toBool() == true) {     // Some kind of simple ack...
        return;

    } else {
        if ( ! chart_study_established_) {
            setup_full_plot(this, ui->chart_surface, json_obj);
        }
        renew_data_full_plot(this, ui->chart_surface, json_obj);
        if ( ! chart_study_established_) {

            ui->chart_surface->rescaleAxes();
            ui->chart_surface->xAxis->scaleRange(1.025, ui->chart_surface->xAxis->range().center());
            ui->chart_surface->yAxis->scaleRange(1.1, ui->chart_surface->yAxis->range().center());

        }
        select_pane(0);
        auto_zoom_value();
        //ui->chart_surface->replot();
        chart_study_established_ = true;
    }

    qDebug() << "Done JSONing" << "\n";

}

void TG_Main_Window::reload_data() {

    // *TODO* if request is ongoing, cancel it, or queue??
    // *TODO* increase timeout time

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QObject::connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(httpReplyHandler(QNetworkReply *)));
    QUrl url("http://tradegun.x/study/run");
    QUrlQuery params;
    QNetworkRequest request(url);
    QByteArray post_data;

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    params.addQueryItem("study_id",     "budbrain_v1");
    params.addQueryItem("arch",         "V8-internal-strategies");
    params.addQueryItem("version",      "0.42");
    params.addQueryItem("start_date",   "2014-01-31 12:15");
    params.addQueryItem("end_date",     "2014-01-31 14:45");

    //url.setQuery(params);
    post_data.append(params.toString());
    manager->post(request, post_data); //.encodedQuery());
}

void TG_Main_Window::send_panic_signal() {
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QObject::connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(httpReplyHandler(QNetworkReply *)));
    QUrl url("http://tradegun.x/study/command");
    QUrlQuery params;
    QNetworkRequest request(url);
    QByteArray post_data;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    params.addQueryItem("study_id",     "budbrain_v1");
    params.addQueryItem("arch",         "V8-internal-strategies");
    params.addQueryItem("version",      "0.42");
    params.addQueryItem("cmd",          "panic");
    post_data.append(params.toString());
    manager->post(request, post_data);
}

void TG_Main_Window::send_save_command() {
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QObject::connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(httpReplyHandler(QNetworkReply *)));
    QUrl url("http://tradegun.x/study/command");
    QUrlQuery params;
    QNetworkRequest request(url);
    QByteArray post_data;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    params.addQueryItem("study_id",     "budbrain_v1");
    params.addQueryItem("arch",         "V8-internal-strategies");
    params.addQueryItem("version",      "0.42");
    params.addQueryItem("cmd",          "save");
    post_data.append(params.toString());
    manager->post(request, post_data);
}


//   *TODO*

/*
QCPFinancialDataMap read_ohlc_data_or_something_like_that()
{
    QCPFinancialDataMap map;
    //  *TODO*  requestDataRegionThroughZMQ()
    //char * buf = TheZmqData.front();
    char * buf = nullptr;

    while(still_data == true) {
        double theTimePoint = read_varlen_data(buf);
        map.insert(theTimePoint, QCPFinancialData(theTimePoint, read_varlen_data(buf), read_varlen_data(buf), read_varlen_data(buf), read_varlen_data(buf)));
    }

    return map;
}

QCPFinancialDataMap make_ohlc_from_hashtree(HashTree tree)
{
    QCPFinancialDataMap map;

    for(auto v : tree) {
        map.insert(tree.get_as<long>("time"), QCPFinancialData(tree.get_as<long>("time"), tree.get_as<long>("open"), tree.get_as<long>("high"), tree.get_as<long>("low"), tree.get_as<long>("close")));
    }

    return map;
}

*/



QCPRange get_weighted_viewable_value_range(QCPFinancialDataMap * data, const QCPRange x_range) {
    //if (fromKey >= toKey || mData->isEmpty()) return;

    double from_key = x_range.lower;
    double to_key = x_range.upper;

    double size = to_key - from_key;
    double outer_size = size / 3;
    double inv_outer_size = 1 / outer_size;
    double pre_from = from_key - outer_size;
    double post_to = to_key + outer_size;
    double weighting;

    double lowest = std::numeric_limits<double>::max();
    double highest = std::numeric_limits<double>::min();

    //MAP_T::const_iterator it;
    //MAP_T::const_iterator end;
    double value;

    QCPFinancialDataMap::const_iterator it;
    QCPFinancialDataMap::const_iterator end;

    // Loop visible interval
    for (it = data->upperBound(from_key), end = data->upperBound(to_key); it != end; ++it) {
        lowest = std::min(lowest, it.value().low);
        highest = std::max(highest, it.value().high);
    }

    // Loop 1/3 PRE visible interval and weight the values
    it = data->upperBound(pre_from);
    end = data->upperBound(from_key);
    while (it != end) {
        weighting = (outer_size - (end.value().key - it.value().key)) * inv_outer_size;
        lowest = std::min(lowest, it.value().low * weighting);
        highest = std::max(highest, it.value().high * weighting);
        ++it;
    }

    // Loop 1/3 POST visible interval and weight the values
    it = data->upperBound(to_key);
    end = data->upperBound(post_to);
    while (it != end) {
        weighting = (end.value().key - it.value().key) * inv_outer_size;
        lowest = std::min(lowest, it.value().low * weighting);
        highest = std::max(highest, it.value().high * weighting);
        ++it;
    }

    QCPRange range;
    range.lower = lowest;
    range.upper = highest;
    return range;
}

QCPRange get_weighted_viewable_value_range(QCPDataMap * data, const QCPRange x_range) {
    //if (fromKey >= toKey || mData->isEmpty()) return;
    const double from_key = x_range.lower;
    const double to_key = x_range.upper;
    const double size = to_key - from_key;
    const double outer_size = size / 5.0;
    const double inv_outer_size = 1.0 / outer_size;
    const double pre_from = from_key - outer_size;
    const double post_to = to_key + outer_size;

    double weighting;
    double inv_weighting;
    double lowest = std::numeric_limits<double>::max();
    double highest = std::numeric_limits<double>::min();

    //MAP_T::const_iterator it;
    //MAP_T::const_iterator end;
    double value;

    QCPDataMap::const_iterator it;
    QCPDataMap::const_iterator end;

    // Standard loop - the visible interval
    for (it = data->lowerBound(from_key), end = data->upperBound(to_key); it != end; ++it) {
        lowest = std::min(lowest, it.value().value);
        highest = std::max(highest, it.value().value);
    }

    /*
    const double lowest_visible = lowest;
    const double highest_visible = highest;

    // Loop PRE visible interval and weight the values
    it = data->lowerBound(pre_from);
    end = data->upperBound(from_key);
    while (it != end) {
        //qDebug() << "weighted range, pre: it = " << it.value().value;
        weighting = (outer_size - (end.value().key - it.value().key)) * inv_outer_size;
        inv_weighting = 1 - weighting;
        lowest = std::min(lowest, it.value().value * weighting + lowest_visible * inv_weighting);
        highest = std::max(highest, it.value().value * weighting + highest_visible * inv_weighting);
        ++it;
    }

    // Loop POST visible interval and weight the values
    //it = std::min(data->upperBound(to_key), data->end());
    //end = std::max(data->upperBound(post_to), data->end());
    it = data->upperBound(to_key);
    end = data->upperBound(post_to);
    while (it != end) {
        weighting = (end.value().key - it.value().key) * inv_outer_size;
        inv_weighting = 1 - weighting;

        double weighted_low = it.value().value * weighting + lowest_visible * inv_weighting;

        if (weighted_low < 1100) {
            //qDebug() << "weighted range, post: it = " << it.value().value << " weighted = " << weighted_low;
            qDebug() << data->upperBound(to_key).value().value << ", to-key: " << data->upperBound(post_to).value().key << ", to-val: " << data->upperBound(post_to).value().value << ", size: " << outer_size << ", x-delta: " << (end.value().key - it.value().key) << ", weighting: " << weighting;
        }

        lowest = std::min(lowest, weighted_low);
        highest = std::max(highest, it.value().value * weighting + highest_visible * inv_weighting);
        ++it;
    }
    */

    QCPRange range;
    range.lower = lowest;
    range.upper = highest;
    return range;
}

void do_auto_zoom(QCustomPlot * chart) {
    //qDebug() << "do_auto_zoom()";

    for (auto i = 0; i < chart->axisRectCount(); ++i) {
        auto pane = chart->axisRect(i);
        QCPRange total_weighted_range;
        total_weighted_range.lower = std::numeric_limits<double>::max();
        total_weighted_range.upper = std::numeric_limits<double>::min();
        auto x_range = xaxis(pane)->range();

        for (auto i2 = 0; i2 < pane->graphs().size(); ++i2) {
            auto org_graph = pane->graphs()[i2];
            QCPRange weighted_range;
            QCPGraph * generic_graph;
            QCPFinancial * finance_graph;

            if ((finance_graph = dynamic_cast<QCPFinancial *>(org_graph))) {
                weighted_range = get_weighted_viewable_value_range(finance_graph->data(), x_range);
            }
            else if ((generic_graph = dynamic_cast<QCPGraph *>(org_graph))) {
                weighted_range = get_weighted_viewable_value_range(generic_graph->data(), x_range);
            }
            total_weighted_range.upper = std::max(total_weighted_range.upper, weighted_range.upper);
            total_weighted_range.lower = std::min(total_weighted_range.lower, weighted_range.lower);
        }

        double range_delta = total_weighted_range.upper - total_weighted_range.lower;
        double value_margin_top = 1.0 / 100.0;
        double value_margin_bottom = 1.0 / 100.0;

        //qDebug() << "do_auto_zoom: Final vals: " << total_weighted_range.upper << ", " << total_weighted_range.lower;

        total_weighted_range.upper += value_margin_top * range_delta;
        total_weighted_range.lower -= value_margin_bottom * range_delta;

        //qDebug() << "do_auto_zoom: Final margined vals: " << total_weighted_range.upper << ", " << total_weighted_range.lower << "\n\n";

        yaxis(pane)->setRange(total_weighted_range);
    }
}


void change_value_zooming(QCustomPlot *chart, QCPAxisRect* pane, double zooming) {
    //chart->yAxis->scaleRange(zooming, chart->yAxis->range().center());
    yaxis(pane)->scaleRange(zooming, yaxis(pane)->range().center());
    chart->replot();
}
void change_value_position(QCustomPlot *chart, QCPAxisRect *pane, double delta) {
    auto range = yaxis(pane)->range();
    range.lower += delta;
    range.upper += delta;
    yaxis(pane)->setRange(range);
    chart->replot();
}

void change_time_zooming(QCustomPlot *chart, QCPAxisRect* pane, double zooming, bool auto_zoom) {
    xaxis(pane)->scaleRange(zooming, xaxis(pane)->range().center());
    if (auto_zoom) {
        do_auto_zoom(chart);
        //chart->yAxis->rescale();
        //chart->yAxis->scaleRange(1, chart->yAxis->range().center());
    }
    chart->replot();
}
void change_time_position(QCustomPlot *chart, double delta, bool auto_zoom) {
    chart->xAxis->setRange(chart->xAxis->range().lower + delta, chart->xAxis->range().size(), Qt::AlignLeft);
    if (auto_zoom) {
        do_auto_zoom(chart);
        //chart->yAxis->rescale();
        //chart->yAxis->scaleRange(1, chart->yAxis->range().center());
    }
    chart->replot();
}

void TG_Main_Window::select_pane(int ix) {
    selected_pane_ix_ = ix;
    selected_pane_ = ui->chart_surface->axisRect(selected_pane_ix_);
}

void TG_Main_Window::toggle_legend() {
    auto chart = ui->chart_surface;
    chart->legend->setVisible(!chart->legend->visible());
    chart->replot();
}

void TG_Main_Window::cycle_pane() {
    if (++selected_pane_ix_ >= ui->chart_surface->axisRectCount()) {
        selected_pane_ix_ = 0;
    }
    select_pane(selected_pane_ix_);
}
void TG_Main_Window::cycle_pane_reverse() {
    if (--selected_pane_ix_ < 0) {
        selected_pane_ix_ = ui->chart_surface->axisRectCount() - 1;
    }
    select_pane(selected_pane_ix_);
}
void TG_Main_Window::zoom_in_time() {
    change_time_zooming(ui->chart_surface, ui->chart_surface->axisRect(0), 0.9, automatic_value_zoom);
}
void TG_Main_Window::zoom_out_time() {
    change_time_zooming(ui->chart_surface, ui->chart_surface->axisRect(0), 1 / 0.9, automatic_value_zoom);
}
void TG_Main_Window::auto_zoom_value() {
    automatic_value_zoom = true;
/*
    QCPDataMap::const_iterator lower;
    QCPDataMap::const_iterator upper;

    ui->chart_surface->graph(0)->getVisibleDataBounds(lower, upper);

    QCPRange range(lower->value, upper->value);
*/

    //.data()->range().lower

    //auto weighted_range = get_weighted_viewable_value_range(static_cast<QCPFinancialDataMap*>(ui->chart_surface->graph(0)->data()), ui->chart_surface->xAxis->range());

    do_auto_zoom(ui->chart_surface);

    ui->chart_surface->replot();
}
void TG_Main_Window::zoom_in_value() {
    automatic_value_zoom = false;
    change_value_zooming(ui->chart_surface, selected_pane_, 0.9);
}
void TG_Main_Window::zoom_out_value() {
    automatic_value_zoom = false;
    change_value_zooming(ui->chart_surface, selected_pane_, 1 / 0.9);
}


void TG_Main_Window::step_left() {
    auto ONE_MAIN_CANDLE_DURATION = G__navigation_ranges[2].time_navigation_step;
    change_time_position(ui->chart_surface, -ONE_MAIN_CANDLE_DURATION, automatic_value_zoom);
}
void TG_Main_Window::step_right() {
    auto ONE_MAIN_CANDLE_DURATION = G__navigation_ranges[2].time_navigation_step;
    change_time_position(ui->chart_surface, ONE_MAIN_CANDLE_DURATION, automatic_value_zoom);
}
void TG_Main_Window::jump_left() {
    auto VISIBLE_RANGE = ui->chart_surface->xAxis->range().size();
    change_time_position(ui->chart_surface, - VISIBLE_RANGE / 2, automatic_value_zoom);
}
void TG_Main_Window::jump_right() {
    auto VISIBLE_RANGE = ui->chart_surface->xAxis->range().size();
    change_time_position(ui->chart_surface, VISIBLE_RANGE / 2, automatic_value_zoom);
}

void TG_Main_Window::step_up() {
    auto range = yaxis(selected_pane_)->range();
    auto ONE_STEP_UNIT = (range.upper - range.lower) / 20;
    change_value_position(ui->chart_surface, selected_pane_, ONE_STEP_UNIT);
}
void TG_Main_Window::step_down() {
    auto range = yaxis(selected_pane_)->range();
    auto ONE_STEP_UNIT = (range.upper - range.lower) / 20;
    change_value_position(ui->chart_surface, selected_pane_, -ONE_STEP_UNIT);
}


// *TODO* left from copied demo code
void TG_Main_Window::screen_shot()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QPixmap pm = QPixmap::grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#else
    QPixmap pm = qApp->primaryScreen()->grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#endif
    QString screenshot_name = "screenshot"; // *TODO*
    QString fileName = screenshot_name.toLower()+".png";

    fileName.replace(" ", "");
    pm.save("./screenshots/"+fileName);
    qApp->quit();
}




void setupKeyboardShortcuts(TG_Main_Window * win) {
    //new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), win, SLOT(close()));
    //new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_R), win, SLOT(reload_data()));
    //new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_I), win, SLOT(toggle_legend()));
    new QShortcut(QKeySequence(Qt::Key_Q), win, SLOT(close()));
    new QShortcut(QKeySequence(Qt::Key_R), win, SLOT(reload_data()));
    new QShortcut(QKeySequence(Qt::Key_I), win, SLOT(toggle_legend()));

    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_P), win, SLOT(send_panic_signal()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_S), win, SLOT(send_save_command()));

    new QShortcut(QKeySequence(Qt::Key_Tab), win, SLOT(cycle_pane()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Tab), win, SLOT(cycle_pane_reverse()));

    new QShortcut(QKeySequence(Qt::Key_Z), win, SLOT(auto_zoom_value()));

    new QShortcut(QKeySequence(Qt::Key_Left), win, SLOT(step_left()));
    new QShortcut(QKeySequence(Qt::Key_Right), win, SLOT(step_right()));
    new QShortcut(QKeySequence(Qt::Key_Up), win, SLOT(step_up()));
    new QShortcut(QKeySequence(Qt::Key_Down), win, SLOT(step_down()));

    // *TODO* CTRL doesn't work!!
    new QShortcut(QKeySequence(Qt::Key_PageUp), win, SLOT(jump_left()));
    new QShortcut(QKeySequence(Qt::Key_PageDown), win, SLOT(jump_right()));

    // *TODO* CTRL doesn't work!!
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Plus), win, SLOT(zoom_in_time()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus), win, SLOT(zoom_out_time()));

    new QShortcut(QKeySequence(Qt::Key_G), win, SLOT(zoom_out_time()));
    new QShortcut(QKeySequence(Qt::Key_H), win, SLOT(zoom_in_time()));
    new QShortcut(QKeySequence(Qt::Key_Y), win, SLOT(zoom_in_value()));
    new QShortcut(QKeySequence(Qt::Key_B), win, SLOT(zoom_out_value()));

    // *TODO* key 0 = zoom to full dataset

}

void TG_Main_Window::setupTGQ7(QCustomPlot *chart) {
    chart->legend->setVisible(true);

    // *TODO* mouse-drag in ticks-labels => scroll X / Y
    // *TODO* mouse-drag in axis_rect => zoom in region

    chart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    setupKeyboardShortcuts(this);
}



