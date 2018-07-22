#include "audioplotthread.h"

AudioPlotThread::AudioPlotThread(QObject *parent) : QObject(parent)
{
    bufferpos = 0;
    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, &AudioPlotThread::dataTranslation);
    //50ms刷新一次
    timer->start(50);
}

AudioPlotThread::~AudioPlotThread()
{
}

void AudioPlotThread::dataTranslation()
{

}
