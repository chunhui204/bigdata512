#include "framewidget.h"

FrameWidget::FrameWidget(QWidget *parent) : QWidget(parent)
{

}
void FrameWidget::onWidgetPainted(const QImage &image)
{
    m_image = image;
    repaint();
}

void FrameWidget::paintEvent(QPaintEvent *event)
{
    if(m_image.byteCount() > 0)
    {
        QPainter painter(this);
        m_image = m_image.mirrored(true, true);
        painter.drawImage(this->rect(), m_image, m_image.rect());
        m_image = QImage();
    }
}
