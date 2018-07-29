#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QCameraInfo>
#include <QHostAddress>

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);


    videoThread = new QThread(this);
    video_thread = new VideoThread;
    video_thread->moveToThread(videoThread);

    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    foreach (const QCameraInfo &cameraInfo, cameras) {
        cout << cameraInfo.deviceName();
    }
    m_cam = new QCamera(QCameraInfo::defaultCamera());

    videoSurface = new VideoSurface(this);
    m_cam->setViewfinder(videoSurface);

    connect(videoSurface, &VideoSurface::framePresented, ui->widget, &VideoWidget::onFramePresented);
    connect(videoSurface, &VideoSurface::framePresented, video_thread, &VideoThread::onFramePresented);
    m_cam->start();

}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::on_pushButton_clicked()
{
    videoThread->start();
}

void MainWidget::on_pushButton_2_clicked()
{
    videoThread->quit();
    videoThread->wait();
}
