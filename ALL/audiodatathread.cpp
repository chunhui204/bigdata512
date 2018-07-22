#include "audiodatathread.h"
#include <QHostAddress>

AudioDataThread::AudioDataThread(QObject *parent) : QObject(parent)
{
    bufferpos = 0;
    server = new QTcpServer(this);
    socket = NULL;
    server->listen(QHostAddress::Any, AUDIO_PORT);

    connect(server, &QTcpServer::newConnection,
            [=]()
    {
        socket = server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &AudioDataThread::dataRecv);

    });
}
//线程函数
void AudioDataThread::dataRecv()
{
    qint64 datasize;
    QByteArray array;
    AudioBufFree.acquire();//lock

    QByteArray response = socket->readAll();
    QDataStream stream(&response, QIODevice::ReadOnly);

    stream >> datasize >> array;

    for(int i=0; i<array.size(); i++)
    {
        AudioBuffer[bufferpos] = *(array.data()+i);
        //循环队列，当缓冲数组满时从开始的地方覆盖原数据
        //这是不合理的可能训练样本采集到[本次覆盖数据...很久以前的数据]，
        //考虑等待所有使用任务的线程使用完缓存数据在从头覆盖，或双缓冲
        bufferpos = (bufferpos + 1) % AudioBufSize;
    }
    AudioBufUsed.release();

}
AudioDataThread::~AudioDataThread()
{
    if(socket != NULL)
    {
        socket->disconnectFromHost();
        socket->close();
    }
    server->close();
}
