#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QCameraInfo>
#include <QHostAddress>

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    videoSocket = new QTcpSocket(this);


    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    foreach (const QCameraInfo &cameraInfo, cameras) {
        cout << cameraInfo.deviceName();
    }
    m_cam = new QCamera(QCameraInfo::defaultCamera());

    videoSurface = new VideoSurface(this);
    m_cam->setViewfinder(videoSurface);

//    connect(videoSurface, &VideoSurface::framePresented, ui->widget, &VideoWidget::onFramePresented);
    connect(videoSurface, &VideoSurface::framePresented, this, &MainWidget::onFramePresented);
    m_cam->start();

}

MainWidget::~MainWidget()
{
    delete ui;
    if(videoSocket->isValid())
    {
        videoSocket->disconnectFromHost();
        videoSocket->close();
    }
}
void MainWidget::onFramePresented(const QImage &image)
{
    if(videoSocket->isValid())
    {
        QByteArray byte;
        QBuffer buf(&byte);

        image.save(&buf,"JPEG");
        QByteArray ss=qCompress(byte,1);
        QByteArray vv=ss.toBase64();

        QByteArray ba;
        QDataStream out(&ba,QIODevice::WriteOnly);

        //三个int变量，分别是图像宽高，尺寸
        out << int(0) << vv;
        out.device()->seek(0);
        out<< vv.size();

        cout << vv;
        cout << vv.size() << ba.size();
        videoSocket->write(ba);

    }
}

void MainWidget::on_pushButton_clicked()
{
    videoSocket->connectToHost(QHostAddress(QString("127.0.0.1")), 8888);

    connect(videoSocket, &QTcpSocket::connected,
            [=]()
    {
//        QByteArray arr;
//        QDataStream stream(&arr, QIODevice::WriteOnly);
//        stream <<"str";
//        videoSocket->write(arr);
    });
}
