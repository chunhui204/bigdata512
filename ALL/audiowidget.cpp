#include "audiowidget.h"
#include "ui_audiowidget.h"
#include "common.h"
#include <iostream>
#include <vector>
#include <string>
#include <QComboBox>

std::vector<std::string> split_stdString(const std::string &str, char ch);
QByteArray tensor_to_bytes(const Tensor &obj);

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
    ui->combo_audio->setInsertPolicy(QComboBox::InsertAtCurrent);
//    connect(ui->combo_audio, &QComboBox::currentIndexChanged, [=]()
//    {
//        qcout<< ui->combo_audio->currentIndex() << ui->combo_audio->currentText();
//    });
    this->ui->button_audio_recordStart->setDisabled(true);
    this->ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart->setDisabled(true);
    ui->button_audio_sampleEnd->setDisabled(true);
//    this->ui->button_audio_capEnd->setDisabled(true);
//    this->ui->button_audio_capStart->setDisabled(false);
}

AudioWidget::~AudioWidget()
{
    delete ui;
}
/**
 * 1.如果保证绘图线程处理速度尽量与接收线程同步，那么曲线和计算的label就是匹配的，（为了实时性应该是同步的）
 * 2.当接受完整个buffer长度才计算label会有些滞后性，如果一般数据就能预测那会实时性强
 */
