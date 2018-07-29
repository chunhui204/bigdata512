#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QCamera>
#include <QCameraImageCapture>
#include "videosurface.h"
#include "videowidget.h"
#include <QThread>
#include "videothread.h"

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

private:
    Ui::MainWidget *ui;
    QCamera *m_cam;
    VideoSurface *videoSurface;
    VideoThread *video_thread;
    QThread *videoThread;

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
};

#endif // MAINWIDGET_H
