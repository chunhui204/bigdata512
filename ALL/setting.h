#ifndef AUDIOSETTING_H
#define AUDIOSETTING_H

#include <QDialog>
#include "common.h"

namespace Ui {
class Setting;
}

class Setting : public QDialog
{
    Q_OBJECT

public:
    explicit Setting(QWidget *parent = 0);
    ~Setting();
    void designWidget(const QList<AudioSettingFormat> &) ;

signals:
    void audioFormatChanged(const AudioSettingFormat &);

private:
    Ui::Setting *ui;
    AudioSettingFormat currentFormat;
};

#endif // AUDIOSETTING_H
