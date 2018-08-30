#include "videodatathread.h"
#include <QThread>
#include <QTime>

VideoDataThread::VideoDataThread(QObject *parent) : QObject(parent)
{
    bufferpos = 0;
    totalsize = 0;
    image_array.clear();

//    time = QTime::currentTime();
//    time.start();
    videoSocket = NULL;
    videoServer = new QTcpServer(this);
    videoServer->listen(QHostAddress::Any, VIDEO_PORT);

    connect(videoServer, &QTcpServer::newConnection,
            [=]()
            {
                videoSocket = videoServer->nextPendingConnection();
                connect(videoSocket, &QTcpSocket::readyRead, this, &VideoDataThread::dataRecv);
            });


}
void VideoDataThread::dataRecv()
{

    if(image_array.isNull())
    {
        QByteArray array = videoSocket->read(sizeof(int));
        QDataStream stream(&array, QIODevice::ReadOnly);
        stream >> totalsize;
        videoSocket->read(4);

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
//    cout << array.size();
    QByteArray unc = qUncompress(array);
    QImage image;
    image.loadFromData(unc, "JPEG");

    VideoBufFree.acquire();

    VideoBuffer[bufferpos] = image;
    bufferpos = (bufferpos + 1) % VideoBufferSize;
    VideoBufUsed.release();
}
VideoDataThread::~VideoDataThread()
{
    if(videoSocket != NULL)
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
