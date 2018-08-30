#include "videoplotthread.h"

VideoPlotThread::VideoPlotThread(QObject *parent) : QObject(parent)
{
    bufferpos = 0;
    isImageSaved = false;
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &VideoPlotThread::processFrame);

    timer->start(30);
}
VideoPlotThread::~VideoPlotThread()
{
    if(timer->isActive())
        timer->stop();
}

void VideoPlotThread::processFrame()
{
        VideoBufUsed.acquire();
        QImage image = VideoBuffer[bufferpos];
        bufferpos = (1 + bufferpos) % VideoBufferSize;

        if(isImageSaved)
        {
            image.mirrored().save(imageName, "JPEG");
            isImageSaved = false;
        }
        VideoBufFree.release();

        emit framePresented(image.copy());
}
void VideoPlotThread::onImageSaved(const QString &name)
{
    isImageSaved = true;
    imageName = name;
//    cout << "name: "<< name;
}
