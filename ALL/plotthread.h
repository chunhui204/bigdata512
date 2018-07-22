#ifndef PLOTTHREAD_H
#define PLOTTHREAD_H

#include <QThread>

class PlotThread : public QThread
{
    Q_OBJECT
public:
    explicit PlotThread(QThread *parent = 0);
    ~PlotThread();

protected:
    void run();
signals:

public slots:
};

#endif // PLOTTHREAD_H
