#ifndef AUDIOWIDGET_H
#define AUDIOWIDGET_H

#include <QWidget>
#include <QDir>
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
    //文件保存
    QString saveDir;        //ROOT目录路径
    QDir qDir;

public slots:
    void onDataProcessed(const QVector<double> &xs, const QVector<double> &ys);
    void onSavePathChanged(const QString &dir);
private slots:
    void on_button_audio_capStart_clicked();
    void on_button_audio_capEnd_clicked();
    void on_button_audio_recordStart_clicked();

    void on_button_audio_recordEnd_clicked();

signals:
    void commandIssued(QString command);
    void recordStart(const QString &filename);
    void recordStop();
};

#endif // AUDIOWIDGET_H
