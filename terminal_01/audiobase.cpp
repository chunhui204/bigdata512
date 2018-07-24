#include "audiobase.h"

AudioBase::AudioBase(QObject *parent) : QObject(parent)
{
    initializeAudio();

}
AudioBase::~AudioBase()
{
    delete m_buffer;
}

qint64 AudioBase::audioBufferLength(const QAudioFormat &format) const
{
    qint64 len = (format.sampleRate() * format.channelCount() * (format.sampleSize() / 8))
            * (BufferDurationUs / 1000000);
    len -= len % (format.channelCount() * (format.sampleSize() / 8));
    return len;
}

//初始化个变量值
void AudioBase::initializeAudio()
{
    //初始化audioFormat
    QAudioFormat format;
    // Set up the desired format, for example:
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleSize(8);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(format)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
        format = info.nearestFormat(format);
    }
        m_format = format;
        //初始化缓冲
        m_totalLength = audioBufferLength(m_format);
        m_currentPosition = 0;
        m_tcpReadPosition = 0;
        m_buffer = new QByteArray;
        m_buffer->resize(m_totalLength);

   //初始化AudioInput
        m_audioInput = new QAudioInput(m_format, this);
        //设置通知间隔，每次来通知时通过网络向上位机发送数据
        m_audioInput->setNotifyInterval(NotifyIntervalMs);
        connect(m_audioInput, &QAudioInput::notify,
                [=]()
        {//如果有数据可以传输
            if(m_tcpReadPosition != m_currentPosition)
            {

                qint64 endpos = m_currentPosition;
                if(m_tcpReadPosition < m_currentPosition)
                {
                    //当缓冲数组存满时
                    endpos = m_totalLength;
                    m_tcpReadPosition = 0;
                }
                emit dataReadyEvent(m_buffer, m_tcpReadPosition, endpos);
            }
        });

    audioIO = NULL;


}

void AudioBase::audioDataReady()
{
    //更新m_buffer
    qint64 dataLength = m_audioInput->bytesReady();
    qint64 freeLength = m_totalLength - m_currentPosition;
    dataLength = qMin(dataLength, freeLength);

    qint64 readLength = audioIO->read(
                m_buffer->data()+m_currentPosition, dataLength);
    if(readLength)
    {
        //如果读到了数据，则更新参数
        m_currentPosition += readLength;

        if(m_currentPosition >= m_totalLength)
        {
            //当缓冲数组满时，清空缓冲，从数组开始的地方覆盖数据
            m_currentPosition = 0;
        }
    }
}

//开启声音采集
void AudioBase::startAudio()
{
    audioIO = m_audioInput->start();
    //每次声卡采集到数据，IOdevice会受到新数据触发readyread信号。
    //更新m_buffer
    connect(audioIO, &QIODevice::readyRead, this, &AudioBase::audioDataReady);

}
void AudioBase::stopAudio()
{
    m_audioInput->stop();
    //清空buffer，重新采集
    m_buffer->clear();
    m_buffer->resize(m_totalLength);
    m_currentPosition = 0;
    m_tcpReadPosition = 0;
}

/* 获取音频设备信息（输入设备）
 * tcp信息头格式：
 * audioDeviceInfo##_deviceName##_sampleRate0,_sampleRate1...##_channel##_sampleSize##_audioCodec###
 * 上式中带"_"均为变量,如果每种变量存在多个值则以","分隔, _channel取1.
 * 如果有多个设备，上述从_deviceName的字符串循环添加。
 */
QByteArray AudioBase::audioInfoToTcp() const
{
    //获取输入设备信息
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << QString("audioDeviceInfo");

    //先发送正在使用的设备信息
    QStringList s3;
    s3.append(QString("%1").arg(m_format.sampleRate()));
    QStringList s4;
    s4.append(QString("%1").arg(m_format.sampleSize()));
    stream << s3 << QString("%1").arg(m_format.channelCount()) << s4;

    //发送可选的设备信息
    QList<QAudioDeviceInfo> availableInputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    foreach (QAudioDeviceInfo inDevice, availableInputDevices)
    {
        stream << inDevice.deviceName();
        QStringList s1;
        foreach (int sr, inDevice.supportedSampleRates())
        {
            s1.append(QString("%1").arg(sr));
        }
        stream << s1;
        stream << QString("1");

        QStringList s2;
        foreach (int ss, inDevice.supportedSampleSizes())
        {
            s2.append(QString("%1").arg(ss));
        }
        stream << s2;
        stream << inDevice.supportedCodecs()[0];
    }
    return array;
}
