#include "videowidget.h"
#include <QBuffer>

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent)
{
}
void VideoWidget::paintEvent(QPaintEvent *event)
{
        QPainter painter(this);
        m_image = m_image.mirrored(true, true);
        painter.drawImage(this->rect(), m_image, m_image.rect());
}
void VideoWidget::onFramePresented(const QImage &image)
{
    m_image = image;
    repaint();

}
