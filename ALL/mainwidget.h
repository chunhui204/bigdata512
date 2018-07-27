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
#include "audiosetting.h"
#include <QThread>
#include "audiodatathread.h"
#include "audioplotthread.h"
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>

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
    void onCommandIssued(QString command);

signals:
    void audioFormatInit(const QString &rate, const QString &chns, const QString &size);
    void savePathChanged(const QString & dir);
    void wavFileOpened(const QString &name);
private:
    //ui
    Ui::MainWidget *ui;
    QLabel *labelConnection;
    AudioSetting *audioSetting;
    AudioWidget *audioWidget;

    //
    QString saveRootPath;
    QString saveAudioPath;
    QString saveVideoPath;

    //网络
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;
    //线程
    QThread *audioThread, *videoThread, *aPlotThread, *vPlotThread;
    AudioDataThread *adata_thread;
    AudioPlotThread *aplot_thread;

    //设备参数
    QList<AudioSettingFormat> audioformats;

private:
    void designMenu(void);
    void connectUI();
//private signals:
};

#endif // MAINWIDGET_H
