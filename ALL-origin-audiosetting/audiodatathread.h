#ifndef AUDIODATATHREAD_H
#define AUDIODATATHREAD_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include "common.h"
#include <QDataStream>

class AudioDataThread : public QObject
{
    Q_OBJECT
public:
    explicit AudioDataThread(QObject *parent = 0);
    ~AudioDataThread();
    void dataRecv();

signals:

public slots:
private:
    QTcpServer *server;
    QTcpSocket *socket;
    //当前访问缓存数组的位置
    int bufferpos;
    //分布读取
    qint64 readBytes;
    qint64 totalSize;
};

#endif // AUDIODATATHREAD_H
