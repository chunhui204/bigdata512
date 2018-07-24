#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QHostAddress>
#include <QTime>


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
    connect(commandSocket, &QTcpSocket::disconnected,
            [=]()
            {
                ui->textEdit->setText("断开连接");
                audioBase->stopAudio();
            });
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


    func();
}
void MainWidget::sendAudioData(const QByteArray* buffer, qint64 startPos, qint64 endPos)
{
    /*
     * 消息流：
     * audioData _datasize _data
     */
    cout<<"------------"<<endPos-startPos<<"------------"<<buffer->size();
    if(endPos < startPos)
    {
        return;
    }
    char ch[endPos-startPos];

    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    for(int i=0;i<endPos-startPos;i++)
    {
        ch[i] = qrand()%256;
    }
    QByteArray array;
    QByteArray buffer_t(ch, endPos-startPos);
//    QByteArray buffer_t(buffer->data()+startPos, endPos-startPos);
    QDataStream stream(&array, QIODevice::WriteOnly);


    stream << endPos - startPos << buffer_t;
    int ret = audioSocket->write(array);
    cout<<"------"<<ret;
    QThread::sleep(1);

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
void MainWidget::func()
{
    qint64 startPos = 0;
    qint64 endPos = 44100;

    qint64 sum= 0;

    if(endPos < startPos)
    {
        return;
    }
    char ch[endPos-startPos];

    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    for(int i=0;i<endPos-startPos;i++)
    {
        ch[i] = 120;//qrand()%256;
        cout << "[] " << ch[i];
        sum += ch[i];
    }
    QByteArray array;
    QByteArray buffer_t(ch, endPos-startPos);
//    QByteArray buffer_t(buffer->data()+startPos, endPos-startPos);
    QDataStream stream(&array, QIODevice::WriteOnly);


    stream << qint64(0) << buffer_t;
    stream.device()->seek(0);
    stream << endPos - startPos;
    int ret = audioSocket->write(array);
    cout<<"------"<<sum;
}
