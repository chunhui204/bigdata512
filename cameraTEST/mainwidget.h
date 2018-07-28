#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QCamera>
#include <QCameraImageCapture>
#include "videosurface.h"
#include "videowidget.h"
#include <QTcpSocket>
#include <QBuffer>

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
    QTcpSocket *videoSocket;

private slots:
    void onFramePresented(const QImage &);
};

#endif // MAINWIDGET_H
