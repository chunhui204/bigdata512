#include "audiowidget.h"
#include "ui_audiowidget.h"

AudioWidget::AudioWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AudioWidget)
{
    ui->setupUi(this);

    // 初始化绘图组件customPlot
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
    ui->customPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue
    //y周范围
    ui->customPlot->yAxis->setRange(-10, 10);
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m");
    ui->customPlot->xAxis->setTicker(timeTicker);
    // 上方和右边坐标显示
    ui->customPlot->xAxis2->setVisible(true);
    ui->customPlot->xAxis2->setTickLabels(false);
    ui->customPlot->yAxis2->setVisible(true);
    ui->customPlot->yAxis2->setTickLabels(false);
    // make left and bottom axes always transfer their ranges to right and top axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));
    // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
    ui->customPlot->graph(0)->rescaleAxes();
    // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
    ui->customPlot->axisRect(0)->setRangeDrag(Qt::Horizontal);
    ui->customPlot->axisRect(0)->setRangeZoom(Qt::Horizontal);

    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}

AudioWidget::~AudioWidget()
{
    delete ui;
}

void AudioWidget::onDataProcessed(const QVector<double> &xs, const QVector<double> &ys)
{
    ui->customPlot->graph(0)->setData(xs, ys);
    ui->customPlot->replot();
}

void AudioWidget::on_button_audio_capStart_clicked()
{
    commandIssued("startAudio");
}

void AudioWidget::on_button_audio_capEnd_clicked()
{
    commandIssued("stopAudio");
}
