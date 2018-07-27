/********************************************************************************
** Form generated from reading UI file 'audiosetting.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AUDIOSETTING_H
#define UI_AUDIOSETTING_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AudioSetting
{
public:
    QVBoxLayout *verticalLayout;
    QWidget *widget;
    QGridLayout *gridLayout_2;
    QLabel *label;
    QComboBox *comboBox_device;
    QLabel *label_2;
    QComboBox *comboBox_sampRate;
    QLabel *label_3;
    QComboBox *comboBox_chn;
    QLabel *label_4;
    QComboBox *comboBox_sampSize;
    QLabel *label_5;
    QLabel *label_codec;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *AudioSetting)
    {
        if (AudioSetting->objectName().isEmpty())
            AudioSetting->setObjectName(QStringLiteral("AudioSetting"));
        AudioSetting->resize(374, 229);
        verticalLayout = new QVBoxLayout(AudioSetting);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        widget = new QWidget(AudioSetting);
        widget->setObjectName(QStringLiteral("widget"));
        gridLayout_2 = new QGridLayout(widget);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        label = new QLabel(widget);
        label->setObjectName(QStringLiteral("label"));

        gridLayout_2->addWidget(label, 0, 0, 1, 1);

        comboBox_device = new QComboBox(widget);
        comboBox_device->setObjectName(QStringLiteral("comboBox_device"));

        gridLayout_2->addWidget(comboBox_device, 0, 1, 1, 1);

        label_2 = new QLabel(widget);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout_2->addWidget(label_2, 1, 0, 1, 1);

        comboBox_sampRate = new QComboBox(widget);
        comboBox_sampRate->setObjectName(QStringLiteral("comboBox_sampRate"));

        gridLayout_2->addWidget(comboBox_sampRate, 1, 1, 1, 1);

        label_3 = new QLabel(widget);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout_2->addWidget(label_3, 2, 0, 1, 1);

        comboBox_chn = new QComboBox(widget);
        comboBox_chn->setObjectName(QStringLiteral("comboBox_chn"));

        gridLayout_2->addWidget(comboBox_chn, 2, 1, 1, 1);

        label_4 = new QLabel(widget);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout_2->addWidget(label_4, 3, 0, 1, 1);

        comboBox_sampSize = new QComboBox(widget);
        comboBox_sampSize->setObjectName(QStringLiteral("comboBox_sampSize"));

        gridLayout_2->addWidget(comboBox_sampSize, 3, 1, 1, 1);

        label_5 = new QLabel(widget);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout_2->addWidget(label_5, 4, 0, 1, 1);

        label_codec = new QLabel(widget);
        label_codec->setObjectName(QStringLiteral("label_codec"));

        gridLayout_2->addWidget(label_codec, 4, 1, 1, 1);


        verticalLayout->addWidget(widget);

        verticalSpacer = new QSpacerItem(20, 13, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        buttonBox = new QDialogButtonBox(AudioSetting);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(AudioSetting);
        QObject::connect(buttonBox, SIGNAL(accepted()), AudioSetting, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), AudioSetting, SLOT(reject()));

        QMetaObject::connectSlotsByName(AudioSetting);
    } // setupUi

    void retranslateUi(QDialog *AudioSetting)
    {
        AudioSetting->setWindowTitle(QApplication::translate("AudioSetting", "Dialog", Q_NULLPTR));
        label->setText(QApplication::translate("AudioSetting", "\350\256\276\345\244\207\345\220\215\347\247\260", Q_NULLPTR));
        label_2->setText(QApplication::translate("AudioSetting", "\351\207\207\346\240\267\347\216\207", Q_NULLPTR));
        label_3->setText(QApplication::translate("AudioSetting", "\351\207\207\346\240\267\351\200\232\351\201\223", Q_NULLPTR));
        label_4->setText(QApplication::translate("AudioSetting", "\351\207\207\346\240\267\344\275\215\346\225\260", Q_NULLPTR));
        label_5->setText(QApplication::translate("AudioSetting", "\347\274\226\347\240\201\346\240\274\345\274\217", Q_NULLPTR));
        label_codec->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class AudioSetting: public Ui_AudioSetting {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUDIOSETTING_H
