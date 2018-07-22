#include "audioplotthread.h"

AudioPlotThread::AudioPlotThread(QObject *parent) : QObject(parent)
{
    bufferpos = 0;
    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, &AudioPlotThread::dataTranslation);
    //50ms刷新一次
    timer->start(RefreshTime);
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
    if(audioFormat.deviceName.isNull())
        return;
    //释放空间
    xs.clear();
    QVector<double>().swap(xs);
    ys.clear();
    QVector<double>().swap(ys);

    int length = audioFormat.channel.toInt() * audioFormat.sampleRates[0].toInt()
                * int(RefreshTime/ 1000);
    AudioBufFree.acquire();

    qint16 tmp = 0;
    for(int i=0; i<length; i++)
    {
        xs.push_back(i);
        if(audioFormat.sampleSizes[0].toInt() == 8)
        {
            tmp = AudioBuffer[bufferpos];
            ys.push_back(tmp / 128); //因为8位的范围-127~128，使曲线纵轴在[-1,1]
        }
        else if(audioFormat.sampleSizes[0].toInt() == 16)
        {
            tmp = AudioBuffer[2*bufferpos]<<8 | AudioBuffer[2*bufferpos+1];
            ys.push_back(tmp / 32768);//因为16位的范围-32767~32768
        }
        else
            ys.push_back(-1);//方便出错查找

        bufferpos = (1 + bufferpos)%AudioBufSize;
    }
    AudioBufUsed.release();

    emit dataProcessed(xs, ys);

}

void AudioPlotThread::onAudioFormatChanged(const AudioSettingFormat & format)
{
    audioFormat = format;
}
