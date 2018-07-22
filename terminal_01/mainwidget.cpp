#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QHostAddress>

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    ui->lineEdit_ip->setText("127.0.0.1");
    //初始化套接字
    commandSocket = new QTcpSocket(this);
    audioSocket = new QTcpSocket(this);
    audioBase = new AudioBase(this);

    connect(commandSocket, &QTcpSocket::connected, this, &MainWidget::dealConnection);
    connect(commandSocket, &QTcpSocket::readyRead, this, &MainWidget::dealCommandResponse);
    connect(audioBase, &AudioBase::dataReadyEvent, this, &MainWidget::sendAudioData);

}

MainWidget::~MainWidget()
{
    delete ui;
    commandSocket->disconnectFromHost();
    commandSocket->close();
    audioSocket->disconnectFromHost();
    audioSocket->close();
}

void MainWidget::on_button_connect_clicked()
{
    //连接服务器
    QString ip = ui->lineEdit_ip->text();

    if(ip.isEmpty() == false)
    {
        commandSocket->connectToHost(QHostAddress(ip), COMMAND_PORT);
        audioSocket->connectToHost(QHostAddress(ip), AUDIO_PORT);
    }
}
void MainWidget::dealConnection()
{
    ui->textEdit->setText("连接成功!!!!");
    //向服务器发送设备信息（麦克风和摄像头）
    QByteArray audioInfo = audioBase->audioInfoToTcp();
    //QString videoInfo = videoBase->getVideoInfo();
    //QString info = audioInfo + videoInfo;

    commandSocket->write(audioInfo);
}
void MainWidget::sendAudioData(const QByteArray* buffer, qint64 startPos, qint64 endPos)
{
    /*
     * 消息流：
     * audioData _datasize _data
     */
    QByteArray array;
    QByteArray buffer_t(buffer->data()+startPos, endPos-startPos);
    QDataStream stream(&array, QIODevice::WriteOnly);

    stream << QString("audioData") << endPos - startPos << buffer_t;
    audioSocket->write(array);
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
    cout<<stream;

    if("resetAudioFormat" == head)
    {
        //更新audio配置
        ui->textEdit->append("更新配置成功");
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

}

