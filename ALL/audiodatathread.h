#ifndef AUDIODATATHREAD_H
#define AUDIODATATHREAD_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include "common.h"

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

};

#endif // AUDIODATATHREAD_H
