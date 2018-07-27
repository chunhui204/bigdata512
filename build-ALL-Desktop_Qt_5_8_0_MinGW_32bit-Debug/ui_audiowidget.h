/********************************************************************************
** Form generated from reading UI file 'audiowidget.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AUDIOWIDGET_H
#define UI_AUDIOWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <qcustomplot.h>

QT_BEGIN_NAMESPACE

class Ui_AudioWidget
{
public:
    QStackedWidget *stackedWidget;
    QWidget *page_3;
    QFrame *frame_7;
    QWidget *widget_3;
    QVBoxLayout *verticalLayout;
    QLabel *label_audio_class;
    QCustomPlot *customPlot;
    QFrame *frame_8;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_3;
    QPushButton *button_audio_capStart;
    QPushButton *button_audio_capEnd;
    QSpacerItem *verticalSpacer_6;
    QLabel *label_7;
    QPushButton *button_audio_recordStart;
    QPushButton *button_audio_recordEnd;
    QWidget *widget_6;
    QHBoxLayout *horizontalLayout_4;
    QFrame *frame_9;
    QVBoxLayout *verticalLayout_4;
    QLabel *label_8;
    QWidget *widget_7;
    QHBoxLayout *horizontalLayout_5;
    QLineEdit *lineEdit_audio;
    QPushButton *button_audio_labelAdd;
    QLabel *label_9;
    QWidget *widget_8;
    QHBoxLayout *horizontalLayout_6;
    QPushButton *button_audio_sampleStart;
    QPushButton *button_audio_sampleEnd;
    QComboBox *combo_audio;
    QFrame *frame_10;
    QGridLayout *gridLayout_2;
    QLabel *label_10;
    QSpacerItem *verticalSpacer_7;
    QSpacerItem *verticalSpacer_8;
    QPushButton *button_audio_train;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *verticalSpacer_9;
    QPushButton *button_audio_param;
    QSpacerItem *verticalSpacer_10;
    QWidget *page_4;

    void setupUi(QWidget *AudioWidget)
    {
        if (AudioWidget->objectName().isEmpty())
            AudioWidget->setObjectName(QStringLiteral("AudioWidget"));
        AudioWidget->resize(800, 624);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(AudioWidget->sizePolicy().hasHeightForWidth());
        AudioWidget->setSizePolicy(sizePolicy);
        stackedWidget = new QStackedWidget(AudioWidget);
        stackedWidget->setObjectName(QStringLiteral("stackedWidget"));
        stackedWidget->setGeometry(QRect(0, 30, 800, 624));
        sizePolicy.setHeightForWidth(stackedWidget->sizePolicy().hasHeightForWidth());
        stackedWidget->setSizePolicy(sizePolicy);
        page_3 = new QWidget();
        page_3->setObjectName(QStringLiteral("page_3"));
        frame_7 = new QFrame(page_3);
        frame_7->setObjectName(QStringLiteral("frame_7"));
        frame_7->setGeometry(QRect(10, 0, 781, 351));
        frame_7->setFrameShape(QFrame::StyledPanel);
        frame_7->setFrameShadow(QFrame::Sunken);
        widget_3 = new QWidget(frame_7);
        widget_3->setObjectName(QStringLiteral("widget_3"));
        widget_3->setGeometry(QRect(0, 0, 621, 341));
        verticalLayout = new QVBoxLayout(widget_3);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label_audio_class = new QLabel(widget_3);
        label_audio_class->setObjectName(QStringLiteral("label_audio_class"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_audio_class->sizePolicy().hasHeightForWidth());
        label_audio_class->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(label_audio_class);

        customPlot = new QCustomPlot(widget_3);
        customPlot->setObjectName(QStringLiteral("customPlot"));

        verticalLayout->addWidget(customPlot);

        frame_8 = new QFrame(frame_7);
        frame_8->setObjectName(QStringLiteral("frame_8"));
        frame_8->setGeometry(QRect(620, 0, 161, 351));
        sizePolicy.setHeightForWidth(frame_8->sizePolicy().hasHeightForWidth());
        frame_8->setSizePolicy(sizePolicy);
        frame_8->setFrameShape(QFrame::StyledPanel);
        frame_8->setFrameShadow(QFrame::Sunken);
        frame_8->setLineWidth(10);
        verticalLayout_3 = new QVBoxLayout(frame_8);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        label_3 = new QLabel(frame_8);
        label_3->setObjectName(QStringLiteral("label_3"));

        verticalLayout_3->addWidget(label_3);

        button_audio_capStart = new QPushButton(frame_8);
        button_audio_capStart->setObjectName(QStringLiteral("button_audio_capStart"));

        verticalLayout_3->addWidget(button_audio_capStart);

        button_audio_capEnd = new QPushButton(frame_8);
        button_audio_capEnd->setObjectName(QStringLiteral("button_audio_capEnd"));

        verticalLayout_3->addWidget(button_audio_capEnd);

        verticalSpacer_6 = new QSpacerItem(20, 80, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_6);

        label_7 = new QLabel(frame_8);
        label_7->setObjectName(QStringLiteral("label_7"));

        verticalLayout_3->addWidget(label_7);

        button_audio_recordStart = new QPushButton(frame_8);
        button_audio_recordStart->setObjectName(QStringLiteral("button_audio_recordStart"));

        verticalLayout_3->addWidget(button_audio_recordStart);

        button_audio_recordEnd = new QPushButton(frame_8);
        button_audio_recordEnd->setObjectName(QStringLiteral("button_audio_recordEnd"));

        verticalLayout_3->addWidget(button_audio_recordEnd);

        widget_6 = new QWidget(frame_8);
        widget_6->setObjectName(QStringLiteral("widget_6"));
        horizontalLayout_4 = new QHBoxLayout(widget_6);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);

        verticalLayout_3->addWidget(widget_6);

        frame_9 = new QFrame(page_3);
        frame_9->setObjectName(QStringLiteral("frame_9"));
        frame_9->setGeometry(QRect(10, 350, 391, 211));
        frame_9->setFrameShape(QFrame::StyledPanel);
        frame_9->setFrameShadow(QFrame::Sunken);
        verticalLayout_4 = new QVBoxLayout(frame_9);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        label_8 = new QLabel(frame_9);
        label_8->setObjectName(QStringLiteral("label_8"));

        verticalLayout_4->addWidget(label_8);

        widget_7 = new QWidget(frame_9);
        widget_7->setObjectName(QStringLiteral("widget_7"));
        horizontalLayout_5 = new QHBoxLayout(widget_7);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        lineEdit_audio = new QLineEdit(widget_7);
        lineEdit_audio->setObjectName(QStringLiteral("lineEdit_audio"));

        horizontalLayout_5->addWidget(lineEdit_audio);

        button_audio_labelAdd = new QPushButton(widget_7);
        button_audio_labelAdd->setObjectName(QStringLiteral("button_audio_labelAdd"));

        horizontalLayout_5->addWidget(button_audio_labelAdd);


        verticalLayout_4->addWidget(widget_7);

        label_9 = new QLabel(frame_9);
        label_9->setObjectName(QStringLiteral("label_9"));

        verticalLayout_4->addWidget(label_9);

        widget_8 = new QWidget(frame_9);
        widget_8->setObjectName(QStringLiteral("widget_8"));
        horizontalLayout_6 = new QHBoxLayout(widget_8);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        button_audio_sampleStart = new QPushButton(widget_8);
        button_audio_sampleStart->setObjectName(QStringLiteral("button_audio_sampleStart"));

        horizontalLayout_6->addWidget(button_audio_sampleStart);

        button_audio_sampleEnd = new QPushButton(widget_8);
        button_audio_sampleEnd->setObjectName(QStringLiteral("button_audio_sampleEnd"));

        horizontalLayout_6->addWidget(button_audio_sampleEnd);

        combo_audio = new QComboBox(widget_8);
        combo_audio->setObjectName(QStringLiteral("combo_audio"));

        horizontalLayout_6->addWidget(combo_audio);


        verticalLayout_4->addWidget(widget_8);

        frame_10 = new QFrame(page_3);
        frame_10->setObjectName(QStringLiteral("frame_10"));
        frame_10->setGeometry(QRect(400, 350, 391, 211));
        frame_10->setFrameShape(QFrame::StyledPanel);
        frame_10->setFrameShadow(QFrame::Sunken);
        gridLayout_2 = new QGridLayout(frame_10);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        label_10 = new QLabel(frame_10);
        label_10->setObjectName(QStringLiteral("label_10"));

        gridLayout_2->addWidget(label_10, 0, 0, 1, 1);

        verticalSpacer_7 = new QSpacerItem(20, 13, QSizePolicy::Minimum, QSizePolicy::Fixed);

        gridLayout_2->addItem(verticalSpacer_7, 0, 1, 1, 1);

        verticalSpacer_8 = new QSpacerItem(20, 39, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer_8, 1, 1, 1, 1);

        button_audio_train = new QPushButton(frame_10);
        button_audio_train->setObjectName(QStringLiteral("button_audio_train"));

        gridLayout_2->addWidget(button_audio_train, 2, 0, 1, 2);

        horizontalSpacer_2 = new QSpacerItem(177, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_2, 2, 2, 1, 1);

        verticalSpacer_9 = new QSpacerItem(20, 36, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer_9, 3, 1, 1, 1);

        button_audio_param = new QPushButton(frame_10);
        button_audio_param->setObjectName(QStringLiteral("button_audio_param"));

        gridLayout_2->addWidget(button_audio_param, 4, 0, 1, 2);

        verticalSpacer_10 = new QSpacerItem(20, 5, QSizePolicy::Minimum, QSizePolicy::Fixed);

        gridLayout_2->addItem(verticalSpacer_10, 5, 0, 1, 2);

        stackedWidget->addWidget(page_3);
        page_4 = new QWidget();
        page_4->setObjectName(QStringLiteral("page_4"));
        stackedWidget->addWidget(page_4);

        retranslateUi(AudioWidget);

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(AudioWidget);
    } // setupUi

    void retranslateUi(QWidget *AudioWidget)
    {
        AudioWidget->setWindowTitle(QApplication::translate("AudioWidget", "Form", Q_NULLPTR));
        label_audio_class->setText(QApplication::translate("AudioWidget", "\347\261\273\345\210\253", Q_NULLPTR));
        label_3->setText(QApplication::translate("AudioWidget", "\347\273\210\347\253\257\346\216\247\345\210\266", Q_NULLPTR));
        button_audio_capStart->setText(QApplication::translate("AudioWidget", "\345\274\200\345\247\213\351\207\207\351\233\206", Q_NULLPTR));
        button_audio_capEnd->setText(QApplication::translate("AudioWidget", "\351\207\207\351\233\206\347\273\223\346\235\237", Q_NULLPTR));
        label_7->setText(QApplication::translate("AudioWidget", "\345\243\260\351\237\263\345\275\225\345\210\266", Q_NULLPTR));
        button_audio_recordStart->setText(QApplication::translate("AudioWidget", "\345\274\200\345\247\213\345\275\225\345\210\266", Q_NULLPTR));
        button_audio_recordEnd->setText(QApplication::translate("AudioWidget", "\347\273\223\346\235\237\345\275\225\345\210\266", Q_NULLPTR));
        label_8->setText(QApplication::translate("AudioWidget", "\346\240\207\347\255\276\351\207\207\351\233\206", Q_NULLPTR));
        button_audio_labelAdd->setText(QApplication::translate("AudioWidget", "\346\267\273\345\212\240\346\240\207\347\255\276", Q_NULLPTR));
        label_9->setText(QApplication::translate("AudioWidget", "\345\243\260\351\237\263\351\207\207\351\233\206", Q_NULLPTR));
        button_audio_sampleStart->setText(QApplication::translate("AudioWidget", "\345\274\200\345\247\213", Q_NULLPTR));
        button_audio_sampleEnd->setText(QApplication::translate("AudioWidget", "\347\273\223\346\235\237", Q_NULLPTR));
        label_10->setText(QApplication::translate("AudioWidget", "\346\250\241\345\236\213\350\256\255\347\273\203", Q_NULLPTR));
        button_audio_train->setText(QApplication::translate("AudioWidget", "\350\256\255\347\273\203", Q_NULLPTR));
        button_audio_param->setText(QApplication::translate("AudioWidget", "\345\217\221\351\200\201\345\217\202\346\225\260", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class AudioWidget: public Ui_AudioWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUDIOWIDGET_H
