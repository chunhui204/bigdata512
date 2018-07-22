#ifndef AUDIOSETTING_H
#define AUDIOSETTING_H

#include <QDialog>

//为麦克风设备信息结构体
typedef struct {
    QString deviceName;
    QStringList sampleRates;
    QString channel;
    QStringList sampleSizes;
    QString codec;
} AudioSettingFormat;

namespace Ui {
class AudioSetting;
}

class AudioSetting : public QDialog
{
    Q_OBJECT

public:
    explicit AudioSetting(QWidget *parent = 0);
    ~AudioSetting();
    void designWidget(const QList<AudioSettingFormat> &) ;

signals:
    void updateAudioFormat(const AudioSettingFormat &);

private:
    Ui::AudioSetting *ui;
    AudioSettingFormat currentFormat;
};

#endif // AUDIOSETTING_H
