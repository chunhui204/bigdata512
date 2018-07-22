#ifndef AUDIOBASE_H
#define AUDIOBASE_H

#include <QObject>
#include <QAudio>
#include <QIODevice>
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QAudioInput>
#include <QByteArray>
#include <QList>
#include <QDataStream>
#include <QDebug>

const qint64 BufferDurationUs       = 1 * 1000000;// 单位： seconds
const int    NotifyIntervalMs       = 500;

const int    WaveplotDurationMs     =100;

class AudioBase : public QObject
{
    Q_OBJECT
public:
    explicit AudioBase(QObject *parent = 0);
    ~AudioBase();
    QByteArray audioInfoToTcp() const;

    void startAudio();
    void stopAudio();
    void resetAudioFormat(const QString & head);

//    char * buffer() const;
signals:
    void dataReadyEvent(const QByteArray* ,qint64 startPos, qint64 endPos);
public slots:

private:
    qint64 audioBufferLength(const QAudioFormat &format) const;
    void initializeAudio();
private slots:
    void audioDataReady();

private:
    QByteArray *m_buffer;
    //上位机读到的位置，当前数据长度，总长度
    qint64 m_tcpReadPosition;
    qint64 m_currentPosition;
    const qint64 m_totalLength;
    //audio 设备
    QAudioFormat m_format;
    QAudioInput *m_audioInput;
    QIODevice *audioIO;

};

#endif // AUDIOBASE_H
