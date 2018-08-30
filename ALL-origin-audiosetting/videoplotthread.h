#ifndef VIDEOPLOTTHREAD_H
#define VIDEOPLOTTHREAD_H

#include <QObject>
#include <QTimer>
#include "common.h"

class VideoPlotThread : public QObject
{
    Q_OBJECT
public:
    explicit VideoPlotThread(QObject *parent = 0);
    ~VideoPlotThread();
signals:
    void framePresented(const QImage &);
public slots:
    void onImageSaved(const QString & name);

private slots:
    void processFrame();
private:
    QTimer *timer;
    int bufferpos;
    bool isImageSaved;
    QString imageName;
};

#endif // VIDEOPLOTTHREAD_H
