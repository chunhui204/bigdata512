#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QImage>
#include <QDir>
#include "framewidget.h"
#include "common.h"
#include <QDateTime>

namespace Ui {
class VideoWidget;
}

class VideoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoWidget(QWidget *parent = 0);
    ~VideoWidget();

signals:
    void commandIssued(const QByteArray &command);
    void widgetPainted(const QImage &);
    void imageSaved(const QString & name);

public slots:
    void onFramePresented(const QImage &image);
    void onSavePathChanged(const QString & name);
private slots:
    void on_button_video_startcap_clicked();

    void on_button_video_endcap_clicked();

    void on_button_video_saveimg_clicked();


private:
    Ui::VideoWidget *ui;
    QString saveDir;
    QDir qDir;
};

#endif // VIDEOWIDGET_H
