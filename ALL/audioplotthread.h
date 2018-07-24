#ifndef AUDIOPLOTTHREAD_H
#define AUDIOPLOTTHREAD_H

#include <QObject>
#include "common.h"
#include <QTimer>

const int RefreshTime = 50; //ms
class AudioPlotThread : public QObject
{
    Q_OBJECT
public:
    explicit AudioPlotThread(QObject *parent = 0);
    ~AudioPlotThread();
    void dataTranslation();
public slots:
    void onAudioFormatInit(const QString &rate, const QString &chns, const QString &size);

signals:
    void dataProcessed(const QVector<double> &xs, const QVector<double> &ys);

private:
    int bufferpos;
    QTimer *timer;
    //坐标
    QVector<double> xs;
    QVector<double> ys;
    AudioSettingFormat audioFormat;


};

#endif // AUDIOPLOTTHREAD_H
