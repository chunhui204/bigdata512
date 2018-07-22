#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QHostAddress>

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    //初始化套接字
    tcpSocket = new QTcpSocket(this);
    audioBase = new AudioBase(this);

    connect(tcpSocket, &QTcpSocket::connected, this, &MainWidget::dealConnection);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &MainWidget::dealResponseFromServer);
    connect(audioBase, &AudioBase::dataReadyEvent, this, &MainWidget::dealAudioData);
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::on_button_connect_clicked()
{
    //连接服务器
    QString ip = ui->lineEdit_ip->text();
    quint16 port = ui->lineEdit_port->text().toInt();

    if(ip.isEmpty() == false && port > 0)
        tcpSocket->connectToHost(QHostAddress(ip), port);
}
void MainWidget::dealConnection()
{
    ui->textEdit->setText("连接成功!!!!");
    //向服务器发送设备信息（麦克风和摄像头）
    QByteArray audioInfo = audioBase->audioInfoToTcp();
    //QString videoInfo = videoBase->getVideoInfo();
    //QString info = audioInfo + videoInfo;

    tcpSocket->write(audioInfo);
}
void MainWidget::dealAudioData(const QByteArray* buffer, qint64 startPos, qint64 endPos)
{
    /*
     * 消息流：
     * audioData _datasize _data
     */
    QByteArray array;
    QByteArray buffer_t(buffer->data()+startPos, endPos-startPos);
    QDataStream stream(&array, QIODevice::WriteOnly);

    stream << QString("audioData") << endPos - startPos << buffer_t;
    tcpSocket->write(array);
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
void MainWidget::dealResponseFromServer()
{
    QByteArray response = tcpSocket->readAll();
    QDataStream stream(&response, QIODevice::ReadOnly);
    QString head;

    stream >> head;

    if("resetAudioFormat" == head)
    {
        //更新audio配置

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



void MainWidget::on_button_quit_clicked()
{
    tcpSocket->disconnectFromHost();
    tcpSocket->close();
    close();
}
