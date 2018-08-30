#include "audioplotthread.h"
#include <QTime>

AudioPlotThread::AudioPlotThread(QObject *parent) : QObject(parent)
{
    bufferpos = 0;
    recordpos = 0;
    isPlotBefore = true;
    isSaveRecording = false;
    isSaveTraining = false;

    timer = new QTimer(this);
    wavFile = new WavFile(this);

    connect(timer, &QTimer::timeout, this, &AudioPlotThread::dataTranslation);
    //30ms刷新一次
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
}

void AudioPlotThread::dataTranslation()
{
    //还未设置format,说明未连上设备
    if(audioFormat.channelCount() == -1)
        return;
    //释放空间
    if(xs.size() > 0)
    {
        xs.clear();
        QVector<double>().swap(xs);
    }
    if(ys.size() > 0)
    {
        ys.clear();
        QVector<double>().swap(ys);
    }
//    static QTime time = QTime::currentTime();
    static double now = 0;//time.elapsed() /1000.0;
    //显示时出现每段音频间有连线或不同时间段内分辨率不同都是因为横轴时间量有误
    //这里显示RefreshTime+50是尽快的显示做到实时效果，如果不加的话每段length取整会损失精度
    //，就是说每段显示的数据长度都比这个时间段该显示的少一点点时间长了会造成累计误差滞后严重。
    int length = audioFormat.channelCount() * audioFormat.sampleRate()
            *audioFormat.sampleSize() / 8 * (RefreshTime+5)/ 1000 ;

    length -= length % 2;
    Q_ASSERT(AudioBufUsed.available() % 2 == 0);

    length = qMin(length, AudioBufUsed.available());

    if(length > 0 && isPlotBefore== true) //模拟采集按键被按下
    {
        isPlotBefore = false;
        emit dataProcessed(xs, ys);//清空曲线
    }

    int unitByte = audioFormat.channelCount() *audioFormat.sampleSize() / 8;
    int pointNum = length / unitByte;

    AudioBufUsed.acquire(length);
    //写文件,在处理到数组最后的时将内容统一写入文件
    if(isSaveRecording && bufferpos + length >= AudioBufSize)
    {
        wavFile->writeWave(recordFile, audioFormat, AudioBuffer + recordpos, AudioBufSize - recordpos);
        recordpos = 0;
    }

    for(int i=0; i< pointNum; i++)
    {//处理完PointNum个点正好RefreshTime，而RefreshTime时间后定时器再次中断继续处理下一个RefreshTime间隔的点。
        now = now + 1.0 / audioFormat.sampleRate();
//        cout << now;
        xs.push_back(now);
        if(unitByte == 1)
        {
            ys.push_back(AudioBuffer[bufferpos] / 128.0); //因为8位的范围-127~128，使曲线纵轴在[-1,1]
        }
        else if(unitByte == 2)
        {
            qint16 *ptr = reinterpret_cast<qint16 *>(AudioBuffer + bufferpos);
            ys.push_back(*ptr / 32768.0);//因为16位的范围-32767~32768
        }
        bufferpos = (unitByte + bufferpos) % AudioBufSize;
    }
    AudioBufFree.release(length);

    if(xs.isEmpty() == false)
    {//只在有值的情况下发送，防止之前的曲线被清空
        emit dataProcessed(xs, ys);
    }

}

void AudioPlotThread::onRecordStart(const QString &name)
{
    recordFile = name;
    isSaveRecording = true;
    recordpos = bufferpos;
}

void AudioPlotThread::onRecordStop()
{
    isSaveRecording = false;
    Q_ASSERT(bufferpos >= recordpos);

    if(recordFile.isEmpty() == false)
        wavFile->writeWave(recordFile, audioFormat, AudioBuffer + recordpos, bufferpos - recordpos);
    recordFile.clear();
}
//显示wav file的音频信号
void AudioPlotThread::onWavFileOpened(const QString &name)
{   //关闭录音
    onRecordStop();
    //声音采集已关闭，所以又处于绘制实时曲线之前
    isPlotBefore = true;

    WavFile file;
    if(false == file.open(name, true))
        return;

    QByteArray array = file.readAll();
    file.close();
    xs.clear();
    QVector<double>().swap(xs);
    ys.clear();
    QVector<double>().swap(ys);
//清空界面
    emit dataProcessed(xs, ys);

    double now = 0;
    quint32 length = file.dataLength();
    length -= length % 2;

    int unitByte = file.fileFormat().channelCount() * file.fileFormat().sampleSize() / 8;

    for(int i=0; i < length; i+=unitByte)
    {//处理完PointNum个点正好RefreshTime，而RefreshTime时间后定时器再次中断继续处理下一个RefreshTime间隔的点。
        now = now + 1.0 / file.fileFormat().sampleRate();
        xs.push_back(now);
        if(unitByte == 1)
            ys.push_back(array[i] / 128.0); //因为8位的范围-127~128，使曲线纵轴在[-1,1]
        else if(unitByte == 2)
        {
            qint16 *ptr = reinterpret_cast<qint16 *>(array.data() + i);
            ys.push_back(*ptr / 32768.0);//因为16位的范围-32767~32768
        }else
        {
            qint16 *ptr = reinterpret_cast<qint16 *>(array.data() + i);
            ys.push_back(*ptr / 32768.0);
        }
    }
    emit dataProcessed(xs, ys);
}

void AudioPlotThread::onAudioFormatInit(const QString &rate, const QString &chns, const QString &size)
{
    audioFormat.setSampleRate(rate.toInt());
    audioFormat.setSampleSize(size.toInt());
    audioFormat.setChannelCount(chns.toInt());
}

