#ifndef FRAMEWIDGET_H
#define FRAMEWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPainter>

class FrameWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FrameWidget(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event);
public slots:
    void onWidgetPainted(const QImage &image);
private:
    QImage m_image;
};

#endif // FRAMEWIDGET_H
