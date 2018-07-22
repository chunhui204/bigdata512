#ifndef AUDIOWIDGET_H
#define AUDIOWIDGET_H

#include <QWidget>

namespace Ui {
class AudioWidget;
}

class AudioWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AudioWidget(QWidget *parent = 0);
    ~AudioWidget();

private:
    Ui::AudioWidget *ui;
public slots:
    void drawAudioCurve(const QByteArray &);
};

#endif // AUDIOWIDGET_H
