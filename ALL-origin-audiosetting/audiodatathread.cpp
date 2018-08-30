#include "audiodatathread.h"
#include <QHostAddress>

AudioDataThread::AudioDataThread(QObject *parent) : QObject(parent)
{
    bufferpos = 0;
    readBytes = 0;
    totalSize = 0;
    server = new QTcpServer(this);
    socket = NULL;
    server->listen(QHostAddress::Any, AUDIO_PORT);

    connect(server, &QTcpServer::newConnection,
            [=]()
    {
        socket = server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &AudioDataThread::dataRecv);
//        cout << socket->peerAddress().toString()<<socket->peerPort();
    });
}
//线程函数
/*
 * 由于一次发送的数据量较大，这里做了分步接受，每次接受1000B，直到将终端一次发送的数据接受完为止。
 * 我们把终端一次发送的数据叫做一个完整包
 * 使用readBytes标记上位机在接受一个完整包过程中读到的字节。
 * readBytes=0: 还没读取完整包；0<readByte<totalSize: 正在读取中；readBytes==totalSize: 读取结束
 *
 */
void AudioDataThread::dataRecv()
{
    if(readBytes == 0)
    {//读包的开始部分
        QByteArray headarr = socket->read(sizeof(qint64));
        QDataStream headsm(&headarr, QIODevice::ReadOnly);
        headsm >> totalSize;
        //qtcp 在序列化数组或字符串时先序列化其长度，即前四个字节表示长度，所以这里在实际长度上加4表示数组长度。而且读的时候也要跳过长度信息。
        //当然如果先转datastream再读出array的话就不用考虑这一点。
        totalSize += 4;

        QByteArray array = socket->read(totalSize);
        readBytes += array.size();

        AudioBufFree.acquire(array.size() - 4);//lock when free size == 0
        BufFree_spectrum.acquire(array.size() - 4);
        for(int i=4; i<array.size(); i++)
        {
            AudioBuffer[bufferpos] = *(array.data()+i);
            //循环队列，当缓冲数组满时从开始的地方覆盖原数据
            //这是不合理的可能训练样本采集到[本次覆盖数据...很久以前的数据]，
            //考虑等待所有使用任务的线程使用完缓存数据在从头覆盖，或双缓冲
            bufferpos = (bufferpos + 1) % AudioBufSize;
        }
        AudioBufUsed.release(array.size() - 4);
        BufUsed_spectrum.release(array.size() - 4);
    }
    else if(readBytes < totalSize)
    {

        QByteArray array = socket->read(totalSize - readBytes);
        readBytes += array.size();

        AudioBufFree.acquire(array.size());//lock
        for(int i=0; i<array.size(); i++)
        {
            AudioBuffer[bufferpos] = *(array.data()+i);
            bufferpos = (bufferpos + 1) % AudioBufSize;
        }

        AudioBufUsed.release(array.size());
    }
    if(readBytes == totalSize)
        readBytes = 0;
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
