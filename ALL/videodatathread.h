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
#include "common.h"

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
//    QTime time;
};

#endif // VIDEODATATHREAD_H
