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

    static double last_time = 0;
    static double now = 0;
    static QTime time(QTime::currentTime());
    //释放空间
    xs.clear();
    QVector<double>().swap(xs);
    ys.clear();
    QVector<double>().swap(ys);

    int length = audioFormat.channel.toInt() * audioFormat.sampleRates[0].toInt()
                * RefreshTime/ 1000;
    int interval = length / PointNum;
//    length = qMin(length, AudioBufUsed.available());
//考虑最后一段qcustomplot对于大小的vector如何都正常显示    ？？？？？？
    AudioBufUsed.acquire(length);

    qint16 tmp = 0;
    last_time = now;
    now = time.elapsed() /1000.0; //minutes
    for(int i=0; i< PointNum; i++)
    {//处理完PointNum个点正好RefreshTime，而RefreshTime时间后定时器再次中断继续处理下一个RefreshTime间隔的点。
        xs.push_back(now + i * (RefreshTime / 1.0 / PointNum) / 1000.0);
        if(audioFormat.sampleSizes[0].toInt() == 8)
        {
            tmp = AudioBuffer[bufferpos];
            ys.push_back(AudioBuffer[bufferpos] / 1.0); //因为8位的范围-127~128，使曲线纵轴在[-1,1]
        }
        else if(audioFormat.sampleSizes[0].toInt() == 16)
        {
            tmp = AudioBuffer[2*bufferpos+1]<<8 | AudioBuffer[2*bufferpos];
            ys.push_back(tmp);//因为16位的范围-32767~32768
        }
        else
            ys.push_back(-1);//方便出错查找

        bufferpos = (interval + bufferpos)%AudioBufSize;
    }
    bufferpos += length % PointNum; // 保证bufferPos增加了length。
    AudioBufFree.release(length);

    if(AudioBufUsed.available() > 0)
    {
//        cout << "now" << now << RefreshTime / 1000.0 << "code interval " << now + PointNum * (RefreshTime / 1.0 / PointNum) / 1000.0
//             << "length" << length;
    //    cout << AudioBufUsed.available() << ys[0];
        emit dataProcessed(xs, ys);
    }

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
