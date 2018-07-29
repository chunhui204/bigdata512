#include "videothread.h"

VideoThread::VideoThread(QObject *parent) : QObject(parent)
{
    time=QTime::currentTime();
    time.start();

    videoSocket = new QTcpSocket(this);
    videoSocket->connectToHost(QHostAddress(QString("127.0.0.1")), 8888);
}
VideoThread::~VideoThread()
{
    videoSocket->disconnectFromHost();
    videoSocket->close();
}

void VideoThread::onFramePresented(const QImage &image)
{
    static double now=0;
    static double last=0;
    if(videoSocket->isValid())
    {
        cout << now-last;
        last = now;
        now = time.elapsed() / 1000.0;

        QByteArray byte;
        QBuffer buf(&byte);

        image.save(&buf,"JPEG");
        QByteArray ss=qCompress(byte,1);
//        QByteArray vv=ss.toBase64();

//        QByteArray ss=qCompress(QString("abcd").toUtf8(),1);
        QByteArray ba;
        QDataStream out(&ba,QIODevice::WriteOnly);

        //三个int变量，分别是图像宽高，尺寸
        out << int(0) << ss;
        out.device()->seek(0);
        out<< ss.size();

//        cout << ss;
//        cout << byte.size() << ss.size() << vv.size() << ba.size();
        videoSocket->write(ba);

    }
}
