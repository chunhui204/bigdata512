#ifndef AUDIOWIDGET_H
#define AUDIOWIDGET_H

#include <QWidget>
#include <QDir>
#include "gaussmodel/tensor.h"

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
    void onLabelCalculated(int albel);
    void onAlreadyTrained(const Tensor &m_featMean, const Tensor &m_featStd, const Tensor &m_gate);
private slots:
    void on_button_audio_capStart_clicked();
    void on_button_audio_capEnd_clicked();
    void on_button_audio_recordStart_clicked();

    void on_button_audio_recordEnd_clicked();

    void on_button_audio_sampleStart_clicked();

    void on_button_audio_sampleEnd_clicked();

    void on_button_audio_labelAdd_clicked();

    void on_button_audio_train_clicked();

signals:
    void commandIssued(const QByteArray &command);
    void recordStart(const QString &filename);
    void recordStop();
    void trainModel(const QVector<QString> filenames, const QVector<int> label);
};

#endif // AUDIOWIDGET_H
