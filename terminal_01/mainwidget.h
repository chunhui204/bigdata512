#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include "audiobase.h"

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

private slots:
    void dealConnection();
    void dealResponseFromServer();
    void dealAudioData(const QByteArray*, qint64, qint64);
    void on_button_connect_clicked();

    void on_button_quit_clicked();

private:
    Ui::MainWidget *ui;
    QTcpSocket *tcpSocket;
    AudioBase *audioBase;

};

#endif // MAINWIDGET_H
