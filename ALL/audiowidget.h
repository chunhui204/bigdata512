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
    void onDataProcessed(const QVector<double> &xs, const QVector<double> &ys);
private slots:
    void on_button_audio_capStart_clicked();
    void on_button_audio_capEnd_clicked();
signals:
    void commandIssued(QString command);
};

#endif // AUDIOWIDGET_H
