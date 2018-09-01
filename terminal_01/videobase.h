#ifndef VIDEOBASE_H
#define VIDEOBASE_H

#include <QObject>
#include <QThread>
#include "videosurface.h"
#include "videothread.h"
#include <QCamera>
#include <QDataStream>
#include <QCameraInfo>

class VideoBase : public QObject
{
    Q_OBJECT
public:
    explicit VideoBase( QObject *parent = 0);
    ~VideoBase();
    QByteArray videoInfoToTcp();
    void startVideo();
    void stopVideo();

signals:

private:
    QCamera *m_cam;
    VideoSurface *videoSurface;
};

#endif // VIDEOBASE_H
