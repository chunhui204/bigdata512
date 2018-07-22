#ifndef NETWORKTHREAD_H
#define NETWORKTHREAD_H

#include <QThread>

class NetworkThread : public QThread
{
    Q_OBJECT
public:
    explicit NetworkThread(QThread *parent = 0);
    ~NetworkThread();

protected:
    void run();
signals:

public slots:
};

#endif // NETWORKTHREAD_H
