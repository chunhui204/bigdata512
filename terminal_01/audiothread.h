#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>

class AudioThread : public QObject
{
    Q_OBJECT
public:
    explicit AudioThread(QObject *parent = 0);
    ~AudioThread();

public:
    void connectHost(QString ip, int port);

public slots:
    void onDataReadyEvent(const QByteArray &, qint64 ,qint64);
private:
    QTcpSocket *audioSocket;
};

#endif // AUDIOTHREAD_H
