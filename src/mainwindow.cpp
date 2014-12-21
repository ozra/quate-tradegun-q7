#include "mainwindow.h"
#include "mainwindow_ui.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>
#include <QMetaEnum>
#include <QShortcut>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setGeometry(50, 50, 1142, 790);

    setupTGQ7(ui->chart_surface);
    setWindowTitle("TradeGunQ7");
    statusBar()->clearMessage();
    ui->chart_surface->replot();

}

MainWindow::~MainWindow()
{
    delete ui;
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

void dual_connect_panes(QWidget * view, QCPAxis* axis1, QCPAxis* axis2) {
    view->connect(axis1, SIGNAL(rangeChanged(QCPRange)), axis2, SLOT(setRange(QCPRange)));
    view->connect(axis2, SIGNAL(rangeChanged(QCPRange)), axis1, SLOT(setRange(QCPRange)));
}

QCPAxisRect* addChartPane(QWidget * view, QCustomPlot* cplt, int row_ix, QCPMarginGroup* margin_group) {
    /*
    demoName = "Line Style Demo";
    cplt->legend->setVisible(true);
    cplt->legend->setFont(QFont("Helvetica", 9));
    */

    auto pane_axis_rect = new QCPAxisRect(cplt);
    pane_axis_rect->setMarginGroup(QCP::msLeft|QCP::msRight, margin_group);
    //pane_axis_rect->setMargins();

    auto x_axis = pane_axis_rect->axis(QCPAxis::atBottom);
    x_axis->setTickLabelType(QCPAxis::ltDateTime);
    x_axis->setDateTimeSpec(Qt::UTC);
    x_axis->setDateTimeFormat("dd. MMM");
    x_axis->setVisible(false);

    cplt->plotLayout()->addElement(row_ix, 0, pane_axis_rect);

    // interconnect x axis ranges of main and bottom axis rects:
    //connect(cplt->xAxis, SIGNAL(rangeChanged(QCPRange)), x_axis, SLOT(setRange(QCPRange)));
    //connect(x_axis, SIGNAL(rangeChanged(QCPRange)), cplt->xAxis, SLOT(setRange(QCPRange)));
    dual_connect_panes(view, x_axis, cplt->xAxis);
    return pane_axis_rect;
}

QCPGraph* addPlot(QCustomPlot *chart_surface, QCPAxisRect* axisRect, QVector<double> times, QVector<double> values) {
    static int foooo_counter  = 0;
    ++foooo_counter;


    auto graph = new QCPGraph(axisRect->axis(QCPAxis::atBottom), axisRect->axis(QCPAxis::atLeft));
    chart_surface->addPlottable(graph);

    QPen pen;
    pen.setColor(QColor(qSin(foooo_counter*1+1.2)*80+80, qSin(foooo_counter*0.3+0)*80+80, qSin(foooo_counter*0.3+1.5)*80+80));
    graph->setPen(pen);
    graph->setName("Monkeeeey");

    graph->setLineStyle(QCPGraph::lsLine);
    auto marker =
        foooo_counter == 1 ? QCPScatterStyle::ssCircle :
        foooo_counter == 2 ? QCPScatterStyle::ssDiamond :
        foooo_counter == 3 ? QCPScatterStyle::ssCrossCircle :
        QCPScatterStyle::ssCross
    ;
    graph->setScatterStyle(QCPScatterStyle(marker, 5));

    graph->setData(times, values);
    graph->rescaleAxes(true);

    /*
    // zoom out a bit:
    chart_surface->yAxis->scaleRange(1.1, chart_surface->yAxis->range().center());
    chart_surface->xAxis->scaleRange(1.1, chart_surface->xAxis->range().center());
    // set blank axis lines:
    chart_surface->xAxis->setTicks(false);
    chart_surface->yAxis->setTicks(true);
    chart_surface->xAxis->setTickLabels(false);
    chart_surface->yAxis->setTickLabels(true);
    // make top right axes clones of bottom left axes:
    chart_surface->axisRect()->setupFullAxesBox();
    */

    return graph;
}

double rnd(double max) {
    return (qrand() / (double) RAND_MAX) * max;
}

inline void add_drawing_arrow(QCustomPlot *cplt, double sx, double sy, double ex, double ey, const QPen & pen) {
    auto *arrow = new QCPItemLine(cplt);
    cplt->addItem(arrow);
    auto lay = cplt->layer(0);
    arrow->setLayer(lay);
    arrow->setPen(pen);
    arrow->start->setCoords(sx, sy); // setParentAnchor(textLabel->bottom);
    arrow->end->setCoords(ex, ey);
    arrow->setHead(QCPLineEnding::esSpikeArrow);
}

void addDrawLayer(QCustomPlot *cplt, QVector<double> times, QVector<double> values) {
    auto i = rnd(10);
    auto len = times.size();

    QPen npen(QColor(50, 60, 85));
    QPen xpen(QColor(50, 90, 55));
    QPen rpen(QColor(90, 60, 55));

    while(i < len) {
        int end = i + rnd(1) + 30;
        if (end >= len)
            break;

        auto ratio = (values[end] - values[i]) /  (times[end] - times[i]);
        auto duration = (times[end] - times[end-1]);
        double extend = (rnd(150) + 50) * duration;

        // add the arrow:
        add_drawing_arrow(cplt, times[i], values[i], times[end] + extend, values[end] + (ratio * 2 * extend), xpen);
        add_drawing_arrow(cplt, times[i], values[i], times[end] + extend, values[end] + (ratio * extend), npen);
        add_drawing_arrow(cplt, times[i], values[i], times[end] + extend, values[end] + (-ratio * extend), rpen);

        i += rnd(50);
    }
}

QCPAxisRect * addVolumeBars(QCustomPlot* cplt, QCPAxisRect* vol_pane, QVector<double> times) {
    // create bottom axis rect for volume bar chart:
    //QCPAxisRect *vol_pane = new QCPAxisRect(cplt);
    vol_pane->setMaximumSize(QSize(QWIDGETSIZE_MAX, 100));
    vol_pane->axis(QCPAxis::atBottom)->setLayer("axes");
    vol_pane->axis(QCPAxis::atBottom)->grid()->setLayer("grid");

    // bring bottom and main axis rect closer together:
    cplt->plotLayout()->setRowSpacing(0);
    vol_pane->setAutoMargins(QCP::msLeft|QCP::msRight|QCP::msBottom);
    vol_pane->setMargins(QMargins(0, 0, 0, 0));


    // create two bar plottables, for positive (green) and negative (red) volume bars:
    QCPBars *volumeUp = new QCPBars(vol_pane->axis(QCPAxis::atBottom), vol_pane->axis(QCPAxis::atLeft));
    QCPBars *volumeDown = new QCPBars(vol_pane->axis(QCPAxis::atBottom), vol_pane->axis(QCPAxis::atLeft));
    auto n = times.size();
    for (int i = 0; i < n / 1; ++i) {
        int v = qrand() % 20000 + qrand() % 20000 + qrand() % 20000 - 10000 * 3;
        (v < 0 ? volumeDown : volumeUp)->addData(times[0] + 3600 * 1.0 * i, qAbs(v)); // add data to either volumeDown or volumeUp, depending on sign of v
    }
    cplt->setAutoAddPlottableToLegend(false);
    cplt->addPlottable(volumeUp);
    cplt->addPlottable(volumeDown);
    volumeUp->setWidth(3600 * 0.9);
    volumeUp->setPen(Qt::NoPen);
    volumeUp->setBrush(QColor(100, 180, 110));
    volumeDown->setWidth(3600 * 0.9);
    volumeDown->setPen(Qt::NoPen);
    volumeDown->setBrush(QColor(180, 90, 90));

    return vol_pane;
}

void addCandlePlot(QCustomPlot * cplt, QCPFinancialDataMap * candle_data, double candle_duration) {
    QCPFinancial *candlesticks = new QCPFinancial(cplt->xAxis, cplt->yAxis);
    cplt->addPlottable(candlesticks);
    candlesticks->setName("Candlestick");
    candlesticks->setChartStyle(QCPFinancial::csCandlestick);
    candlesticks->setData(candle_data, true);
    candlesticks->setWidth(candle_duration * 0.9);
    candlesticks->setTwoColored(true);
    candlesticks->setBrushPositive(QColor(245, 245, 245));
    candlesticks->setBrushNegative(QColor(0, 0, 0));
    candlesticks->setPenPositive(QPen(QColor(0, 0, 0)));
    candlesticks->setPenNegative(QPen(QColor(0, 0, 0)));
}

void addOhlcPlot(QCustomPlot * cplt, QCPFinancialDataMap * candle_data, double candle_duration) {
    QCPFinancial *ohlc = new QCPFinancial(cplt->xAxis, cplt->yAxis);
    cplt->addPlottable(ohlc);
    ohlc->setName("OHLC");
    ohlc->setChartStyle(QCPFinancial::csOhlc);
    ohlc->setData(candle_data, true);
    ohlc->setWidth(candle_duration*0.5);
    ohlc->setTwoColored(true);
}

void change_time_zooming(QCustomPlot *cplt, double zooming) {
    cplt->xAxis->scaleRange(zooming, cplt->xAxis->range().center());
    cplt->yAxis->rescale();
    cplt->yAxis->scaleRange(1, cplt->yAxis->range().center());
    cplt->replot();
}

void change_value_zooming(QCustomPlot *cplt, double zooming) {
    cplt->yAxis->scaleRange(zooming, cplt->yAxis->range().center());
    cplt->replot();
}

void change_position(QCustomPlot *cplt, double delta) {
    cplt->xAxis->setRange(cplt->xAxis->range().lower + delta, cplt->xAxis->range().size(), Qt::AlignLeft);
    cplt->yAxis->rescale();
    cplt->yAxis->scaleRange(1, cplt->yAxis->range().center());
    cplt->replot();
}

void MainWindow::zoom_in_time() {
    change_time_zooming(ui->chart_surface, 0.9);
}
void MainWindow::zoom_out_time() {
    change_time_zooming(ui->chart_surface, 1 / 0.9);
}
void MainWindow::zoom_in_value() {
    change_value_zooming(ui->chart_surface, 0.9);
}
void MainWindow::zoom_out_value() {
    change_value_zooming(ui->chart_surface, 1 / 0.9);
}
void MainWindow::step_left() {
    auto ONE_MAIN_CANDLE_DURATION = 3600;
    change_position(ui->chart_surface, -ONE_MAIN_CANDLE_DURATION);
}
void MainWindow::step_right() {
    auto ONE_MAIN_CANDLE_DURATION = 3600;
    change_position(ui->chart_surface, ONE_MAIN_CANDLE_DURATION);
}
void MainWindow::jump_left() {
    auto VISIBLE_RANGE = ui->chart_surface->xAxis->range().size();
    change_position(ui->chart_surface, - VISIBLE_RANGE / 2);
}
void MainWindow::jump_right() {
    auto VISIBLE_RANGE = ui->chart_surface->xAxis->range().size();
    change_position(ui->chart_surface, VISIBLE_RANGE / 2);
}

void MainWindow::setupTGQ7(QCustomPlot *cplt) {
    // // // // // // // // // // // // // // // // // // // // // // // //
    // CREATE FAKE DATA
    // CREATE FAKE DATA
    // generate two sets of random walk data (one for candlestick and one for ohlc chart):
    int n = 1000;
    QVector<double>
        time2(n*4),
        value1(n*4),
        value2(n*4),
        value3(n),
        value4(n),
        value5(n),
        time(n);

    QDateTime start = QDateTime(QDate(2014, 6, 11));
    start.setTimeSpec(Qt::UTC);
    double startTime = start.toTime_t();
    double binSize = 3600; // bin data in 1 day intervals
    qsrand(9);
    time[0] = startTime;
    value3[0] = 47;
    value4[0] = 0;
    value5[0] = 0;

    time2[0] = startTime;
    value1[0] = 60;
    value2[0] = 42;

    for (int i = 1; i < n * 4; ++i) {
        time2[i] = startTime + (3600/4)*i;
        value1[i] = value1[i-1] + (qrand()/(double)RAND_MAX-0.5)*10;
        value2[i] = value2[i-1] + (qrand()/(double)RAND_MAX-0.5)*10;
    }
    for (int i = 1; i < n; ++i) {
        time[i] = startTime + 3600*i;
        value3[i] = value3[i-1] + (qrand()/(double)RAND_MAX-0.5)*1;
        value4[i] = value4[i-1] + (qrand()/(double)RAND_MAX-0.5)*0.001;
        value5[i] = value5[i-1] + (qrand()/(double)RAND_MAX-0.5)*0.001;
    }
    QCPFinancialDataMap data1 = QCPFinancial::timeSeriesToOhlc(time2, value1, binSize, startTime);
    QCPFinancialDataMap data2 = QCPFinancial::timeSeriesToOhlc(time2, value2, binSize / 2, startTime); // divide binSize by 3 just to make the ohlc bars a bit denser
    // CREATE FAKE DATA
    // CREATE FAKE DATA
    // // // // // // // // // // // // // // // // // // // // // // // //



    //demoName = "TradeGunQ7";

    cplt->legend->setVisible(true);
    cplt->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    new QShortcut(QKeySequence(Qt::Key_Q), this, SLOT(close()));

    new QShortcut(QKeySequence(Qt::Key_Left), this, SLOT(step_left()));
    new QShortcut(QKeySequence(Qt::Key_Right), this, SLOT(step_right()));

    // *TODO* CTRL doesn't work!!
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Plus), this, SLOT(jump_left()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus), this, SLOT(jump_right()));

    new QShortcut(QKeySequence(Qt::Key_PageUp), this, SLOT(jump_left()));
    new QShortcut(QKeySequence(Qt::Key_PageDown), this, SLOT(jump_right()));

    // *TODO* CTRL doesn't work!!
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Plus), this, SLOT(zoom_in_time()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus), this, SLOT(zoom_out_time()));

    new QShortcut(QKeySequence(Qt::Key_Plus), this, SLOT(zoom_in_time()));
    new QShortcut(QKeySequence(Qt::Key_Minus), this, SLOT(zoom_out_time()));

    new QShortcut(QKeySequence(Qt::Key_Up), this, SLOT(zoom_in_value()));
    new QShortcut(QKeySequence(Qt::Key_Down), this, SLOT(zoom_out_value()));


    // make axis rects' left side line up:
    auto *margin_group = new QCPMarginGroup(cplt);

    auto pane1 = cplt->axisRect();

    pane1->setMarginGroup(QCP::msLeft|QCP::msRight, margin_group);
    pane1->setMinimumSize(50, 400);


    int row_ix = 0;

    // create candlestick chart, pane 1:1
    addCandlePlot(cplt, &data1, binSize);
    // create ohlc chart:, pane 1:2
    addOhlcPlot(cplt, &data2, binSize);
    // create lines / drawings layer - , pane 1:3
    addDrawLayer(cplt, time2, value1);

    // create pane 2
    auto pane2 = addChartPane(this, cplt, ++row_ix, margin_group);
    // create volume bars 2:1
    addVolumeBars(cplt, pane2, time);

    // create indicator pane 3
    auto pane3 = addChartPane(this, cplt, ++row_ix, margin_group);
    // create indicator plot 3:1
    addPlot(cplt, pane3, time, value3);

    // create indicator pane 4
    auto pane4 = addChartPane(this, cplt, ++row_ix, margin_group);
    // create indicator plot 4:1
    addPlot(cplt, pane4, time, value4);
    // create indicator plot 4:2
    addPlot(cplt, pane4, time, value5);


    // configure axes of both main and bottom axis rect:
    auto p2_x_axis = pane2->axis(QCPAxis::atBottom);
    p2_x_axis->setAutoTickStep(false);
    p2_x_axis->setTickStep(3600 * 24 * 1); // 5 day step (one session week)
    p2_x_axis->setTickLabelType(QCPAxis::ltDateTime);
    p2_x_axis->setDateTimeSpec(Qt::UTC);
    p2_x_axis->setDateTimeFormat("dd. MMM");
    p2_x_axis->setTickLabelRotation(15);
    pane2->axis(QCPAxis::atLeft)->setAutoTickCount(3);

    auto p1_x_axis = cplt->xAxis;
    p1_x_axis->setBasePen(Qt::NoPen);
    p1_x_axis->setTickLabels(false);
    p1_x_axis->setTicks(false); // only want vertical grid in main axis rect, so hide xAxis backbone, ticks, and labels
    p1_x_axis->setAutoTickStep(false);
    p1_x_axis->setTickStep(3600 * 24 * 5); // 4 day tickstep

    cplt->rescaleAxes();

    p1_x_axis->scaleRange(1.025, p1_x_axis->range().center());
    cplt->yAxis->scaleRange(1.1, cplt->yAxis->range().center());

}


















