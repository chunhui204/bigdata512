#ifndef AUDIOSETTING_H
#define AUDIOSETTING_H

#include <QDialog>
#include "common.h"

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
    void audioFormatChanged(const AudioSettingFormat &);

private:
    Ui::AudioSetting *ui;
    AudioSettingFormat currentFormat;
};

#endif // AUDIOSETTING_H
