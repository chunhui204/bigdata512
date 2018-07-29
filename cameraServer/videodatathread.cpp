#include "videodatathread.h"
#include <QThread>

QList<QImage> VideoBuffer;
const int VideoBufferSize = 10;
QSemaphore VideoBufUsed(0);
QSemaphore VideoBufFree(VideoBufferSize);

VideoDataThread::VideoDataThread(QObject *parent) : QObject(parent)
{
    bufferpos = 0;
    totalsize = 0;
    image_array.clear();

    videoSocket = NULL;
    videoServer = new QTcpServer(this);
    videoServer->listen(QHostAddress::Any, 8888);

    connect(videoServer, &QTcpServer::newConnection,
            [=]()
            {
                videoSocket = videoServer->nextPendingConnection();
                cout << videoSocket->peerAddress().toString()<<videoSocket->peerPort();
                cout << QThread::currentThreadId();
                connect(videoSocket, &QTcpSocket::readyRead,
                        [=]()
                {
                    cout <<"data ready";
                });
                connect(videoSocket, &QTcpSocket::readyRead, this, &VideoDataThread::dataRecv);
            });


}
void VideoDataThread::dataRecv()
{
    cout <<"data ready";
    if(image_array.isNull())
    {
        QByteArray array = videoSocket->read(sizeof(int));
        QDataStream stream(&array, QIODevice::ReadOnly);
        stream >> totalsize;

        image_array = QByteArray(videoSocket->read(totalsize));
        if(image_array.size() == totalsize)
        {
            arrayToImage(image_array);
            image_array.clear();
        }
    }
    else if(image_array.size() < totalsize)
    {
        QByteArray array = videoSocket->read(totalsize - image_array.size());
        image_array.append(array);

        if(image_array.size() == totalsize)
        {
            arrayToImage(image_array);
            image_array.clear();
        }
    }
}
void VideoDataThread::arrayToImage(const QByteArray &array)
{
    QByteArray base64 = QByteArray::fromBase64(array);
    QByteArray unc = qUncompress(base64);
    QImage image;
    image.loadFromData(unc, "JPEG");

    VideoBufFree.acquire();

    if(VideoBuffer.size() == VideoBufferSize)
        VideoBuffer.pop_front();//队列满了，第一个元素出队
    VideoBuffer.append(image);//append添加的是引用
    VideoBufUsed.release();
}
VideoDataThread::~VideoDataThread()
{
    if(videoSocket->isValid())
    {
        videoSocket->disconnectFromHost();
        videoSocket->close();
    }
    if(videoServer->isListening())
    {
        videoServer->disconnect();
        videoServer->close();
    }
}
