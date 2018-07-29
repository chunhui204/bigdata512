#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHostAddress>
#include <QTime>

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

    time = QTime::currentTime();
    time.start();
    cout << QThread::currentThreadId();

    connect(this, &MainWindow::framePresented, ui->videoWidget, &VideoWidget::onFramePresented);
    connect(timer, &QTimer::timeout, this, &MainWindow::timeout);

}

MainWindow::~MainWindow()
{
    delete ui;
    videoThread->quit();
    videoThread->wait();
    delete video_thread;

    if(timer->isActive())
        timer->stop();
    delete timer;
}
void MainWindow::timeout()
{
    static double now = time.elapsed() / 1000.0;
    static double last = 0;

//    cout << now-last <<VideoBufUsed.available();
    last = now;
    now = time.elapsed() / 1000.0;

     if(VideoBufUsed.available() > 0)
     {
         VideoBufUsed.acquire();
         QImage image = VideoBuffer[0];
         VideoBufFree.release();

         emit framePresented(image.copy());
     }
}
