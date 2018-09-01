#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QHostAddress>
#include <QTime>
#include <QMessageBox>

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    ui->lineEdit_ip->setText("127.0.0.1");
    //thread
    videoThread = new QThread(this);
    video_thread = new VideoThread;
    video_thread->moveToThread(videoThread);

    audioThread = new QThread(this);
    audio_thread = new AudioThread;
    audio_thread->moveToThread(audioThread);

    connect(videoThread, &QThread::finished, video_thread, &VideoThread::deleteLater);
    connect(audioThread, &QThread::finished, audio_thread, &AudioThread::deleteLater);

    //初始化套接字
    commandSocket = new QTcpSocket(this);

    audioBase = new AudioBase(this);
    videoBase = new VideoBase(this);

    connect(audioBase, &AudioBase::audioError, this, &MainWidget::onAudioError);
    connect(commandSocket, &QTcpSocket::connected, this, &MainWidget::dealConnection);
    connect(commandSocket, &QTcpSocket::readyRead, this, &MainWidget::dealCommandResponse);
    connect(audioBase, &AudioBase::dataReadyEvent, this, &MainWidget::sendAudioData);
    connect(this, &MainWidget::audioFormatChanged, audioBase, &AudioBase::onAudioFormatChanged);

    connect(commandSocket, &QTcpSocket::disconnected,
            [=]()
            {
                ui->textEdit->setText("断开连接");
                audioBase->stopAudio();
                videoBase->stopVideo();
            });

    videoThread->start();
}

MainWidget::~MainWidget()
{
    delete ui;
    if(commandSocket->isValid())
    {
        commandSocket->disconnectFromHost();
        commandSocket->close();
    }
    if(audioSocket->isValid())
    {
        audioSocket->disconnectFromHost();
        audioSocket->close();
    }
    if(videoThread->isRunning())
    {
        videoThread->quit();
        videoThread->wait();
    }
}

void MainWidget::on_button_connect_clicked()
{
    //连接服务器
    QString ip = ui->lineEdit_ip->text();

    if(ip.isEmpty() == false)
    {
        commandSocket->connectToHost(QHostAddress(ip), COMMAND_PORT);
        audioSocket->connectToHost(QHostAddress(ip), AUDIO_PORT);
        video_thread->connectHost(ip, VIDEO_PORT);
    }
}
void MainWidget::dealConnection()
{
    ui->textEdit->setText("连接成功!!!!");
    //向服务器发送设备信息（麦克风和摄像头）
    QByteArray audioInfo = audioBase->audioInfoToTcp();
    audioInfo.append(videoBase->videoInfoToTcp());

    commandSocket->write(audioInfo);
}

//解析服务器响应
/*
 * resetAudioFormat : 更新audio配置
 * resetVideoFormat : 更新视频配置
 * readyAudioData: 确认接受音频数据
 * readyVideoData: 确认接受视频数据
 * startAudio
 * startVideo
 * stopAudio
 * stopVideo
 */
void MainWidget::dealCommandResponse()
{
    QByteArray response = commandSocket->readAll();
    QDataStream stream(&response, QIODevice::ReadOnly);
    QString head;

    stream >> head;

    if("resetAudioFormat" == head)
    {
        //更新audio配置
        ui->textEdit->append("更新配置成功");

        QString device, rate, chn, byte, codec;
        stream >> device >>rate >> chn >> byte >> codec;
        emit audioFormatChanged(device, rate, chn, byte, codec);
    }
    else if("resetVideoFormat" == head)
    {
        //更新video配置
    }
    else if("startAudio" == head)
    {
        audioBase->startAudio();
    }
    else if("stopAudio" == head)
    {
        audioBase->stopAudio();
    }
    else if("startVideo" == head)
    {
        videoBase->startVideo();
    }
    else if("stopVideo" == head)
    {
        videoBase->stopVideo();
    }

}
void MainWidget::onAudioError(QString str)
{
    ui->textEdit->append(str);
}

