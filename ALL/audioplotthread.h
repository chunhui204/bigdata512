#ifndef AUDIOPLOTTHREAD_H
#define AUDIOPLOTTHREAD_H

#include <QObject>
#include "common.h"
#include <QTimer>
#include <QTime>
#include "wavfile.h"
#include <QDir>

const int RefreshTime = 30; //ms
class AudioPlotThread : public QObject
{
    Q_OBJECT
public:
    explicit AudioPlotThread(QObject *parent = 0);
    ~AudioPlotThread();

public slots:
    void onAudioFormatInit(const QString &rate, const QString &chns, const QString &size);
    void onRecordStart(const QString &);
    void onRecordStop();
    void onWavFileOpened(const QString &name);
//    void
signals:
    void dataProcessed(const QVector<double> &xs, const QVector<double> &ys);

private slots:
    void dataTranslation();
private:

private:
    int bufferpos;
    QTimer *timer;
//    QTime timeCount;
    bool isPlotBefore; //指定是否为绘图之前，一旦开始绘图此标志位false
    //坐标
    QVector<double> xs;
    QVector<double> ys;
    QAudioFormat audioFormat;
    bool isFormatReady;
    //保存录音
    WavFile *wavFile;
    bool isSaveRecording; //保存录音？
    bool isSaveTraining;  //保存训练样本？
    QString recordFile;
    int recordpos;
};

#endif // AUDIOPLOTTHREAD_H
