#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#include <QObject>
#include <QAbstractVideoSurface>
#include <QVideoFrame>
#include <QImage>
//#include <QDebug>

//#define cout qDebug()<<__FILE__<<__LINE__

class VideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    explicit VideoSurface(QObject *parent = 0);

protected:

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType type = QAbstractVideoBuffer::NoHandle) const;
    bool present(const QVideoFrame &frame);


signals:
    void framePresented(const QImage &frame);
public slots:
};

#endif // VIDEOSURFACE_H
