#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHostAddress>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    videoThread = new QThread(this);
    video_thread = new VideoDataThread;
    video_thread->moveToThread(videoThread);

    videoThread->start();
    timer = new QTimer(this);
    timer->start(30);

    cout << QThread::currentThreadId();

    connect(this, &MainWindow::framePresented, ui->videoWidget, &VideoWidget::onFramePresented);
    connect(timer, &QTimer::timeout,
            [=]()
    {
        if(VideoBufUsed.available() > 0)
        {
            VideoBufUsed.acquire();
            QImage image = VideoBuffer[0];
            VideoBufFree.release();
            emit framePresented(image.copy());
        }
    });

}

MainWindow::~MainWindow()
{
    delete ui;
    if(timer->isActive())
        timer->stop();
    delete timer;
}
