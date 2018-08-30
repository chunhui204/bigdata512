#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QHostAddress>
#include "common.h"

QSemaphore AudioBufFree(AudioBufSize);
QSemaphore AudioBufUsed;
QSemaphore BufFree_spectrum(AudioBufSize);
QSemaphore BufUsed_spectrum;
char AudioBuffer[AudioBufSize] = {0};

QSemaphore VideoBufUsed;
QSemaphore VideoBufFree(VideoBufferSize);
QImage VideoBuffer[VideoBufferSize];

MainWidget::MainWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWidget)

{
    ui->setupUi(this);
    /*线程初始化*/
    initThread();
    qcout << "main thread: " << QThread::currentThreadId();

    //初始化UI
    setting = new Setting(this);
    audioWidget = new AudioWidget(this);
    videoWidget = new VideoWidget(this);

    ui->stackedWidget->insertWidget(AUDIO_WIDGET_INDEX, audioWidget);
    ui->stackedWidget->insertWidget(VIDEO_WIDGET_INDEX, videoWidget);
    ui->stackedWidget->setCurrentIndex(AUDIO_WIDGET_INDEX);
    setCentralWidget(ui->stackedWidget);

    //设置窗口菜单栏，标题，图标等
    designMenu();
    setWindowTitle(tr("CVPR"));

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
    videoThread->start();
//    vPlotThread->start();
    aSptrmThread->start();
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
    aPlotThread->quit();
    videoThread->quit();
    vPlotThread->quit();
    aSptrmThread->quit();

    audioThread->wait();
    videoThread->wait();
    aPlotThread->wait();
    vPlotThread->wait();
    aSptrmThread->wait();
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
        QString name = QFileDialog::getOpenFileName(this, tr("请选择wav文件"), QDir::currentPath(),
                                                    tr("AUDIO files (*.wav);; VIDEO files (*.avi)"));
//        cout << "name " <<name;
        if(tcpSocket != NULL)
        {
            //关闭声音采集
            QByteArray array;
            QDataStream stream(&array, QIODevice::WriteOnly);
            stream << QString("stopAudio");
            tcpSocket->write(array);
        }
        //显示曲线
        if(name.isEmpty() == false)
            emit wavFileOpened(name);
    });
    //菜单栏，”文件“下”路径“
    connect(actionPath, &QAction::triggered,
            [=]()
    {
        //设置保存根路径，则音频文件或视频文件分别保存在下面的audio_database和video_database
        QString dir = QFileDialog::getExistingDirectory(this, tr("请设置数据保存目录"), QDir::currentPath(),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        emit savePathChanged(dir);
    });

    QAction *actionAudioFunc = menufunction->addAction(tr("音频"));
    QAction *actionVideoFunc = menufunction->addAction(tr("视频"));

    connect(actionAudioFunc, &QAction::triggered, this,
            [=]()
    {
        //跳转到音频页面
        ui->stackedWidget->setCurrentIndex(AUDIO_WIDGET_INDEX);
    });
    connect(actionVideoFunc, &QAction::triggered, this,
            [=]()
    {
        //跳转到视频页面
        ui->stackedWidget->setCurrentIndex(VIDEO_WIDGET_INDEX);
    });
    menuDevice->addAction("设备");
    connect(menuDevice, &QMenu::triggered,
            [=]()
    {
        //音频设备设置对话框
        setting->designWidget(audioformats);
        setting->show();
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
    connect(this, &MainWidget::audioFormatInit, asptrm_thread, &SpectrumThread::onAudioFormatInit);
    //当对话框设置改变时通知终端进行重新配置
    connect(setting, &Setting::audioFormatChanged, this, &MainWidget::onAudioFormatChanged);
    //widget通过command metwork进行命令传递
    connect(audioWidget, &AudioWidget::commandIssued, this, &MainWidget::onCommandIssued);
    connect(videoWidget, &VideoWidget::commandIssued, this, &MainWidget::onCommandIssued);
    //音频数据后台处理线程通知绘图部件
    qRegisterMetaType<QVector<double>>("QVector<double>");
    connect(aplot_thread, &AudioPlotThread::dataProcessed, audioWidget, &AudioWidget::onDataProcessed);
    connect(audioWidget, &AudioWidget::recordStart, aplot_thread, &AudioPlotThread::onRecordStart);
    connect(audioWidget, &AudioWidget::recordStop, aplot_thread, &AudioPlotThread::onRecordStop);
    connect(this, &MainWidget::wavFileOpened, aplot_thread, &AudioPlotThread::onWavFileOpened);

    connect(asptrm_thread, &SpectrumThread::labelCalculated, audioWidget, &AudioWidget::onLabelCalculated);
    qRegisterMetaType<QVector<int>>("QVector<int>");
    qRegisterMetaType<QVector<QString>>("QVector<QString>");
    connect(audioWidget, &AudioWidget::trainModel, asptrm_thread, &SpectrumThread::train_from_file);
    qRegisterMetaType<Tensor>("Tensor");
    connect(asptrm_thread, &SpectrumThread::alreadyTrained, audioWidget, &AudioWidget::onAlreadyTrained);
    // 更新保存路径后通知需要保存文件的类
    connect(this, &MainWidget::savePathChanged, audioWidget, &AudioWidget::onSavePathChanged);
    connect(this, &MainWidget::savePathChanged, videoWidget, &VideoWidget::onSavePathChanged);
    connect(videoWidget, &VideoWidget::imageSaved, vplot_thread, &VideoPlotThread::onImageSaved);
    //摄像头显示
    connect(vplot_thread, &VideoPlotThread::framePresented, videoWidget, &VideoWidget::onFramePresented);

}
void MainWidget::initThread()
{
    audioThread = new QThread(this);
    videoThread = new QThread(this);
    aPlotThread = new QThread(this);
    vPlotThread = new QThread(this);
    aSptrmThread = new QThread(this);
    adata_thread = new AudioDataThread;
    aplot_thread = new AudioPlotThread;
    vdata_thread = new VideoDataThread;
    vplot_thread = new VideoPlotThread;
    asptrm_thread = new SpectrumThread;

    adata_thread->moveToThread(audioThread);
    aplot_thread->moveToThread(aPlotThread);
    vdata_thread->moveToThread(videoThread);
    vplot_thread->moveToThread(vPlotThread);
    asptrm_thread->moveToThread(aSptrmThread);

    connect(audioThread, &QThread::finished, adata_thread, &QObject::deleteLater);
    connect(aPlotThread, &QThread::finished, aplot_thread, &QObject::deleteLater);
    connect(videoThread, &QThread::finished, vdata_thread, &QObject::deleteLater);
    connect(vPlotThread, &QThread::finished, vplot_thread, &QObject::deleteLater);
    connect(aSptrmThread, &QThread::finished, asptrm_thread, &QObject::deleteLater);
}

//通知终端重置麦克风配置
void MainWidget::onAudioFormatChanged(const AudioSettingFormat &format)
{
    if(tcpSocket != NULL)
    {
        QByteArray array;
        QDataStream stream(&array, QIODevice::WriteOnly);
        stream << QString("resetAudioFormat") << format.deviceName << format.sampleRates[0]
               << format.channel << format.sampleSizes[0] << format.codec;
        tcpSocket->write(array);
    }
}

//处理命令网络接收数据
void MainWidget::dealResponseFromClient()
{
    QByteArray response = tcpSocket->readAll();
    AudioSettingFormat used_format;

    audioformats.clear();
    QDataStream stream(&response, QIODevice::ReadOnly);

    QString head;
    //正在使用的设备信息
    used_format.deviceName = "";
    used_format.codec = "";
    stream >> used_format.sampleRates >> used_format.channel >> used_format.sampleSizes;
    //可选的设备信息
    stream >> head;

    while(head == "audioDevice")//(stream.atEnd() == false)
    {
        AudioSettingFormat format;
        stream >> format.deviceName >> format.sampleRates >> format.channel
                >> format.sampleSizes >> format.codec;
        stream >> head;

        audioformats.append(format);
    }
    if(head == "videoDevice")
    {
        while(stream.atEnd() == false)
        {
            stream >> head;
//            cout <<  head;
        }
    }

    emit audioFormatInit(used_format.sampleRates, used_format.channel, used_format.sampleSizes);
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
void MainWidget::onCommandIssued(const QByteArray &command)
{
    if(tcpSocket == NULL)
        return;

    tcpSocket->write(command);
}
