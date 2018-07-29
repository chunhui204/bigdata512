#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QImage>
#include <QTime>
#include <QDebug>

#define cout qDebug()<<__FILE__<<__LINE__

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = 0);

signals:

public slots:
    void onFramePresented(const QImage &image);
protected:
    void paintEvent(QPaintEvent *event);
private:
    QImage m_image;
};

#endif // VIDEOWIDGET_H
