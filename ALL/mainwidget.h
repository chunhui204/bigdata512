#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QStatusBar>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include "audiowidget.h"
#include "setting.h"
#include <QThread>
#include "audiodatathread.h"
#include "audioplotthread.h"
#include "videodatathread.h"
#include "spectrumthread.h"
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include "videowidget.h"
#include "videoplotthread.h"
#include <QStackedWidget>

//widget在stackedwidget中的索引
const int AUDIO_WIDGET_INDEX = 0;
const int VIDEO_WIDGET_INDEX =1;

namespace Ui {
class MainWidget;
}

class MainWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

private slots:
    void dealResponseFromClient();
    void onAudioFormatChanged(const AudioSettingFormat &);
    void onCommandIssued(const QByteArray &);

signals:
    void audioFormatInit(const QString &rate, const QString &chns, const QString &size);
    void savePathChanged(const QString & dir);
    void wavFileOpened(const QString &name);
private:
    //ui
    Ui::MainWidget *ui;
    QLabel *labelConnection;
    Setting *setting;
    AudioWidget *audioWidget;
    VideoWidget *videoWidget;

    //
    QString saveRootPath;
    QString saveAudioPath;
    QString saveVideoPath;

    //网络
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;
    //线程
    QThread *audioThread, *videoThread, *aPlotThread, *vPlotThread, *aSptrmThread;
    AudioDataThread *adata_thread;//recv audio data
    AudioPlotThread *aplot_thread;//plot audio
    VideoDataThread *vdata_thread;//recv vedio data
    VideoPlotThread *vplot_thread;//plot video
    SpectrumThread *asptrm_thread;//audio pectrum

    //设备参数
    QList<AudioSettingFormat> audioformats;

private:
    void designMenu(void);
    void connectUI();
    void initThread();
//private signals:
};

#endif // MAINWIDGET_H
