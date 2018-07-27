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
    ui->customPlot->yAxis->setRange(-1,1);//(-32768, 32768);
    ui->customPlot->xAxis->setRange(0, 5, Qt::AlignLeft);
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

    //其他初始化
    saveDir = QDir::currentPath();

    this->ui->button_audio_recordStart->setDisabled(true);
    this->ui->button_audio_recordEnd->setDisabled(true);
//    this->ui->button_audio_capEnd->setDisabled(true);
//    this->ui->button_audio_capStart->setDisabled(false);
}

AudioWidget::~AudioWidget()
{
    delete ui;
}

void AudioWidget::onDataProcessed(const QVector<double> &xs, const QVector<double> &ys)
{
    if(xs.isEmpty()== true)
        ui->customPlot->graph(0)->setData(xs, ys); // 清空界面
    else
    {
        for(int i=0; i< xs.size(); i++)
        {
            ui->customPlot->graph(0)->addData(xs[i], ys[i]);
        }
        ui->customPlot->xAxis->setRange(xs[0], 5, Qt::AlignHCenter);
    }

    ui->customPlot->replot();
}

void AudioWidget::on_button_audio_capStart_clicked()
{
//    this->ui->button_audio_capStart->setDisabled(true);
    this->ui->button_audio_recordStart->setDisabled(false);
    this->ui->button_audio_recordEnd->setDisabled(true);
//    this->ui->button_audio_capEnd->setDisabled(false);
    emit commandIssued(QString("startAudio"));
}

void AudioWidget::on_button_audio_capEnd_clicked()
{
    this->ui->button_audio_recordStart->setDisabled(true);
    this->ui->button_audio_recordEnd->setDisabled(true);
//    this->ui->button_audio_capEnd->setDisabled(true);
//    this->ui->button_audio_capStart->setDisabled(false);

    emit recordStop();
    emit commandIssued(QString("stopAudio"));
}

void AudioWidget::on_button_audio_recordStart_clicked()
{
    this->ui->button_audio_recordStart->setDisabled(true);
    this->ui->button_audio_recordEnd->setDisabled(false);
    //拼接路径
    qDir.cd(saveDir);
//    cout << saveDir;
    if(qDir.exists(AUDIO_PATH) == false)
        qDir.mkdir(AUDIO_PATH);
    qDir.cd(AUDIO_PATH);

    if(qDir.exists(AUDIO_FILE_PATH) == false)
        qDir.mkdir(AUDIO_FILE_PATH);
    qDir.cd(AUDIO_FILE_PATH);

    QDateTime date =QDateTime::currentDateTime();
    QString name =qDir.absolutePath() + "/" + date.toString("yyyy-MM-dd-hh-mm") + ".wav";
    emit recordStart(name);
}

void AudioWidget::on_button_audio_recordEnd_clicked()
{
    this->ui->button_audio_recordStart->setDisabled(false);
    this->ui->button_audio_recordEnd->setDisabled(true);
    emit recordStop();
}
void AudioWidget::onSavePathChanged(const QString &dir)
{
    qDir.cd(dir);

    if(qDir.exists(AUDIO_PATH) == false)
        qDir.mkdir(AUDIO_PATH);
    qDir.cd(AUDIO_PATH);

    if(qDir.exists(AUDIO_FILE_PATH) == false)
        qDir.mkdir(AUDIO_FILE_PATH);
    if(qDir.exists(AUDIO_TRAIN_PATH) == false)
        qDir.mkdir(AUDIO_TRAIN_PATH);
}
