#ifndef VIDEODATATHREAD_H
#define VIDEODATATHREAD_H

#include <QObject>
#include <QList>
#include <QSemaphore>
#include <QDataStream>
#include <QTcpServer>
#include <QTcpSocket>
#include <QImage>
#include <QDebug>
#include <QTime>

#define cout qDebug()<<__FILE__<<__LINE__


extern QList<QImage> VideoBuffer;
extern const int VideoBufferSize;
extern QSemaphore VideoBufUsed;
extern QSemaphore VideoBufFree;

class VideoDataThread : public QObject
{
    Q_OBJECT
public:
    explicit VideoDataThread(QObject *parent = 0);
    ~VideoDataThread();
private:

    void arrayToImage(const QByteArray &array);

private slots:
    void dataRecv();
private:
    int bufferpos;
    int totalsize;

    QTcpSocket *videoSocket;
    QTcpServer *videoServer;
    QByteArray image_array;
    QTime time;
};

#endif // VIDEODATATHREAD_H
