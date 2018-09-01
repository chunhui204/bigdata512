#include "videobase.h"

VideoBase::VideoBase(QObject *parent) : QObject(parent)
{
    m_cam = new QCamera(QCameraInfo::defaultCamera());

    videoSurface = new VideoSurface(this);
    m_cam->setViewfinder(videoSurface);

//    connect(videoSurface, &VideoSurface::framePresented, &thread, &VideoThread::onFramePresented);
//    videoThread->start();
}
VideoBase::~VideoBase()
{
    delete m_cam;
}
QByteArray VideoBase::videoInfoToTcp()
{
    QByteArray arr;
    QDataStream stream(&arr, QIODevice::WriteOnly);

    stream << QString("videoDevice");
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    foreach (const QCameraInfo &cameraInfo, cameras) {
        stream << cameraInfo.deviceName();
    }
    return arr;
}
void VideoBase::startVideo()
{
    m_cam->start();
}
void VideoBase::stopVideo()
{
    m_cam->stop();
}
