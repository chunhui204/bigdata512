#include "audiowidget.h"
#include "ui_audiowidget.h"
#include "common.h"

AudioWidget::AudioWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AudioWidget)
{
    ui->setupUi(this);

    // 初始化绘图组件customPlot
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
//    ui->customPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue
    //y周范围
    ui->customPlot->yAxis->setRange(-32768, 32768);
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%m:%s");
    ui->customPlot->xAxis->setTicker(timeTicker);

    ui->customPlot->axisRect()->setupFullAxesBox();
    // 上方和右边坐标显示
    ui->customPlot->xAxis2->setVisible(true);
    ui->customPlot->xAxis2->setTickLabels(false);
    ui->customPlot->yAxis2->setVisible(true);
    ui->customPlot->yAxis2->setTickLabels(false);
    // make left and bottom axes always transfer their ranges to right and top axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));
    // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
//    ui->customPlot->graph(0)->rescaleAxes();
    // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
    ui->customPlot->axisRect(0)->setRangeDrag(Qt::Horizontal);
    ui->customPlot->axisRect(0)->setRangeZoom(Qt::Horizontal);

    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

AudioWidget::~AudioWidget()
{
    delete ui;
}

void AudioWidget::onDataProcessed(const QVector<double> &xs, const QVector<double> &ys)
{
//    cout <<"size" << xs.size();
//    for(int i=0; i< xs.size(); i++)
//    {
//        ui->customPlot->graph(0)->addData(xs[i], ys[i]);
//        ui->customPlot->xAxis->setRange(xs[i], 8, Qt::AlignRight);
//    }

//    ui->customPlot->replot();

    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    static double lastPointKey = 0;
    if (xs[0]-lastPointKey > 0.002) // at most add point every 2 ms
    {
        cout << "key" << xs[0] << "value" << ys[0];
      // add data to lines:

      ui->customPlot->graph(0)->addData(xs[0], ys[0]);
      // rescale value (vertical) axis to fit the current data:
      //ui->customPlot->graph(0)->rescaleValueAxis();
      //ui->customPlot->graph(1)->rescaleValueAxis(true);
      lastPointKey = xs[0];
    }
    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlot->xAxis->setRange(xs[0], 8, Qt::AlignRight);
    ui->customPlot->replot();
}

void AudioWidget::on_button_audio_capStart_clicked()
{
    cout<<"start";
    emit commandIssued(QString("startAudio"));
}

void AudioWidget::on_button_audio_capEnd_clicked()
{
    emit commandIssued(QString("stopAudio"));
}
