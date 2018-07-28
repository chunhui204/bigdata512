#include "videowidget.h"

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent)
{
    flag = true;
}
void VideoWidget::paintEvent(QPaintEvent *event)
{
    if(flag == false)
    {
        QPainter painter(this);
        m_image = m_image.mirrored(true, true);
        painter.drawImage(this->rect(), m_image, m_image.rect());
        flag = true;
    }
    else
    {
//        cout << "------------------" << m_image.byteCount();
    }

}
void VideoWidget::onFramePresented(const QImage &image)
{
    m_image = image;
    flag = false;
    repaint();

}
