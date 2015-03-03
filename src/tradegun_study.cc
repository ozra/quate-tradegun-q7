#include "tradegun_study.hh"



/*
 *
 *TODO*

class PlotStyle {
   public:
    QPen                pen;
    QBrush              brush;
};

class PlotPalette {
   public:
    vector<PlotStyle>    styles;
};

class Series {
   public:
    int                 type;
    QString             title;
    PlotPalette         palette;
    QCPDataCollection*  data;
};

class ChartStudy {
   public:
    QCPCustomPlot*              chart_surface;
    vector<QCPAxisRect*>        panes;
    vector<Series*>             series;
};

*/




TradeGun_Study::TradeGun_Study(QObject *parent) :
    QObject(parent)
{

}
