#include "audiothread.h"

AudioThread::AudioThread(QObject *parent) : QObject(parent)
{
    audioSocket = new QTcpSocket(this);

}
void AudioThread::connectHost(QString ip, int port)
{
    audioSocket->connectToHost(QHostAddress(ip), port);
}
AudioThread::~AudioThread()
{
    if(audioSocket->isValid())
    {
        audioSocket->disconnectFromHost();
        audioSocket->close();
    }
}
void AudioThread::onDataReadyEvent(const QByteArray& buffer, qint64 startPos, qint64 endPos)
{
    QByteArray array;
    QByteArray buffer_t(buffer.data()+startPos, endPos-startPos);
    QDataStream stream(&array, QIODevice::WriteOnly);

    stream << endPos - startPos << buffer_t;

    if(audioSocket->isValid())
        audioSocket->write(array);
}
