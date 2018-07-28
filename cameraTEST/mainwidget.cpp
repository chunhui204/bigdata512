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
    videoSocket->connectToHost(QHostAddress(QString("127.0.0.1")), 8888);

    connect(videoSocket, &QTcpSocket::connected,
            [=]()
    {
        cout << "连接成功了";
    });

    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    foreach (const QCameraInfo &cameraInfo, cameras) {
        cout << cameraInfo.deviceName();
    }
    m_cam = new QCamera(QCameraInfo::defaultCamera());

    videoSurface = new VideoSurface(this);
    m_cam->setViewfinder(videoSurface);

    connect(videoSurface, &VideoSurface::framePresented, ui->widget, &VideoWidget::onFramePresented);
    connect(videoSurface, &VideoSurface::framePresented, this, &MainWidget::onFramePresented);
    m_cam->start();

}

MainWidget::~MainWidget()
{
    delete ui;
    videoSocket->disconnectFromHost();
    videoSocket->close();
}
void MainWidget::onFramePresented(const QImage &image)
{
    QByteArray byte;
    QBuffer buf(&byte);

    image.save(&buf,"JPEG");
    QByteArray ss=qCompress(byte,1);
    QByteArray vv=ss.toBase64();

    QByteArray ba;
    QDataStream out(&ba,QIODevice::WriteOnly);

    //三个int变量，分别是图像宽高，尺寸
    out<< image.width() << image.height() << vv.size() << vv;
    tcpSocket->write(ba);
}