void AudioWidget::onLabelCalculated(int label)
{
    if(label == -1)
        ui->label_audio_class->setText(QString("未知类别"));
    else
        ui->label_audio_class->setText(QString("类别")+QString::number(label, 10));
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
/**
 * @brief 开启终端麦克风
 */
void AudioWidget::on_button_audio_capStart_clicked()
{
//    this->ui->button_audio_capStart->setDisabled(true);
    this->ui->button_audio_recordStart->setDisabled(false);
    this->ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart->setDisabled(false);
    ui->button_audio_sampleEnd->setDisabled(true);
//    this->ui->button_audio_capEnd->setDisabled(false);

    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << QString("startAudio");
    emit commandIssued(array);
}
/**
 * @brief 关闭终端麦克风
 */
void AudioWidget::on_button_audio_capEnd_clicked()
{
    this->ui->button_audio_recordStart->setDisabled(true);
    this->ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart->setDisabled(true);
    ui->button_audio_sampleEnd->setDisabled(true);
//    this->ui->button_audio_capEnd->setDisabled(true);
//    this->ui->button_audio_capStart->setDisabled(false);

    emit recordStop();

    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << QString("stopAudio");
    emit commandIssued(array);
}
/**
 * @brief 开启录音
 */
void AudioWidget::on_button_audio_recordStart_clicked()
{
    this->ui->button_audio_recordStart->setDisabled(true);
    this->ui->button_audio_recordEnd->setDisabled(false);
    ui->button_audio_sampleStart->setDisabled(true);
    ui->button_audio_sampleEnd->setDisabled(true);
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
/**
 * @brief 结束录音
 */
void AudioWidget::on_button_audio_recordEnd_clicked()
{
    this->ui->button_audio_recordStart->setDisabled(false);
    this->ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart->setDisabled(false);
    ui->button_audio_sampleEnd->setDisabled(true);
    emit recordStop();
}
/**
 * @brief 保存路径改变
 * @param dir
 */
void AudioWidget::onSavePathChanged(const QString &dir)
{
    saveDir = dir;
}
/**
 * @brief 开始采集样本,和录音不能同时进行
 */
void AudioWidget::on_button_audio_sampleStart_clicked()
{
    //get label
    std::string str = ui->combo_audio->currentText().toStdString();
    if(str.empty())
    {
        return;
    }

    std::vector<std::string> vec = split_stdString(str, ' ');

    ui->button_audio_sampleStart->setDisabled(true);
    ui->button_audio_sampleEnd->setDisabled(false);
    this->ui->button_audio_recordStart->setDisabled(true);
    this->ui->button_audio_recordEnd->setDisabled(true);
    //拼接路径
    qDir.cd(saveDir);
    if(qDir.exists(AUDIO_PATH) == false)
        qDir.mkdir(AUDIO_PATH);
    qDir.cd(AUDIO_PATH);

    if(qDir.exists(AUDIO_TRAIN_PATH) == false)
        qDir.mkdir(AUDIO_TRAIN_PATH);
    qDir.cd(AUDIO_TRAIN_PATH);

    QDateTime date =QDateTime::currentDateTime();
    QString name =qDir.absolutePath() + "/" + QString::fromStdString(vec[0])
            + "_" + date.toString("yyyy-MM-dd-hh-mm") + ".wav";
    qcout << name;

    emit recordStart(name);
}
/**
 * @brief 结束采集
 */
void AudioWidget::on_button_audio_sampleEnd_clicked()
{
    this->ui->button_audio_recordStart->setDisabled(false);
    this->ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart->setDisabled(false);
    ui->button_audio_sampleEnd->setDisabled(true);
    emit recordStop();
}
/**
 * @brief 添加label信息
 */
void AudioWidget::on_button_audio_labelAdd_clicked()
{
    QString text = ui->lineEdit_audio->text().simplified();//delete duplicate spaces
    ui->lineEdit_audio->clear();

    if(text.isEmpty())
        return;
    if(ui->combo_audio->findText(text) == -1)
        ui->combo_audio->addItem(text);

}

void AudioWidget::on_button_audio_train_clicked()
{
    ui->button_audio_train->setDisabled(true);
    qDir.cd(saveDir);
    if(qDir.exists(AUDIO_PATH) == true)
        qDir.cd(AUDIO_PATH);
    if(qDir.exists(AUDIO_TRAIN_PATH) == true)
        qDir.cd(AUDIO_TRAIN_PATH);

    QStringList files = QFileDialog::getOpenFileNames(this, "选择训练文件",
                            qDir.absolutePath(), tr("AUDIO files (*.wav)"));
    if(false == files.isEmpty())
    {
        QVector<QString> filenames;
        QVector<int> labels;
        for(QStringList::iterator i=files.begin();i!=files.end();i++)
        {
            filenames.push_back(*i);

            std::vector<std::string> name = split_stdString((*i).toStdString(), '/');
            std::vector<std::string> label_t = split_stdString(name[name.size()-1], '_');
            labels.push_back( std::stoi(label_t[0]));
        }
        emit trainModel(filenames, labels);
    }
}
void AudioWidget::onAlreadyTrained(const Tensor &m_featMean, const Tensor &m_featStd, const Tensor &m_gate)
{
    ui->button_audio_train->setDisabled(false);

    QByteArray featMean = tensor_to_bytes(m_featMean);
    QByteArray featStd = tensor_to_bytes(m_featStd);
    QByteArray gate = tensor_to_bytes(m_gate);

    QByteArray snd;
    QDataStream ss(&snd, QIODevice::WriteOnly);
    ss << QString("paramesters")
       << qint64(featMean.size()) << featMean << qint64(featStd.size()) << featStd
       << qint64(gate.size()) << gate;
    emit commandIssued(snd);
}

/**
 * @brief split_stdString by charactor, util function
 * @param str
 * @param vec
 */
std::vector<std::string> split_stdString(const std::string &str, char ch)
{
    std::vector<std::string> vec;
    std::string::const_iterator start = str.begin(), end = str.begin();

    while(end != str.end())
    {
        while(*end == ch && end != str.end())
            end++;
        start = end;

        while(*end != ch && end != str.end())
            end++;

        if(end != start)
        {
            //valid substr
            vec.push_back(std::string(start, end));
        }
    }
    return vec;
}
/**
 * @brief tensor_to_bytes,小端模式：低字节-低地址
 * @param arr
 * @param len
 * @return
 */
QByteArray tensor_to_bytes(const Tensor &obj)
{
    const int unitbyte = sizeof(double);
    const int size = obj.rows() * obj.cols() * unitbyte;
    Tensor::const_iterator ptr = obj.begin();
    QByteArray ret(size, 0);

    for(int i=0; i< obj.rows()*obj.cols(); i++)
    {
        for(int j=0; j< unitbyte; j++)
        {
            ret[i*unitbyte + j]=((char*)(ptr + i))[j];
        }
    }
    return ret;
}
