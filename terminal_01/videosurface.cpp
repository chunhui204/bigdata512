#include "videosurface.h"
#include <QTime>

VideoSurface::VideoSurface(QObject *parent) : QAbstractVideoSurface(parent)
{

}

QList<QVideoFrame::PixelFormat> VideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const
{
//    Q_UNUSED(handleType);
        QList<QVideoFrame::PixelFormat> list;
        list << QVideoFrame::Format_RGB32;
        list << QVideoFrame::Format_ARGB32;
        list << QVideoFrame::Format_RGB24;
        list << QVideoFrame::Format_UYVY;
        list << QVideoFrame::Format_Y8;
        list << QVideoFrame::Format_YUYV;
        return list;
}

bool VideoSurface::present(const QVideoFrame &frame)
{
    static double last = 0;
    static QTime time = QTime::currentTime() ;
    double now = time.elapsed() / 1000.0;
    static int cnt = 0;

    cnt ++;
    if(now - last > 1)
    {
        last = now;
//        cout << "frame: "<< cnt;
        cnt = 0;
    }
    QVideoFrame frame_copy(frame);
    //已经映射到内存中了
    if(!frame_copy.isMapped())
        frame_copy.map(QAbstractVideoBuffer::ReadOnly);

        QImage image(
                    frame.bits(),
                    frame.width(),
                    frame.height(),
                    frame.bytesPerLine(),
                    QImage::Format_RGB32);
        frame_copy.unmap();
        emit framePresented(image.copy());

        return true;
}