/*





void MainWindow::setupDemo(int demoIndex) {
    switch (demoIndex) {
    case 0:  setupQuadraticDemo(ui->chart_surface); break;
    case 1:  setupSimpleDemo(ui->chart_surface); break;
    case 2:  setupSincScatterDemo(ui->chart_surface); break;
    case 3:  setupScatterStyleDemo(ui->chart_surface); break;
    case 4:  setupScatterPixmapDemo(ui->chart_surface); break;
    case 5:  setupLineStyleDemo(ui->chart_surface); break;
    case 6:  setupDateDemo(ui->chart_surface); break;
    case 7:  setupTextureBrushDemo(ui->chart_surface); break;
    case 8:  setupMultiAxisDemo(ui->chart_surface); break;
    case 9:  setupLogarithmicDemo(ui->chart_surface); break;
    case 10: setupRealtimeDataDemo(ui->chart_surface); break;
    case 11: setupParametricCurveDemo(ui->chart_surface); break;
    case 12: setupBarChartDemo(ui->chart_surface); break;
    case 13: setupStatisticalDemo(ui->chart_surface); break;
    case 14: setupSimpleItemDemo(ui->chart_surface); break;
    case 15: setupItemDemo(ui->chart_surface); break;
    case 16: setupStyledDemo(ui->chart_surface); break;
    case 17: setupAdvancedAxesDemo(ui->chart_surface); break;
    case 18: setupColorMapDemo(ui->chart_surface); break;
    case 19: setupTGQ7(ui->chart_surface); break;
    }
    setWindowTitle("QCustomPlot: "+demoName);
    statusBar()->clearMessage();
    currentDemoIndex = demoIndex;
    ui->chart_surface->replot();
}

void MainWindow::setupQuadraticDemo(QCustomPlot *chart_surface) {
    demoName = "Quadratic Demo";
    // generate some data:
    QVector<double> x(101), y(101); // initialize with entries 0..100
    for (int i=0; i<101; ++i)
    {
        x[i] = i/50.0 - 1; // x goes from -1 to 1
        y[i] = x[i]*x[i];  // let's plot a quadratic function
    }
    // create graph and assign data to it:
    chart_surface->addGraph();
    chart_surface->graph(0)->setData(x, y);
    // give the axes some labels:
    chart_surface->xAxis->setLabel("x");
    chart_surface->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    chart_surface->xAxis->setRange(-1, 1);
    chart_surface->yAxis->setRange(0, 1);
}

void MainWindow::setupSimpleDemo(QCustomPlot *chart_surface)
{
    demoName = "Simple Demo";

    // add two new graphs and set their look:
    chart_surface->addGraph();
    chart_surface->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
    chart_surface->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue
    chart_surface->addGraph();
    chart_surface->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph
    // generate some points of data (y0 for first, y1 for second graph):
    QVector<double> x(250), y0(250), y1(250);
    for (int i=0; i<250; ++i)
    {
        x[i] = i;
        y0[i] = qExp(-i/150.0)*qCos(i/10.0); // exponentially decaying cosine
        y1[i] = qExp(-i/150.0);              // exponential envelope
    }
    // configure right and top axis to show ticks but no labels:
    // (see QCPAxisRect::setupFullAxesBox for a quicker method to do this)
    chart_surface->xAxis2->setVisible(true);
    chart_surface->xAxis2->setTickLabels(false);
    chart_surface->yAxis2->setVisible(true);
    chart_surface->yAxis2->setTickLabels(false);
    // make left and bottom axes always transfer their ranges to right and top axes:
    connect(chart_surface->xAxis, SIGNAL(rangeChanged(QCPRange)), chart_surface->xAxis2, SLOT(setRange(QCPRange)));
    connect(chart_surface->yAxis, SIGNAL(rangeChanged(QCPRange)), chart_surface->yAxis2, SLOT(setRange(QCPRange)));
    // pass data points to graphs:
    chart_surface->graph(0)->setData(x, y0);
    chart_surface->graph(1)->setData(x, y1);
    // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
    chart_surface->graph(0)->rescaleAxes();
    // same thing for graph 1, but only enlarge ranges (in case graph 1 is smaller than graph 0):
    chart_surface->graph(1)->rescaleAxes(true);
    // Note: we could have also just called chart_surface->rescaleAxes(); instead
    // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
    chart_surface->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}

void MainWindow::setupSincScatterDemo(QCustomPlot *chart_surface)
{
    demoName = "Sinc Scatter Demo";
    chart_surface->legend->setVisible(true);
    chart_surface->legend->setFont(QFont("Helvetica",9));
    // set locale to english, so we get english decimal separator:
    chart_surface->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));
    // add confidence band graphs:
    chart_surface->addGraph();
    QPen pen;
    pen.setStyle(Qt::DotLine);
    pen.setWidth(1);
    pen.setColor(QColor(180,180,180));
    chart_surface->graph(0)->setName("Confidence Band 68%");
    chart_surface->graph(0)->setPen(pen);
    chart_surface->graph(0)->setBrush(QBrush(QColor(255,50,30,20)));
    chart_surface->addGraph();
    chart_surface->legend->removeItem(chart_surface->legend->itemCount()-1); // don't show two confidence band graphs in legend
    chart_surface->graph(1)->setPen(pen);
    chart_surface->graph(0)->setChannelFillGraph(chart_surface->graph(1));
    // add theory curve graph:
    chart_surface->addGraph();
    pen.setStyle(Qt::DashLine);
    pen.setWidth(2);
    pen.setColor(Qt::red);
    chart_surface->graph(2)->setPen(pen);
    chart_surface->graph(2)->setName("Theory Curve");
    // add data point graph:
    chart_surface->addGraph();
    chart_surface->graph(3)->setPen(QPen(Qt::blue));
    chart_surface->graph(3)->setLineStyle(QCPGraph::lsNone);
    chart_surface->graph(3)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 4));
    chart_surface->graph(3)->setErrorType(QCPGraph::etValue);
    chart_surface->graph(3)->setErrorPen(QPen(QColor(180,180,180)));
    chart_surface->graph(3)->setName("Measurement");

    // generate ideal sinc curve data and some randomly perturbed data for scatter plot:
    QVector<double> x0(250), y0(250);
    QVector<double> yConfUpper(250), yConfLower(250);
    for (int i=0; i<250; ++i)
    {
        x0[i] = (i/249.0-0.5)*30+0.01; // by adding a small offset we make sure not do divide by zero in next code line
        y0[i] = qSin(x0[i])/x0[i]; // sinc function
        yConfUpper[i] = y0[i]+0.15;
        yConfLower[i] = y0[i]-0.15;
        x0[i] *= 1000;
    }
    QVector<double> x1(50), y1(50), y1err(50);
    for (int i=0; i<50; ++i)
    {
        // generate a gaussian distributed random number:
        double tmp1 = rand()/(double)RAND_MAX;
        double tmp2 = rand()/(double)RAND_MAX;
        double r = qSqrt(-2*qLn(tmp1))*qCos(2*M_PI*tmp2); // box-muller transform for gaussian distribution
        // set y1 to value of y0 plus a random gaussian pertubation:
        x1[i] = (i/50.0-0.5)*30+0.25;
        y1[i] = qSin(x1[i])/x1[i]+r*0.15;
        x1[i] *= 1000;
        y1err[i] = 0.15;
    }
    // pass data to graphs and let QCustomPlot determine the axes ranges so the whole thing is visible:
    chart_surface->graph(0)->setData(x0, yConfUpper);
    chart_surface->graph(1)->setData(x0, yConfLower);
    chart_surface->graph(2)->setData(x0, y0);
    chart_surface->graph(3)->setDataValueError(x1, y1, y1err);
    chart_surface->graph(2)->rescaleAxes();
    chart_surface->graph(3)->rescaleAxes(true);
    // setup look of bottom tick labels:
    chart_surface->xAxis->setTickLabelRotation(30);
    chart_surface->xAxis->setAutoTickCount(9);
    chart_surface->xAxis->setNumberFormat("ebc");
    chart_surface->xAxis->setNumberPrecision(1);
    chart_surface->xAxis->moveRange(-10);
    // make top right axes clones of bottom left axes. Looks prettier:
    chart_surface->axisRect()->setupFullAxesBox();
}

void MainWindow::setupScatterStyleDemo(QCustomPlot *chart_surface)
{
    demoName = "Scatter Style Demo";
    chart_surface->legend->setVisible(true);
    chart_surface->legend->setFont(QFont("Helvetica", 9));
    chart_surface->legend->setRowSpacing(-3);
    QVector<QCPScatterStyle::ScatterShape> shapes;
    shapes << QCPScatterStyle::ssCross;
    shapes << QCPScatterStyle::ssPlus;
    shapes << QCPScatterStyle::ssCircle;
    shapes << QCPScatterStyle::ssDisc;
    shapes << QCPScatterStyle::ssSquare;
    shapes << QCPScatterStyle::ssDiamond;
    shapes << QCPScatterStyle::ssStar;
    shapes << QCPScatterStyle::ssTriangle;
    shapes << QCPScatterStyle::ssTriangleInverted;
    shapes << QCPScatterStyle::ssCrossSquare;
    shapes << QCPScatterStyle::ssPlusSquare;
    shapes << QCPScatterStyle::ssCrossCircle;
    shapes << QCPScatterStyle::ssPlusCircle;
    shapes << QCPScatterStyle::ssPeace;
    shapes << QCPScatterStyle::ssCustom;

    QPen pen;
    // add graphs with different scatter styles:
    for (int i=0; i<shapes.size(); ++i)
    {
        chart_surface->addGraph();
        pen.setColor(QColor(qSin(i*0.3)*100+100, qSin(i*0.6+0.7)*100+100, qSin(i*0.4+0.6)*100+100));
        // generate data:
        QVector<double> x(10), y(10);
        for (int k=0; k<10; ++k)
        {
            x[k] = k/10.0 * 4*3.14 + 0.01;
            y[k] = 7*qSin(x[k])/x[k] + (shapes.size()-i)*5;
        }
        chart_surface->graph()->setData(x, y);
        chart_surface->graph()->rescaleAxes(true);
        chart_surface->graph()->setPen(pen);
        chart_surface->graph()->setName(QCPScatterStyle::staticMetaObject.enumerator(QCPScatterStyle::staticMetaObject.indexOfEnumerator("ScatterShape")).valueToKey(shapes.at(i)));
        chart_surface->graph()->setLineStyle(QCPGraph::lsLine);
        // set scatter style:
        if (shapes.at(i) != QCPScatterStyle::ssCustom)
        {
            chart_surface->graph()->setScatterStyle(QCPScatterStyle(shapes.at(i), 10));
        }
        else
        {
            QPainterPath customScatterPath;
            for (int i=0; i<3; ++i)
                customScatterPath.cubicTo(qCos(2*M_PI*i/3.0)*9, qSin(2*M_PI*i/3.0)*9, qCos(2*M_PI*(i+0.9)/3.0)*9, qSin(2*M_PI*(i+0.9)/3.0)*9, 0, 0);
            chart_surface->graph()->setScatterStyle(QCPScatterStyle(customScatterPath, QPen(Qt::black, 0), QColor(40, 70, 255, 50), 10));
        }
    }
    // set blank axis lines:
    chart_surface->rescaleAxes();
    chart_surface->xAxis->setTicks(false);
    chart_surface->yAxis->setTicks(false);
    chart_surface->xAxis->setTickLabels(false);
    chart_surface->yAxis->setTickLabels(false);
    // make top right axes clones of bottom left axes:
    chart_surface->axisRect()->setupFullAxesBox();
}

void MainWindow::setupLineStyleDemo(QCustomPlot *chart_surface)
{
    demoName = "Line Style Demo";
    chart_surface->legend->setVisible(true);
    chart_surface->legend->setFont(QFont("Helvetica", 9));
    QPen pen;
    QStringList lineNames;
    lineNames << "lsNone" << "lsLine" << "lsStepLeft" << "lsStepRight"
              << "lsStepCenter" << "lsImpulse";
    // add graphs with different line styles:
    for (int i=QCPGraph::lsNone; i<=QCPGraph::lsImpulse; ++i)
    {
        chart_surface->addGraph();
        pen.setColor(QColor(qSin(i*1+1.2)*80+80, qSin(i*0.3+0)*80+80, qSin(i*0.3+1.5)*80+80));
        chart_surface->graph()->setPen(pen);
        chart_surface->graph()->setName(lineNames.at(i-QCPGraph::lsNone));
        chart_surface->graph()->setLineStyle((QCPGraph::LineStyle)i);
        chart_surface->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
        // generate data:
        QVector<double> x(15), y(15);
        for (int j=0; j<15; ++j)
        {
            x[j] = j/15.0 * 5*3.14 + 0.01;
            y[j] = 7*qSin(x[j])/x[j] - (i-QCPGraph::lsNone)*5 + (QCPGraph::lsImpulse)*5 + 2;
        }
        chart_surface->graph()->setData(x, y);
        chart_surface->graph()->rescaleAxes(true);
    }
    // zoom out a bit:
    chart_surface->yAxis->scaleRange(1.1, chart_surface->yAxis->range().center());
    chart_surface->xAxis->scaleRange(1.1, chart_surface->xAxis->range().center());
    // set blank axis lines:
    chart_surface->xAxis->setTicks(false);
    chart_surface->yAxis->setTicks(true);
    chart_surface->xAxis->setTickLabels(false);
    chart_surface->yAxis->setTickLabels(true);
    // make top right axes clones of bottom left axes:
    chart_surface->axisRect()->setupFullAxesBox();
}

void MainWindow::setupScatterPixmapDemo(QCustomPlot *chart_surface)
{
    demoName = "Scatter Pixmap Demo";
    chart_surface->axisRect()->setBackground(QPixmap("./solarpanels.jpg"));
    chart_surface->addGraph();
    chart_surface->graph()->setLineStyle(QCPGraph::lsLine);
    QPen pen;
    pen.setColor(QColor(255, 200, 20, 200));
    pen.setStyle(Qt::DashLine);
    pen.setWidthF(2.5);
    chart_surface->graph()->setPen(pen);
    chart_surface->graph()->setBrush(QBrush(QColor(255,200,20,70)));
    chart_surface->graph()->setScatterStyle(QCPScatterStyle(QPixmap("./sun.png")));
    // set graph name, will show up in legend next to icon:
    chart_surface->graph()->setName("Data from Photovoltaic\nenergy barometer 2011");
    // set data:
    QVector<double> year, value;
    year  << 2005 << 2006 << 2007 << 2008  << 2009  << 2010;
    value << 2.17 << 3.42 << 4.94 << 10.38 << 15.86 << 29.33;
    chart_surface->graph()->setData(year, value);

    // set title of plot:
    chart_surface->plotLayout()->insertRow(0);
    chart_surface->plotLayout()->addElement(0, 0, new QCPPlotTitle(chart_surface, "Regenerative Energies"));
    // set a fixed tick-step to one tick per year value:
    chart_surface->xAxis->setAutoTickStep(false);
    chart_surface->xAxis->setTickStep(1);
    chart_surface->xAxis->setSubTickCount(3);
    // other axis configurations:
    chart_surface->xAxis->setLabel("Year");
    chart_surface->yAxis->setLabel("Installed Gigawatts of\nphotovoltaic in the European Union");
    chart_surface->xAxis2->setVisible(true);
    chart_surface->yAxis2->setVisible(true);
    chart_surface->xAxis2->setTickLabels(false);
    chart_surface->yAxis2->setTickLabels(false);
    chart_surface->xAxis2->setTicks(false);
    chart_surface->yAxis2->setTicks(false);
    chart_surface->xAxis2->setSubTickCount(0);
    chart_surface->yAxis2->setSubTickCount(0);
    chart_surface->xAxis->setRange(2004.5, 2010.5);
    chart_surface->yAxis->setRange(0, 30);
    // setup legend:
    chart_surface->legend->setFont(QFont(font().family(), 7));
    chart_surface->legend->setIconSize(50, 20);
    chart_surface->legend->setVisible(true);
}

void MainWindow::setupDateDemo(QCustomPlot *chart_surface)
{
    demoName = "Date Demo";
    // set locale to english, so we get english month names:
    chart_surface->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));
    // seconds of current time, we'll use it as starting point in time for data:
    double now = QDateTime::currentDateTime().toTime_t();
    srand(8); // set the random seed, so we always get the same random data
    // create multiple graphs:
    for (int gi=0; gi<5; ++gi)
    {
        chart_surface->addGraph();
        QPen pen;
        pen.setColor(QColor(0, 0, 255, 200));
        chart_surface->graph()->setLineStyle(QCPGraph::lsLine);
        chart_surface->graph()->setPen(pen);
        chart_surface->graph()->setBrush(QBrush(QColor(255/4.0*gi,160,50,150)));
        // generate random walk data:
        QVector<double> time(250), value(250);
        for (int i=0; i<250; ++i)
        {
            time[i] = now + 24*3600*i;
            if (i == 0)
                value[i] = (i/50.0+1)*(rand()/(double)RAND_MAX-0.5);
            else
                value[i] = qFabs(value[i-1])*(1+0.02/4.0*(4-gi)) + (i/50.0+1)*(rand()/(double)RAND_MAX-0.5);
        }
        chart_surface->graph()->setData(time, value);
    }
    // configure bottom axis to show date and time instead of number:
    chart_surface->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    chart_surface->xAxis->setDateTimeFormat("MMMM\nyyyy");
    // set a more compact font size for bottom and left axis tick labels:
    chart_surface->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
    chart_surface->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
    // set a fixed tick-step to one tick per month:
    chart_surface->xAxis->setAutoTickStep(false);
    chart_surface->xAxis->setTickStep(2628000); // one month in seconds
    chart_surface->xAxis->setSubTickCount(3);
    // apply manual tick and tick label for left axis:
    chart_surface->yAxis->setAutoTicks(false);
    chart_surface->yAxis->setAutoTickLabels(false);
    chart_surface->yAxis->setTickVector(QVector<double>() << 5 << 55);
    chart_surface->yAxis->setTickVectorLabels(QVector<QString>() << "Not so\nhigh" << "Very\nhigh");
    // set axis labels:
    chart_surface->xAxis->setLabel("Date");
    chart_surface->yAxis->setLabel("Random wobbly lines value");
    // make top and right axes visible but without ticks and labels:
    chart_surface->xAxis2->setVisible(true);
    chart_surface->yAxis2->setVisible(true);
    chart_surface->xAxis2->setTicks(false);
    chart_surface->yAxis2->setTicks(false);
    chart_surface->xAxis2->setTickLabels(false);
    chart_surface->yAxis2->setTickLabels(false);
    // set axis ranges to show all data:
    chart_surface->xAxis->setRange(now, now+24*3600*249);
    chart_surface->yAxis->setRange(0, 60);
    // show legend:
    chart_surface->legend->setVisible(true);
}

void MainWindow::setupTextureBrushDemo(QCustomPlot *chart_surface)
{
    demoName = "Texture Brush Demo";
    // add two graphs with a textured fill:
    chart_surface->addGraph();
    QPen redDotPen;
    redDotPen.setStyle(Qt::DotLine);
    redDotPen.setColor(QColor(170, 100, 100, 180));
    redDotPen.setWidthF(2);
    chart_surface->graph(0)->setPen(redDotPen);
    chart_surface->graph(0)->setBrush(QBrush(QPixmap("./balboa.jpg"))); // fill with texture of specified image

    chart_surface->addGraph();
    chart_surface->graph(1)->setPen(QPen(Qt::red));

    // activate channel fill for graph 0 towards graph 1:
    chart_surface->graph(0)->setChannelFillGraph(chart_surface->graph(1));

    // generate data:
    QVector<double> x(250);
    QVector<double> y0(250), y1(250);
    for (int i=0; i<250; ++i)
    {
        // just playing with numbers, not much to learn here
        x[i] = 3*i/250.0;
        y0[i] = 1+qExp(-x[i]*x[i]*0.8)*(x[i]*x[i]+x[i]);
        y1[i] = 1-qExp(-x[i]*x[i]*0.4)*(x[i]*x[i])*0.1;
    }

    // pass data points to graphs:
    chart_surface->graph(0)->setData(x, y0);
    chart_surface->graph(1)->setData(x, y1);
    // activate top and right axes, which are invisible by default:
    chart_surface->xAxis2->setVisible(true);
    chart_surface->yAxis2->setVisible(true);
    // make tick labels invisible on top and right axis:
    chart_surface->xAxis2->setTickLabels(false);
    chart_surface->yAxis2->setTickLabels(false);
    // set ranges:
    chart_surface->xAxis->setRange(0, 2.5);
    chart_surface->yAxis->setRange(0.9, 1.6);
    // assign top/right axes same properties as bottom/left:
    chart_surface->axisRect()->setupFullAxesBox();
}

void MainWindow::setupMultiAxisDemo(QCustomPlot *chart_surface)
{
    chart_surface->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    demoName = "Multi Axis Demo";

    chart_surface->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom)); // period as decimal separator and comma as thousand separator
    chart_surface->legend->setVisible(true);
    QFont legendFont = font();  // start out with MainWindow's font..
    legendFont.setPointSize(9); // and make a bit smaller for legend
    chart_surface->legend->setFont(legendFont);
    chart_surface->legend->setBrush(QBrush(QColor(255,255,255,230)));
    // by default, the legend is in the inset layout of the main axis rect. So this is how we access it to change legend placement:
    chart_surface->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignRight);

    // setup for graph 0: key axis left, value axis bottom
    // will contain left maxwell-like function
    chart_surface->addGraph(chart_surface->yAxis, chart_surface->xAxis);
    chart_surface->graph(0)->setPen(QPen(QColor(255, 100, 0)));
    chart_surface->graph(0)->setBrush(QBrush(QPixmap("./balboa.jpg"))); // fill with texture of specified image
    chart_surface->graph(0)->setLineStyle(QCPGraph::lsLine);
    chart_surface->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
    chart_surface->graph(0)->setName("Left maxwell function");

    // setup for graph 1: key axis bottom, value axis left (those are the default axes)
    // will contain bottom maxwell-like function
    chart_surface->addGraph();
    chart_surface->graph(1)->setPen(QPen(Qt::red));
    chart_surface->graph(1)->setBrush(QBrush(QPixmap("./balboa.jpg"))); // same fill as we used for graph 0
    chart_surface->graph(1)->setLineStyle(QCPGraph::lsStepCenter);
    chart_surface->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::red, Qt::white, 7));
    chart_surface->graph(1)->setErrorType(QCPGraph::etValue);
    chart_surface->graph(1)->setName("Bottom maxwell function");

    // setup for graph 2: key axis top, value axis right
    // will contain high frequency sine with low frequency beating:
    chart_surface->addGraph(chart_surface->xAxis2, chart_surface->yAxis2);
    chart_surface->graph(2)->setPen(QPen(Qt::blue));
    chart_surface->graph(2)->setName("High frequency sine");

    // setup for graph 3: same axes as graph 2
    // will contain low frequency beating envelope of graph 2
    chart_surface->addGraph(chart_surface->xAxis2, chart_surface->yAxis2);
    QPen blueDotPen;
    blueDotPen.setColor(QColor(30, 40, 255, 150));
    blueDotPen.setStyle(Qt::DotLine);
    blueDotPen.setWidthF(4);
    chart_surface->graph(3)->setPen(blueDotPen);
    chart_surface->graph(3)->setName("Sine envelope");

    // setup for graph 4: key axis right, value axis top
    // will contain parabolically distributed data points with some random perturbance
    chart_surface->addGraph(chart_surface->yAxis2, chart_surface->xAxis2);
    chart_surface->graph(4)->setPen(QColor(50, 50, 50, 255));
    chart_surface->graph(4)->setLineStyle(QCPGraph::lsNone);
    chart_surface->graph(4)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));
    chart_surface->graph(4)->setName("Some random data around\na quadratic function");

    // generate data, just playing with numbers, not much to learn here:
    QVector<double> x0(25), y0(25);
    QVector<double> x1(15), y1(15), y1err(15);
    QVector<double> x2(250), y2(250);
    QVector<double> x3(250), y3(250);
    QVector<double> x4(250), y4(250);
    for (int i=0; i<25; ++i) // data for graph 0
    {
        x0[i] = 3*i/25.0;
        y0[i] = qExp(-x0[i]*x0[i]*0.8)*(x0[i]*x0[i]+x0[i]);
    }
    for (int i=0; i<15; ++i) // data for graph 1
    {
        x1[i] = 3*i/15.0;;
        y1[i] = qExp(-x1[i]*x1[i])*(x1[i]*x1[i])*2.6;
        y1err[i] = y1[i]*0.25;
    }
    for (int i=0; i<250; ++i) // data for graphs 2, 3 and 4
    {
        x2[i] = i/250.0*3*M_PI;
        x3[i] = x2[i];
        x4[i] = i/250.0*100-50;
        y2[i] = qSin(x2[i]*12)*qCos(x2[i])*10;
        y3[i] = qCos(x3[i])*10;
        y4[i] = 0.01*x4[i]*x4[i] + 1.5*(rand()/(double)RAND_MAX-0.5) + 1.5*M_PI;
    }

    // pass data points to graphs:
    chart_surface->graph(0)->setData(x0, y0);
    chart_surface->graph(1)->setDataValueError(x1, y1, y1err);
    chart_surface->graph(2)->setData(x2, y2);
    chart_surface->graph(3)->setData(x3, y3);
    chart_surface->graph(4)->setData(x4, y4);
    // activate top and right axes, which are invisible by default:
    chart_surface->xAxis2->setVisible(true);
    chart_surface->yAxis2->setVisible(true);
    // set ranges appropriate to show data:
    chart_surface->xAxis->setRange(0, 2.7);
    chart_surface->yAxis->setRange(0, 2.6);
    chart_surface->xAxis2->setRange(0, 3.0*M_PI);
    chart_surface->yAxis2->setRange(-70, 35);
    // set pi ticks on top axis:
    QVector<double> piTicks;
    QVector<QString> piLabels;
    piTicks << 0  << 0.5*M_PI << M_PI << 1.5*M_PI << 2*M_PI << 2.5*M_PI << 3*M_PI;
    piLabels << "0" << QString::fromUtf8("½π") << QString::fromUtf8("π") << QString::fromUtf8("1½π") << QString::fromUtf8("2π") << QString::fromUtf8("2½π") << QString::fromUtf8("3π");
    chart_surface->xAxis2->setAutoTicks(false);
    chart_surface->xAxis2->setAutoTickLabels(false);
    chart_surface->xAxis2->setTickVector(piTicks);
    chart_surface->xAxis2->setTickVectorLabels(piLabels);
    // add title layout element:
    chart_surface->plotLayout()->insertRow(0);
    chart_surface->plotLayout()->addElement(0, 0, new QCPPlotTitle(chart_surface, "Way too many graphs in one plot"));
    // set labels:
    chart_surface->xAxis->setLabel("Bottom axis with outward ticks");
    chart_surface->yAxis->setLabel("Left axis label");
    chart_surface->xAxis2->setLabel("Top axis label");
    chart_surface->yAxis2->setLabel("Right axis label");
    // make ticks on bottom axis go outward:
    chart_surface->xAxis->setTickLength(0, 5);
    chart_surface->xAxis->setSubTickLength(0, 3);
    // make ticks on right axis go inward and outward:
    chart_surface->yAxis2->setTickLength(3, 3);
    chart_surface->yAxis2->setSubTickLength(1, 1);
}

void MainWindow::setupLogarithmicDemo(QCustomPlot *chart_surface)
{
    demoName = "Logarithmic Demo";
    chart_surface->setNoAntialiasingOnDrag(true); // more performance/responsiveness during dragging
    chart_surface->addGraph();
    QPen pen;
    pen.setColor(QColor(255,170,100));
    pen.setWidth(2);
    pen.setStyle(Qt::DotLine);
    chart_surface->graph(0)->setPen(pen);
    chart_surface->graph(0)->setName("x");

    chart_surface->addGraph();
    chart_surface->graph(1)->setPen(QPen(Qt::red));
    chart_surface->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));
    chart_surface->graph(1)->setErrorType(QCPGraph::etBoth);
    chart_surface->graph(1)->setName("-sin(x)exp(x)");

    chart_surface->addGraph();
    chart_surface->graph(2)->setPen(QPen(Qt::blue));
    chart_surface->graph(2)->setBrush(QBrush(QColor(0, 0, 255, 20)));
    chart_surface->graph(2)->setName(" sin(x)exp(x)");

    chart_surface->addGraph();
    pen.setColor(QColor(0,0,0));
    pen.setWidth(1);
    pen.setStyle(Qt::DashLine);
    chart_surface->graph(3)->setPen(pen);
    chart_surface->graph(3)->setBrush(QBrush(QColor(0,0,0,15)));
    chart_surface->graph(3)->setLineStyle(QCPGraph::lsStepCenter);
    chart_surface->graph(3)->setName("x!");

    QVector<double> x0(200), y0(200);
    QVector<double> x1(200), y1(200);
    QVector<double> x2(200), y2(200);
    QVector<double> x3(21), y3(21);
    for (int i=0; i<200; ++i)
    {
        x0[i] = i/10.0;
        y0[i] = x0[i];
        x1[i] = i/10.0;
        y1[i] = -qSin(x1[i])*qExp(x1[i]);
        x2[i] = i/10.0;
        y2[i] = qSin(x2[i])*qExp(x2[i]);
    }
    for (int i=0; i<21; ++i)
    {
        x3[i] = i;
        y3[i] = 1;
        for (int k=1; k<=i; ++k) y3[i] *= k; // factorial
    }
    chart_surface->graph(0)->setData(x0, y0);
    chart_surface->graph(1)->setData(x1, y1);
    chart_surface->graph(2)->setData(x2, y2);
    chart_surface->graph(3)->setData(x3, y3);

    chart_surface->yAxis->grid()->setSubGridVisible(true);
    chart_surface->xAxis->grid()->setSubGridVisible(true);
    chart_surface->yAxis->setScaleType(QCPAxis::stLogarithmic);
    chart_surface->yAxis->setScaleLogBase(100);
    chart_surface->yAxis->setNumberFormat("eb"); // e = exponential, b = beautiful decimal powers
    chart_surface->yAxis->setNumberPrecision(0); // makes sure "1*10^4" is displayed only as "10^4"
    chart_surface->yAxis->setSubTickCount(10);
    chart_surface->xAxis->setRange(0, 19.9);
    chart_surface->yAxis->setRange(1e-2, 1e10);
    // make range draggable and zoomable:
    chart_surface->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    // make top right axes clones of bottom left axes:
    chart_surface->axisRect()->setupFullAxesBox();
    // connect signals so top and right axes move in sync with bottom and left axes:
    connect(chart_surface->xAxis, SIGNAL(rangeChanged(QCPRange)), chart_surface->xAxis2, SLOT(setRange(QCPRange)));
    connect(chart_surface->yAxis, SIGNAL(rangeChanged(QCPRange)), chart_surface->yAxis2, SLOT(setRange(QCPRange)));

    chart_surface->legend->setVisible(true);
    chart_surface->legend->setBrush(QBrush(QColor(255,255,255,150)));
    chart_surface->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft|Qt::AlignTop); // make legend align in top left corner or axis rect
}

void MainWindow::setupRealtimeDataDemo(QCustomPlot *chart_surface)
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
    QMessageBox::critical(this, "", "You're using Qt < 4.7, the realtime data demo needs functions that are available with Qt 4.7 to work properly");
#endif
    demoName = "Real Time Data Demo";

    // include this section to fully disable antialiasing for higher performance:
    / *
  chart_surface->setNotAntialiasedElements(QCP::aeAll);
  QFont font;
  font.setStyleStrategy(QFont::NoAntialias);
  chart_surface->xAxis->setTickLabelFont(font);
  chart_surface->yAxis->setTickLabelFont(font);
  chart_surface->legend->setFont(font);
  * /
    chart_surface->addGraph(); // blue line
    chart_surface->graph(0)->setPen(QPen(Qt::blue));
    chart_surface->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
    chart_surface->graph(0)->setAntialiasedFill(false);
    chart_surface->addGraph(); // red line
    chart_surface->graph(1)->setPen(QPen(Qt::red));
    chart_surface->graph(0)->setChannelFillGraph(chart_surface->graph(1));

    chart_surface->addGraph(); // blue dot
    chart_surface->graph(2)->setPen(QPen(Qt::blue));
    chart_surface->graph(2)->setLineStyle(QCPGraph::lsNone);
    chart_surface->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
    chart_surface->addGraph(); // red dot
    chart_surface->graph(3)->setPen(QPen(Qt::red));
    chart_surface->graph(3)->setLineStyle(QCPGraph::lsNone);
    chart_surface->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);

    chart_surface->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    chart_surface->xAxis->setDateTimeFormat("hh:mm:ss");
    chart_surface->xAxis->setAutoTickStep(false);
    chart_surface->xAxis->setTickStep(2);
    chart_surface->axisRect()->setupFullAxesBox();

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(chart_surface->xAxis, SIGNAL(rangeChanged(QCPRange)), chart_surface->xAxis2, SLOT(setRange(QCPRange)));
    connect(chart_surface->yAxis, SIGNAL(rangeChanged(QCPRange)), chart_surface->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(0); // Interval 0 means to refresh as fast as possible
}

void MainWindow::setupParametricCurveDemo(QCustomPlot *chart_surface)
{
    demoName = "Parametric Curves Demo";

    // create empty curve objects and add them to chart_surface:
    QCPCurve *fermatSpiral1 = new QCPCurve(chart_surface->xAxis, chart_surface->yAxis);
    QCPCurve *fermatSpiral2 = new QCPCurve(chart_surface->xAxis, chart_surface->yAxis);
    QCPCurve *deltoidRadial = new QCPCurve(chart_surface->xAxis, chart_surface->yAxis);
    chart_surface->addPlottable(fermatSpiral1);
    chart_surface->addPlottable(fermatSpiral2);
    chart_surface->addPlottable(deltoidRadial);
    // generate the curve data points:
    int pointCount = 500;
    QVector<double> x1(pointCount), y1(pointCount);
    QVector<double> x2(pointCount), y2(pointCount);
    QVector<double> x3(pointCount), y3(pointCount);
    for (int i=0; i<pointCount; ++i)
    {
        double phi = (i/(double)(pointCount-1))*8*M_PI;
        x1[i] = qSqrt(phi)*qCos(phi);
        y1[i] = qSqrt(phi)*qSin(phi);
        x2[i] = -x1[i];
        y2[i] = -y1[i];
        double t = i/(double)(pointCount-1)*2*M_PI;
        x3[i] = 2*qCos(2*t)+qCos(1*t)+2*qSin(t);
        y3[i] = 2*qSin(2*t)-qSin(1*t);
    }
    // pass the data to the curves:
    fermatSpiral1->setData(x1, y1);
    fermatSpiral2->setData(x2, y2);
    deltoidRadial->setData(x3, y3);
    // color the curves:
    fermatSpiral1->setPen(QPen(Qt::blue));
    fermatSpiral1->setBrush(QBrush(QColor(0, 0, 255, 20)));
    fermatSpiral2->setPen(QPen(QColor(255, 120, 0)));
    fermatSpiral2->setBrush(QBrush(QColor(255, 120, 0, 30)));
    QRadialGradient radialGrad(QPointF(310, 180), 200);
    radialGrad.setColorAt(0, QColor(170, 20, 240, 100));
    radialGrad.setColorAt(0.5, QColor(20, 10, 255, 40));
    radialGrad.setColorAt(1,QColor(120, 20, 240, 10));
    deltoidRadial->setPen(QPen(QColor(170, 20, 240)));
    deltoidRadial->setBrush(QBrush(radialGrad));
    // set some basic chart_surface config:
    chart_surface->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    chart_surface->axisRect()->setupFullAxesBox();
    chart_surface->rescaleAxes();
}

void MainWindow::setupBarChartDemo(QCustomPlot *chart_surface)
{
    demoName = "Bar Chart Demo";
    // create empty bar chart objects:
    QCPBars *regen = new QCPBars(chart_surface->xAxis, chart_surface->yAxis);
    QCPBars *nuclear = new QCPBars(chart_surface->xAxis, chart_surface->yAxis);
    QCPBars *fossil = new QCPBars(chart_surface->xAxis, chart_surface->yAxis);
    chart_surface->addPlottable(regen);
    chart_surface->addPlottable(nuclear);
    chart_surface->addPlottable(fossil);
    // set names and colors:
    QPen pen;
    pen.setWidthF(1.2);
    fossil->setName("Fossil fuels");
    pen.setColor(QColor(255, 131, 0));
    fossil->setPen(pen);
    fossil->setBrush(QColor(255, 131, 0, 50));
    nuclear->setName("Nuclear");
    pen.setColor(QColor(1, 92, 191));
    nuclear->setPen(pen);
    nuclear->setBrush(QColor(1, 92, 191, 50));
    regen->setName("Regenerative");
    pen.setColor(QColor(150, 222, 0));
    regen->setPen(pen);
    regen->setBrush(QColor(150, 222, 0, 70));
    // stack bars ontop of each other:
    nuclear->moveAbove(fossil);
    regen->moveAbove(nuclear);

    // prepare x axis with country labels:
    QVector<double> ticks;
    QVector<QString> labels;
    ticks << 1 << 2 << 3 << 4 << 5 << 6 << 7;
    labels << "USA" << "Japan" << "Germany" << "France" << "UK" << "Italy" << "Canada";
    chart_surface->xAxis->setAutoTicks(false);
    chart_surface->xAxis->setAutoTickLabels(false);
    chart_surface->xAxis->setTickVector(ticks);
    chart_surface->xAxis->setTickVectorLabels(labels);
    chart_surface->xAxis->setTickLabelRotation(60);
    chart_surface->xAxis->setSubTickCount(0);
    chart_surface->xAxis->setTickLength(0, 4);
    chart_surface->xAxis->grid()->setVisible(true);
    chart_surface->xAxis->setRange(0, 8);

    // prepare y axis:
    chart_surface->yAxis->setRange(0, 12.1);
    chart_surface->yAxis->setPadding(5); // a bit more space to the left border
    chart_surface->yAxis->setLabel("Power Consumption in\nKilowatts per Capita (2007)");
    chart_surface->yAxis->grid()->setSubGridVisible(true);
    QPen gridPen;
    gridPen.setStyle(Qt::SolidLine);
    gridPen.setColor(QColor(0, 0, 0, 25));
    chart_surface->yAxis->grid()->setPen(gridPen);
    gridPen.setStyle(Qt::DotLine);
    chart_surface->yAxis->grid()->setSubGridPen(gridPen);

    // Add data:
    QVector<double> fossilData, nuclearData, regenData;
    fossilData  << 0.86*10.5 << 0.83*5.5 << 0.84*5.5 << 0.52*5.8 << 0.89*5.2 << 0.90*4.2 << 0.67*11.2;
    nuclearData << 0.08*10.5 << 0.12*5.5 << 0.12*5.5 << 0.40*5.8 << 0.09*5.2 << 0.00*4.2 << 0.07*11.2;
    regenData   << 0.06*10.5 << 0.05*5.5 << 0.04*5.5 << 0.06*5.8 << 0.02*5.2 << 0.07*4.2 << 0.25*11.2;
    fossil->setData(ticks, fossilData);
    nuclear->setData(ticks, nuclearData);
    regen->setData(ticks, regenData);

    // setup legend:
    chart_surface->legend->setVisible(true);
    chart_surface->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignHCenter);
    chart_surface->legend->setBrush(QColor(255, 255, 255, 200));
    QPen legendPen;
    legendPen.setColor(QColor(130, 130, 130, 200));
    chart_surface->legend->setBorderPen(legendPen);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    chart_surface->legend->setFont(legendFont);
    chart_surface->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void MainWindow::setupStatisticalDemo(QCustomPlot *chart_surface)
{
    demoName = "Statistical Demo";
    // create empty statistical box plottables:
    QCPStatisticalBox *sample1 = new QCPStatisticalBox(chart_surface->xAxis, chart_surface->yAxis);
    QCPStatisticalBox *sample2 = new QCPStatisticalBox(chart_surface->xAxis, chart_surface->yAxis);
    QCPStatisticalBox *sample3 = new QCPStatisticalBox(chart_surface->xAxis, chart_surface->yAxis);
    chart_surface->addPlottable(sample1);
    chart_surface->addPlottable(sample2);
    chart_surface->addPlottable(sample3);
    QBrush boxBrush(QColor(60, 60, 255, 100));
    boxBrush.setStyle(Qt::Dense6Pattern); // make it look oldschool
    sample1->setBrush(boxBrush);
    sample2->setBrush(boxBrush);
    sample3->setBrush(boxBrush);

    // set data:
    sample1->setKey(1);
    sample1->setMinimum(1.1);
    sample1->setLowerQuartile(1.9);
    sample1->setMedian(2.25);
    sample1->setUpperQuartile(2.7);
    sample1->setMaximum(4.2);

    sample2->setKey(2);
    sample2->setMinimum(0.8);
    sample2->setLowerQuartile(1.6);
    sample2->setMedian(2.2);
    sample2->setUpperQuartile(3.2);
    sample2->setMaximum(4.9);
    sample2->setOutliers(QVector<double>() << 0.7 << 0.39 << 0.45 << 6.2 << 5.84);

    sample3->setKey(3);
    sample3->setMinimum(0.2);
    sample3->setLowerQuartile(0.7);
    sample3->setMedian(1.1);
    sample3->setUpperQuartile(1.6);
    sample3->setMaximum(2.9);

    // prepare manual x axis labels:
    chart_surface->xAxis->setSubTickCount(0);
    chart_surface->xAxis->setTickLength(0, 4);
    chart_surface->xAxis->setTickLabelRotation(20);
    chart_surface->xAxis->setAutoTicks(false);
    chart_surface->xAxis->setAutoTickLabels(false);
    chart_surface->xAxis->setTickVector(QVector<double>() << 1 << 2 << 3);
    chart_surface->xAxis->setTickVectorLabels(QVector<QString>() << "Sample 1" << "Sample 2" << "Control Group");

    // prepare axes:
    chart_surface->yAxis->setLabel(QString::fromUtf8("O₂ Absorption [mg]"));
    chart_surface->rescaleAxes();
    chart_surface->xAxis->scaleRange(1.7, chart_surface->xAxis->range().center());
    chart_surface->yAxis->setRange(0, 7);
    chart_surface->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void MainWindow::setupSimpleItemDemo(QCustomPlot *chart_surface)
{
    demoName = "Simple Item Demo";
    chart_surface->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    // add the text label at the top:
    QCPItemText *textLabel = new QCPItemText(chart_surface);
    chart_surface->addItem(textLabel);
    textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
    textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    textLabel->position->setCoords(0.5, 0); // place position at center/top of axis rect
    textLabel->setText("Text Item Demo");
    textLabel->setFont(QFont(font().family(), 16)); // make font a bit larger
    textLabel->setPen(QPen(Qt::black)); // show black border around text

    // add the arrow:
    QCPItemLine *arrow = new QCPItemLine(chart_surface);
    chart_surface->addItem(arrow);
    arrow->start->setParentAnchor(textLabel->bottom);
    arrow->end->setCoords(4, 1.6); // point to (4, 1.6) in x-y-plot coordinates
    arrow->setHead(QCPLineEnding::esSpikeArrow);
}

void MainWindow::setupItemDemo(QCustomPlot *chart_surface)
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
    QMessageBox::critical(this, "", "You're using Qt < 4.7, the animation of the item demo needs functions that are available with Qt 4.7 to work properly");
#endif

    demoName = "Item Demo";

    chart_surface->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    QCPGraph *graph = chart_surface->addGraph();
    int n = 500;
    double phase = 0;
    double k = 3;
    QVector<double> x(n), y(n);
    for (int i=0; i<n; ++i)
    {
        x[i] = i/(double)(n-1)*34 - 17;
        y[i] = qExp(-x[i]*x[i]/20.0)*qSin(k*x[i]+phase);
    }
    graph->setData(x, y);
    graph->setPen(QPen(Qt::blue));
    graph->rescaleKeyAxis();
    chart_surface->yAxis->setRange(-1.45, 1.65);
    chart_surface->xAxis->grid()->setZeroLinePen(Qt::NoPen);

    // add the bracket at the top:
    QCPItemBracket *bracket = new QCPItemBracket(chart_surface);
    chart_surface->addItem(bracket);
    bracket->left->setCoords(-8, 1.1);
    bracket->right->setCoords(8, 1.1);
    bracket->setLength(13);

    // add the text label at the top:
    QCPItemText *wavePacketText = new QCPItemText(chart_surface);
    chart_surface->addItem(wavePacketText);
    wavePacketText->position->setParentAnchor(bracket->center);
    wavePacketText->position->setCoords(0, -10); // move 10 pixels to the top from bracket center anchor
    wavePacketText->setPositionAlignment(Qt::AlignBottom|Qt::AlignHCenter);
    wavePacketText->setText("Wavepacket");
    wavePacketText->setFont(QFont(font().family(), 10));

    // add the phase tracer (red circle) which sticks to the graph data (and gets updated in bracketDataSlot by timer event):
    QCPItemTracer *phaseTracer = new QCPItemTracer(chart_surface);
    chart_surface->addItem(phaseTracer);
    itemDemoPhaseTracer = phaseTracer; // so we can access it later in the bracketDataSlot for animation
    phaseTracer->setGraph(graph);
    phaseTracer->setGraphKey((M_PI*1.5-phase)/k);
    phaseTracer->setInterpolating(true);
    phaseTracer->setStyle(QCPItemTracer::tsCircle);
    phaseTracer->setPen(QPen(Qt::red));
    phaseTracer->setBrush(Qt::red);
    phaseTracer->setSize(7);

    // add label for phase tracer:
    QCPItemText *phaseTracerText = new QCPItemText(chart_surface);
    chart_surface->addItem(phaseTracerText);
    phaseTracerText->position->setType(QCPItemPosition::ptAxisRectRatio);
    phaseTracerText->setPositionAlignment(Qt::AlignRight|Qt::AlignBottom);
    phaseTracerText->position->setCoords(1.0, 0.95); // lower right corner of axis rect
    phaseTracerText->setText("Points of fixed\nphase define\nphase velocity vp");
    phaseTracerText->setTextAlignment(Qt::AlignLeft);
    phaseTracerText->setFont(QFont(font().family(), 9));
    phaseTracerText->setPadding(QMargins(8, 0, 0, 0));

    // add arrow pointing at phase tracer, coming from label:
    QCPItemCurve *phaseTracerArrow = new QCPItemCurve(chart_surface);
    chart_surface->addItem(phaseTracerArrow);
    phaseTracerArrow->start->setParentAnchor(phaseTracerText->left);
    phaseTracerArrow->startDir->setParentAnchor(phaseTracerArrow->start);
    phaseTracerArrow->startDir->setCoords(-40, 0); // direction 30 pixels to the left of parent anchor (tracerArrow->start)
    phaseTracerArrow->end->setParentAnchor(phaseTracer->position);
    phaseTracerArrow->end->setCoords(10, 10);
    phaseTracerArrow->endDir->setParentAnchor(phaseTracerArrow->end);
    phaseTracerArrow->endDir->setCoords(30, 30);
    phaseTracerArrow->setHead(QCPLineEnding::esSpikeArrow);
    phaseTracerArrow->setTail(QCPLineEnding(QCPLineEnding::esBar, (phaseTracerText->bottom->pixelPoint().y()-phaseTracerText->top->pixelPoint().y())*0.85));

    // add the group velocity tracer (green circle):
    QCPItemTracer *groupTracer = new QCPItemTracer(chart_surface);
    chart_surface->addItem(groupTracer);
    groupTracer->setGraph(graph);
    groupTracer->setGraphKey(5.5);
    groupTracer->setInterpolating(true);
    groupTracer->setStyle(QCPItemTracer::tsCircle);
    groupTracer->setPen(QPen(Qt::green));
    groupTracer->setBrush(Qt::green);
    groupTracer->setSize(7);

    // add label for group tracer:
    QCPItemText *groupTracerText = new QCPItemText(chart_surface);
    chart_surface->addItem(groupTracerText);
    groupTracerText->position->setType(QCPItemPosition::ptAxisRectRatio);
    groupTracerText->setPositionAlignment(Qt::AlignRight|Qt::AlignTop);
    groupTracerText->position->setCoords(1.0, 0.20); // lower right corner of axis rect
    groupTracerText->setText("Fixed positions in\nwave packet define\ngroup velocity vg");
    groupTracerText->setTextAlignment(Qt::AlignLeft);
    groupTracerText->setFont(QFont(font().family(), 9));
    groupTracerText->setPadding(QMargins(8, 0, 0, 0));

    // add arrow pointing at group tracer, coming from label:
    QCPItemCurve *groupTracerArrow = new QCPItemCurve(chart_surface);
    chart_surface->addItem(groupTracerArrow);
    groupTracerArrow->start->setParentAnchor(groupTracerText->left);
    groupTracerArrow->startDir->setParentAnchor(groupTracerArrow->start);
    groupTracerArrow->startDir->setCoords(-40, 0); // direction 30 pixels to the left of parent anchor (tracerArrow->start)
    groupTracerArrow->end->setCoords(5.5, 0.4);
    groupTracerArrow->endDir->setParentAnchor(groupTracerArrow->end);
    groupTracerArrow->endDir->setCoords(0, -40);
    groupTracerArrow->setHead(QCPLineEnding::esSpikeArrow);
    groupTracerArrow->setTail(QCPLineEnding(QCPLineEnding::esBar, (groupTracerText->bottom->pixelPoint().y()-groupTracerText->top->pixelPoint().y())*0.85));

    // add dispersion arrow:
    QCPItemCurve *arrow = new QCPItemCurve(chart_surface);
    chart_surface->addItem(arrow);
    arrow->start->setCoords(1, -1.1);
    arrow->startDir->setCoords(-1, -1.3);
    arrow->endDir->setCoords(-5, -0.3);
    arrow->end->setCoords(-10, -0.2);
    arrow->setHead(QCPLineEnding::esSpikeArrow);

    // add the dispersion arrow label:
    QCPItemText *dispersionText = new QCPItemText(chart_surface);
    chart_surface->addItem(dispersionText);
    dispersionText->position->setCoords(-6, -0.9);
    dispersionText->setRotation(40);
    dispersionText->setText("Dispersion with\nvp < vg");
    dispersionText->setFont(QFont(font().family(), 10));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(bracketDataSlot()));
    dataTimer.start(0); // Interval 0 means to refresh as fast as possible
}

void MainWindow::setupStyledDemo(QCustomPlot *chart_surface)
{
    demoName = "Styled Demo";

    // prepare data:
    QVector<double> x1(20), y1(20);
    QVector<double> x2(100), y2(100);
    QVector<double> x3(20), y3(20);
    QVector<double> x4(20), y4(20);
    for (int i=0; i<x1.size(); ++i)
    {
        x1[i] = i/(double)x1.size()*10;
        y1[i] = qCos(x1[i]*0.8+qSin(x1[i]*0.16+1.0))*qSin(x1[i]*0.54)+1.4;
    }
    for (int i=0; i<x2.size(); ++i)
    {
        x2[i] = i/(double)x2.size()*10;
        y2[i] = qCos(x2[i]*0.85+qSin(x2[i]*0.165+1.1))*qSin(x2[i]*0.50)+1.7;
    }
    for (int i=0; i<x3.size(); ++i)
    {
        x3[i] = i/(double)x3.size()*10;
        y3[i] = 0.05+3*(0.5+qCos(x3[i]*x3[i]*0.2+2)*0.5)/(double)(x3[i]+0.7)+qrand()/(double)RAND_MAX*0.01;
    }
    for (int i=0; i<x4.size(); ++i)
    {
        x4[i] = x3[i];
        y4[i] = (0.5-y3[i])+((x4[i]-2)*(x4[i]-2)*0.02);
    }

    // create and configure plottables:
    QCPGraph *graph1 = chart_surface->addGraph();
    graph1->setData(x1, y1);
    graph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black, 1.5), QBrush(Qt::white), 9));
    graph1->setPen(QPen(QColor(120, 120, 120), 2));

    QCPGraph *graph2 = chart_surface->addGraph();
    graph2->setData(x2, y2);
    graph2->setPen(Qt::NoPen);
    graph2->setBrush(QColor(200, 200, 200, 20));
    graph2->setChannelFillGraph(graph1);

    QCPBars *bars1 = new QCPBars(chart_surface->xAxis, chart_surface->yAxis);
    chart_surface->addPlottable(bars1);
    bars1->setWidth(9/(double)x3.size());
    bars1->setData(x3, y3);
    bars1->setPen(Qt::NoPen);
    bars1->setBrush(QColor(10, 140, 70, 160));

    QCPBars *bars2 = new QCPBars(chart_surface->xAxis, chart_surface->yAxis);
    chart_surface->addPlottable(bars2);
    bars2->setWidth(9/(double)x4.size());
    bars2->setData(x4, y4);
    bars2->setPen(Qt::NoPen);
    bars2->setBrush(QColor(10, 100, 50, 70));
    bars2->moveAbove(bars1);

    // move bars above graphs and grid below bars:
    chart_surface->addLayer("abovemain", chart_surface->layer("main"), QCustomPlot::limAbove);
    chart_surface->addLayer("belowmain", chart_surface->layer("main"), QCustomPlot::limBelow);
    graph1->setLayer("abovemain");
    chart_surface->xAxis->grid()->setLayer("belowmain");
    chart_surface->yAxis->grid()->setLayer("belowmain");

    // set some pens, brushes and backgrounds:
    chart_surface->xAxis->setBasePen(QPen(Qt::white, 1));
    chart_surface->yAxis->setBasePen(QPen(Qt::white, 1));
    chart_surface->xAxis->setTickPen(QPen(Qt::white, 1));
    chart_surface->yAxis->setTickPen(QPen(Qt::white, 1));
    chart_surface->xAxis->setSubTickPen(QPen(Qt::white, 1));
    chart_surface->yAxis->setSubTickPen(QPen(Qt::white, 1));
    chart_surface->xAxis->setTickLabelColor(Qt::white);
    chart_surface->yAxis->setTickLabelColor(Qt::white);
    chart_surface->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    chart_surface->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    chart_surface->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    chart_surface->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    chart_surface->xAxis->grid()->setSubGridVisible(true);
    chart_surface->yAxis->grid()->setSubGridVisible(true);
    chart_surface->xAxis->grid()->setZeroLinePen(Qt::NoPen);
    chart_surface->yAxis->grid()->setZeroLinePen(Qt::NoPen);
    chart_surface->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    chart_surface->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    QLinearGradient plotGradient;
    plotGradient.setStart(0, 0);
    plotGradient.setFinalStop(0, 350);
    plotGradient.setColorAt(0, QColor(80, 80, 80));
    plotGradient.setColorAt(1, QColor(50, 50, 50));
    chart_surface->setBackground(plotGradient);
    QLinearGradient axisRectGradient;
    axisRectGradient.setStart(0, 0);
    axisRectGradient.setFinalStop(0, 350);
    axisRectGradient.setColorAt(0, QColor(80, 80, 80));
    axisRectGradient.setColorAt(1, QColor(30, 30, 30));
    chart_surface->axisRect()->setBackground(axisRectGradient);

    chart_surface->rescaleAxes();
    chart_surface->yAxis->setRange(0, 2);
}

void MainWindow::setupAdvancedAxesDemo(QCustomPlot *chart_surface)
{
    demoName = "Advanced Axes Demo";

    // configure axis rect:
    chart_surface->plotLayout()->clear(); // clear default axis rect so we can start from scratch
    QCPAxisRect *wideAxisRect = new QCPAxisRect(chart_surface);
    wideAxisRect->setupFullAxesBox(true);
    wideAxisRect->axis(QCPAxis::atRight, 0)->setTickLabels(true);
    wideAxisRect->addAxis(QCPAxis::atLeft)->setTickLabelColor(QColor("#6050F8")); // add an extra axis on the left and color its numbers
    QCPLayoutGrid *subLayout = new QCPLayoutGrid;
    chart_surface->plotLayout()->addElement(0, 0, wideAxisRect); // insert axis rect in first row
    chart_surface->plotLayout()->addElement(1, 0, subLayout); // sub layout in second row (grid layout will grow accordingly)
    //chart_surface->plotLayout()->setRowStretchFactor(1, 2);
    // prepare axis rects that will be placed in the sublayout:
    QCPAxisRect *subRectLeft = new QCPAxisRect(chart_surface, false); // false means to not setup default axes
    QCPAxisRect *subRectRight = new QCPAxisRect(chart_surface, false);
    subLayout->addElement(0, 0, subRectLeft);
    subLayout->addElement(0, 1, subRectRight);
    subRectRight->setMaximumSize(150, 150); // make bottom right axis rect size fixed 150x150
    subRectRight->setMinimumSize(150, 150); // make bottom right axis rect size fixed 150x150
    // setup axes in sub layout axis rects:
    subRectLeft->addAxes(QCPAxis::atBottom | QCPAxis::atLeft);
    subRectRight->addAxes(QCPAxis::atBottom | QCPAxis::atRight);
    subRectLeft->axis(QCPAxis::atLeft)->setAutoTickCount(2);
    subRectRight->axis(QCPAxis::atRight)->setAutoTickCount(2);
    subRectRight->axis(QCPAxis::atBottom)->setAutoTickCount(2);
    subRectLeft->axis(QCPAxis::atBottom)->grid()->setVisible(true);
    // synchronize the left and right margins of the top and bottom axis rects:
    QCPMarginGroup *marginGroup = new QCPMarginGroup(chart_surface);
    subRectLeft->setMarginGroup(QCP::msLeft, marginGroup);
    subRectRight->setMarginGroup(QCP::msRight, marginGroup);
    wideAxisRect->setMarginGroup(QCP::msLeft | QCP::msRight, marginGroup);
    // move newly created axes on "axes" layer and grids on "grid" layer:
    foreach (QCPAxisRect *rect, chart_surface->axisRects())
    {
        foreach (QCPAxis *axis, rect->axes())
        {
            axis->setLayer("axes");
            axis->grid()->setLayer("grid");
        }
    }

    // prepare data:
    QVector<double> x1a(20), y1a(20);
    QVector<double> x1b(50), y1b(50);
    QVector<double> x2(100), y2(100);
    QVector<double> x3, y3;
    qsrand(3);
    for (int i=0; i<x1a.size(); ++i)
    {
        x1a[i] = i/(double)(x1a.size()-1)*10-5.0;
        y1a[i] = qCos(x1a[i]);
    }
    for (int i=0; i<x1b.size(); ++i)
    {
        x1b[i] = i/(double)x1b.size()*10-5.0;
        y1b[i] = qExp(-x1b[i]*x1b[i]*0.2)*1000;
    }
    for (int i=0; i<x2.size(); ++i)
    {
        x2[i] = i/(double)x2.size()*10;
        y2[i] = qrand()/(double)RAND_MAX-0.5+y2[qAbs(i-1)];
    }
    x3 << 1 << 2 << 3 << 4;
    y3 << 2 << 2.5 << 4 << 1.5;

    // create and configure plottables:
    QCPGraph *mainGraph1 = chart_surface->addGraph(wideAxisRect->axis(QCPAxis::atBottom), wideAxisRect->axis(QCPAxis::atLeft));
    mainGraph1->setData(x1a, y1a);
    mainGraph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black), QBrush(Qt::white), 6));
    mainGraph1->setPen(QPen(QColor(120, 120, 120), 2));
    QCPGraph *mainGraph2 = chart_surface->addGraph(wideAxisRect->axis(QCPAxis::atBottom), wideAxisRect->axis(QCPAxis::atLeft, 1));
    mainGraph2->setData(x1b, y1b);
    mainGraph2->setPen(QPen(QColor("#8070B8"), 2));
    mainGraph2->setBrush(QColor(110, 170, 110, 30));
    mainGraph1->setChannelFillGraph(mainGraph2);
    mainGraph1->setBrush(QColor(255, 161, 0, 50));

    QCPGraph *graph2 = chart_surface->addGraph(subRectLeft->axis(QCPAxis::atBottom), subRectLeft->axis(QCPAxis::atLeft));
    graph2->setData(x2, y2);
    graph2->setLineStyle(QCPGraph::lsImpulse);
    graph2->setPen(QPen(QColor("#FFA100"), 1.5));

    QCPBars *bars1 = new QCPBars(subRectRight->axis(QCPAxis::atBottom), subRectRight->axis(QCPAxis::atRight));
    chart_surface->addPlottable(bars1);
    bars1->setWidth(3/(double)x3.size());
    bars1->setData(x3, y3);
    bars1->setPen(QPen(Qt::black));
    bars1->setAntialiased(false);
    bars1->setAntialiasedFill(false);
    bars1->setBrush(QColor("#705BE8"));
    bars1->keyAxis()->setAutoTicks(false);
    bars1->keyAxis()->setTickVector(x3);
    bars1->keyAxis()->setSubTickCount(0);

    // rescale axes according to graph's data:
    mainGraph1->rescaleAxes();
    mainGraph2->rescaleAxes();
    graph2->rescaleAxes();
    bars1->rescaleAxes();
    wideAxisRect->axis(QCPAxis::atLeft, 1)->setRangeLower(0);
}

void MainWindow::setupColorMapDemo(QCustomPlot *chart_surface)
{
    demoName = "Color Map Demo";

    // configure axis rect:
    chart_surface->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
    chart_surface->axisRect()->setupFullAxesBox(true);
    chart_surface->xAxis->setLabel("x");
    chart_surface->yAxis->setLabel("y");

    // set up the QCPColorMap:
    QCPColorMap *colorMap = new QCPColorMap(chart_surface->xAxis, chart_surface->yAxis);
    chart_surface->addPlottable(colorMap);
    int nx = 200;
    int ny = 200;
    colorMap->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
    colorMap->data()->setRange(QCPRange(-4, 4), QCPRange(-4, 4)); // and span the coordinate range -4..4 in both key (x) and value (y) dimensions
    // now we assign some data, by accessing the QCPColorMapData instance of the color map:
    double x, y, z;
    for (int xIndex=0; xIndex<nx; ++xIndex)
    {
        for (int yIndex=0; yIndex<ny; ++yIndex)
        {
            colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
            double r = 3*qSqrt(x*x+y*y)+1e-2;
            z = 2*x*(qCos(r+2)/r-qSin(r+2)/r); // the B field strength of dipole radiation (modulo physical constants)
            colorMap->data()->setCell(xIndex, yIndex, z);
        }
    }

    // add a color scale:
    QCPColorScale *colorScale = new QCPColorScale(chart_surface);
    chart_surface->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap->setColorScale(colorScale); // associate the color map with the color scale
    colorScale->axis()->setLabel("Magnetic Field Strength");

    // set the color gradient of the color map to one of the presets:
    colorMap->setGradient(QCPColorGradient::gpPolar);
    // we could have also created a QCPColorGradient instance and added own colors to
    // the gradient, see the documentation of QCPColorGradient for what's possible.

    // rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
    colorMap->rescaleDataRange();

    // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    QCPMarginGroup *marginGroup = new QCPMarginGroup(chart_surface);
    chart_surface->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    // rescale the key (x) and value (y) axes so the whole color map is visible:
    chart_surface->rescaleAxes();
}


void MainWindow::realtimeDataSlot()
{
    // calculate two new data points:
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
    double key = 0;
#else
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
#endif
    static double lastPointKey = 0;
    if (key-lastPointKey > 0.01) // at most add point every 10 ms
    {
        double value0 = qSin(key); //qSin(key*1.6+qCos(key*1.7)*2)*10 + qSin(key*1.2+0.56)*20 + 26;
        double value1 = qCos(key); //qSin(key*1.3+qCos(key*1.2)*1.2)*7 + qSin(key*0.9+0.26)*24 + 26;
        // add data to lines:
        ui->chart_surface->graph(0)->addData(key, value0);
        ui->chart_surface->graph(1)->addData(key, value1);
        // set data of dots:
        ui->chart_surface->graph(2)->clearData();
        ui->chart_surface->graph(2)->addData(key, value0);
        ui->chart_surface->graph(3)->clearData();
        ui->chart_surface->graph(3)->addData(key, value1);
        // remove data of lines that's outside visible range:
        ui->chart_surface->graph(0)->removeDataBefore(key-8);
        ui->chart_surface->graph(1)->removeDataBefore(key-8);
        // rescale value (vertical) axis to fit the current data:
        ui->chart_surface->graph(0)->rescaleValueAxis();
        ui->chart_surface->graph(1)->rescaleValueAxis(true);
        lastPointKey = key;
    }
    // make key axis range scroll with the data (at a constant range size of 8):
    ui->chart_surface->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
    ui->chart_surface->replot();

    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
        ui->statusBar->showMessage(
                    QString("%1 FPS, Total Data points: %2")
                    .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
                    .arg(ui->chart_surface->graph(0)->data()->count()+ui->chart_surface->graph(1)->data()->count())
                    , 0);
        lastFpsKey = key;
        frameCount = 0;
    }
}

void MainWindow::bracketDataSlot()
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
    double secs = 0;
#else
    double secs = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
#endif

    // update data to make phase move:
    int n = 500;
    double phase = secs*5;
    double k = 3;
    QVector<double> x(n), y(n);
    for (int i=0; i<n; ++i)
    {
        x[i] = i/(double)(n-1)*34 - 17;
        y[i] = qExp(-x[i]*x[i]/20.0)*qSin(k*x[i]+phase);
    }
    ui->chart_surface->graph()->setData(x, y);

    itemDemoPhaseTracer->setGraphKey((8*M_PI+fmod(M_PI*1.5-phase, 6*M_PI))/k);

    ui->chart_surface->replot();

    // calculate frames per second:
    double key = secs;
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
        ui->statusBar->showMessage(
                    QString("%1 FPS, Total Data points: %2")
                    .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
                    .arg(ui->chart_surface->graph(0)->data()->count())
                    , 0);
        lastFpsKey = key;
        frameCount = 0;
    }
}

void MainWindow::setupPlayground(QCustomPlot *chart_surface)
{
    Q_UNUSED(chart_surface)
}
*/

void MainWindow::screen_shot()
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

/*
 *
void MainWindow::allScreenShots()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QPixmap pm = QPixmap::grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#else
    QPixmap pm = qApp->primaryScreen()->grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#endif
    QString fileName = demoName.toLower()+".png";
    fileName.replace(" ", "");
    pm.save("./screenshots/"+fileName);

    if (currentDemoIndex < 19)
    {
        if (dataTimer.isActive())
            dataTimer.stop();
        dataTimer.disconnect();
        delete ui->chart_surface;
        ui->chart_surface = new QCustomPlot(ui->centralWidget);
        ui->verticalLayout->addWidget(ui->chart_surface);
        setupDemo(currentDemoIndex+1);
        // setup delay for demos that need time to develop proper look:
        int delay = 250;
        if (currentDemoIndex == 10) // Next is Realtime data demo
            delay = 12000;
        else if (currentDemoIndex == 15) // Next is Item demo
            delay = 5000;
        QTimer::singleShot(delay, this, SLOT(allScreenShots()));
    } else
    {
        qApp->quit();
    }
}
*/
