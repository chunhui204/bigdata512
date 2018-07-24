#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include "audiobase.h"
#include <QThread>

const int COMMAND_PORT = 8888;
const int AUDIO_PORT = 8889;
const int VIDEO_PORT = 8890;

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();
    void func();

private slots:
    void dealConnection();
    void dealCommandResponse();
    void sendAudioData(const QByteArray*, qint64, qint64);
    void on_button_connect_clicked();


private:
    Ui::MainWidget *ui;
    QTcpSocket *commandSocket, *audioSocket;
    AudioBase *audioBase;

};

#endif // MAINWIDGET_H
