#ifndef AUDIOPLOTTHREAD_H
#define AUDIOPLOTTHREAD_H

#include <QObject>
#include "common.h"
#include <QTimer>

class AudioPlotThread : public QObject
{
    Q_OBJECT
public:
    explicit AudioPlotThread(QObject *parent = 0);
    ~AudioPlotThread();
    void dataTranslation();
signals:

private:
    int bufferpos;
    QTimer *timer;
};

#endif // AUDIOPLOTTHREAD_H
