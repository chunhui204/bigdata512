#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>
#include "videowidget.h"
#include "videodatathread.h"
#include <QThread>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void framePresented(const QImage &);
private:
    Ui::MainWindow *ui;
    QTimer *timer;
    VideoDataThread *video_thread;
    QThread *videoThread;
};

#endif // MAINWINDOW_H
