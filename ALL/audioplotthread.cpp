#include "audioplotthread.h"
#include <QDateTime>
#include <QTime>

AudioPlotThread::AudioPlotThread(QObject *parent) : QObject(parent)
{
    bufferpos = 0;
    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, &AudioPlotThread::dataTranslation);
    //50ms刷新一次
    timer->start(RefreshTime);
    timeCount.start();
}

AudioPlotThread::~AudioPlotThread()
{
    xs.clear();
    QVector<double>().swap(xs);
    ys.clear();
    QVector<double>().swap(ys);

    if(timer->isActive())
        timer->stop();
    delete timer;
}

void AudioPlotThread::dataTranslation()
{
    //还未设置format
    if(audioFormat.channel.isNull())
        return;
    //释放空间
    xs.clear();
    QVector<double>().swap(xs);
    ys.clear();
    QVector<double>().swap(ys);

    int length = audioFormat.channel.toInt() * audioFormat.sampleRates[0].toInt()
                * RefreshTime/ 1000;
//    length = qMin(length, AudioBufUsed.available());
//考虑最后一段qcustomplot对于大小的vector如何都正常显示    ？？？？？？
    AudioBufUsed.acquire(length);

    qint16 tmp = 0;
    double now = timeCount.elapsed() / 1000 / 60; //minutes
    for(int i=0; i<length; i++)
    {
        xs.push_back(now + i * RefreshTime / 1000 / length);
        if(audioFormat.sampleSizes[0].toInt() == 8)
        {
            tmp = AudioBuffer[bufferpos];
            double t = tmp/128.0;
            ys.push_back(tmp / 128.0); //因为8位的范围-127~128，使曲线纵轴在[-1,1]
        }
        else if(audioFormat.sampleSizes[0].toInt() == 16)
        {
            tmp = AudioBuffer[2*bufferpos]<<8 | AudioBuffer[2*bufferpos+1];
            ys.push_back(tmp / 32768.0);//因为16位的范围-32767~32768
        }
        else
            ys.push_back(-1);//方便出错查找

        bufferpos = (1 + bufferpos)%AudioBufSize;
    }
    AudioBufFree.release(length);

//    cout << AudioBufUsed.available() << ys[0];
    emit dataProcessed(xs, ys);

}

void AudioPlotThread::onAudioFormatInit(const QString &rate, const QString &chns, const QString &size)
{
    audioFormat.deviceName="";
    audioFormat.codec="";
    audioFormat.sampleRates.clear();
    audioFormat.sampleSizes.clear();
    audioFormat.sampleRates.append(rate);
    audioFormat.sampleSizes.append(size);
    audioFormat.channel = chns;
}
