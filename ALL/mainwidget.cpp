#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QHostAddress>
#include "common.h"

QSemaphore AudioBufFree(AudioBufSize);
QSemaphore AudioBufUsed;
char AudioBuffer[AudioBufSize];

MainWidget::MainWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWidget),
    /*线程初始化*/
    audioThread(new QThread(this)),
    videoThread(new QThread(this)),
    aPlotThread(new QThread(this)),
    vPlotThread(new QThread(this)),
    adata_thread(new AudioDataThread),
    aplot_thread(new AudioPlotThread)
{
    ui->setupUi(this);
    //初始化变量


    adata_thread->moveToThread(audioThread);
    aplot_thread->moveToThread(aPlotThread);

    connect(audioThread, &QThread::finished, adata_thread, &QObject::deleteLater);
    connect(aPlotThread, &QThread::finished, aplot_thread, &QObject::deleteLater);

    //设置窗口菜单栏，标题，图标等
    designMenu();
    setWindowTitle(tr("CVPR"));

    //初始化UI
    audioSetting = new AudioSetting(this);
    audioWidget = new AudioWidget(this);

    setCentralWidget(audioWidget);
    connectUI();
    //初始化网络
    tcpSocket = NULL;
    tcpServer = new QTcpServer(this);
    tcpServer->listen(QHostAddress::Any, COMMAND_PORT);

    connect(tcpServer, &QTcpServer::newConnection,
            [=]()
            {//获取通信socket，并显示ip
                tcpSocket = tcpServer->nextPendingConnection();
                QString ip = tcpSocket->peerAddress().toString();
                quint16 port = tcpSocket->peerPort();
                labelConnection->setText(QString("ip: %1, port: %2").arg(ip).arg(port));

                connect(tcpSocket, &QTcpSocket::readyRead, this, &MainWidget::dealResponseFromClient);
            });



    audioThread->start();
    aPlotThread->start();
}

MainWidget::~MainWidget()
{
    delete ui;
    if(tcpSocket != NULL)
    {
        tcpSocket->disconnectFromHost();
        tcpSocket->close();
    }
    tcpServer->close();

    audioThread->quit();
    audioThread->wait();
    videoThread->quit();
    videoThread->wait();
    aPlotThread->quit();
    aPlotThread->wait();
    vPlotThread->quit();
    vPlotThread->wait();

}

void MainWidget::designMenu(void)
{
    QMenuBar *m_menuBar = menuBar();
    setMenuBar(m_menuBar);

    QMenu *menuFile = m_menuBar->addMenu(tr("文件"));
    QMenu *menufunction = m_menuBar->addMenu(tr("功能"));
    QMenu *menuDevice = m_menuBar->addMenu(tr("设备"));

    QAction *actionOpen = menuFile->addAction(tr("打开文件"));
    QAction *actionPath = menuFile->addAction(tr("保存路径"));

    //菜单栏，”文件“下”打开“
    connect(actionOpen, &QAction::triggered,
            [=]()
    {
        //打开文件，将音频或视频且文件信息显示到音频曲线或视频窗口
    });
    //菜单栏，”文件“下”路径“
    connect(actionOpen, &QAction::triggered,
            [=]()
    {
        //设置保存根路径，则音频文件或视频文件分别保存在下面的audio_database和video_database

    });

    QAction *actionAudioFunc = menufunction->addAction(tr("音频"));
    QAction *actionVideoFunc = menufunction->addAction(tr("视频"));

    connect(actionAudioFunc, &QAction::triggered, this,
            [=]()
    {
        //跳转到音频页面
    });
    connect(actionVideoFunc, &QAction::triggered, this,
            [=]()
    {
        //跳转到视频页面
    });
    QAction *actionAudioDevice = menuDevice->addAction(tr("音频设备"));
    QAction *actionVideoDevice = menuDevice->addAction(tr("视频设备"));
    connect(actionAudioDevice, &QAction::triggered,
            [=]()
    {
        //音频设备设置对话框
        audioSetting->designWidget(audioformats);
        audioSetting->show();
    });
    connect(actionVideoDevice, &QAction::triggered,
            [=]()
    {
       //视频设备设置对话框
    });

    //显示终端连接信息，显示ip，port
    labelConnection = new QLabel(this);
    labelConnection->setText(QString(tr("未连接")));
    ui->statusBar->addWidget(labelConnection);

}
void MainWidget::connectUI()
{
    //当终端发来设备信息时通知绘图线程和设置界面(没通过消息机制，直接调用的)
    connect(this, &MainWidget::audioFormatInit, aplot_thread, &AudioPlotThread::onAudioFormatInit);
    //当对话框设置改变时通知终端进行重新配置
    connect(audioSetting, &AudioSetting::audioFormatChanged, this, &MainWidget::onAudioFormatChanged);
    //widget通过command metwork进行命令传递
    connect(audioWidget, &AudioWidget::commandIssued, this, &MainWidget::onCommandIssued);
    //音频数据后台处理线程通知绘图部件
    qRegisterMetaType<QVector<double>>("QVector<double>");
    connect(aplot_thread, &AudioPlotThread::dataProcessed, audioWidget, &AudioWidget::onDataProcessed);

}
//通知终端重置麦克风配置
void MainWidget::onAudioFormatChanged(const AudioSettingFormat &format)
{
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << "audioDeviceInfo" << format.deviceName << format.sampleRates[0]
           << format.channel << format.sampleSizes[0] << format.codec;
    tcpSocket->write(array);
    //采集按钮可视
    cout<<"重置";
}

//处理命令网络接收数据
void MainWidget::dealResponseFromClient()
{
    QByteArray response = tcpSocket->readAll();
    AudioSettingFormat used_format;

    audioformats.clear();
    QDataStream stream(&response, QIODevice::ReadOnly);

    QString head;
    stream >> head;
    //正在使用的设备信息
    used_format.deviceName = "";
    used_format.codec = "";
    stream >> used_format.sampleRates >> used_format.channel >> used_format.sampleSizes;
    //可选的设备信息
    while(stream.atEnd() == false)
    {
        AudioSettingFormat format;
        stream >> format.deviceName >> format.sampleRates >> format.channel
                >> format.sampleSizes >> format.codec;

        audioformats.append(format);
    }

    emit audioFormatInit(used_format.sampleRates[0], used_format.channel, used_format.sampleSizes[0]);
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
void MainWidget::onCommandIssued(QString command)
{
    if(tcpSocket == NULL)
        return;

    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    if(command=="startAudio")
        stream << QString("startAudio");
    else if(command=="stopAudio")
        stream << QString("stopAudio");

    tcpSocket->write(array);
}
