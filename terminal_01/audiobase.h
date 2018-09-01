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
#include <QTime>
#include <QAudioOutput>

#define cout qDebug()<< __FILE__<<__LINE__

const qint64 BufferDurationUs       = 1 * 1000000;// 单位： seconds
const int    NotifyIntervalMs       = 50;

class AudioBase : public QObject
{
    Q_OBJECT
public:
    explicit AudioBase(QObject *parent = 0);
    ~AudioBase();
    QByteArray audioInfoToTcp() const;

    void startAudio();
    void stopAudio();

//    char * buffer() const;
signals:
    void dataReadyEvent(const QByteArray & ,qint64 startPos, qint64 endPos);
    void audioError(QString str);
public slots:
    void onAudioFormatChanged(const QString &,const QString &,const QString &,const QString &,const QString &);

private:
    qint64 audioBufferLength(const QAudioFormat &format) const;
    void initializeAudio();
    void initializeAudioInput();
private slots:
    void audioDataReady();

private:
    QByteArray m_buffer;
    //上位机读到的位置，当前数据长度，总长度
    qint64 m_tcpReadPosition;
    qint64 m_currentPosition;
    qint64 m_totalLength;
    //audio 设备
    QAudioFormat m_format;
    QAudioInput *m_audioInput;
    QAudioDeviceInfo inputDevice;
    QIODevice *audioIO;
    QAudioOutput *output;
};

#endif // AUDIOBASE_H
