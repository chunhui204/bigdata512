#include "audiosetting.h"
#include "ui_audiosetting.h"

AudioSetting::AudioSetting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AudioSetting)
{
    ui->setupUi(this);
    setWindowTitle(tr("请进行麦克风配置"));
}

AudioSetting::~AudioSetting()
{
    delete ui;
}

void AudioSetting::designWidget(const QList<AudioSettingFormat> &formats)
{
    if(formats.size() == 0)
        return;

    //加载麦克风设备信息
    foreach(AudioSettingFormat fm, formats)
        ui->comboBox_device->addItem(fm.deviceName);
    //其他项只加载第一个设备的信息
    ui->comboBox_sampRate->addItems(formats[0].sampleRates);
    ui->comboBox_chn->addItem(formats[0].channel);
    ui->comboBox_sampSize->addItems(formats[0].sampleSizes);
    ui->label_codec->setText(formats[0].codec);

    //当设备名发生改变时，采样率等信息随之变化
    connect(ui->comboBox_device, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          [=](int index)
    {
        ui->comboBox_sampRate->addItems(formats[index].sampleRates);
        ui->comboBox_chn->addItem(formats[index].channel);
        ui->comboBox_sampSize->addItems(formats[index].sampleSizes);
        ui->label_codec->setText(formats[index].codec);
    });
    //点击确认
    connect(this, &QDialog::accepted,
            [=]()
    {
        //如果改变了配置，重置音频设备，并停止终端采样工作。
        //如果在录音过程或采集训练样本过程中打开dialog，破坏数据，待处理。
        if(ui->comboBox_device->currentIndex()>0 || ui->comboBox_chn->currentIndex()>0||
                ui->comboBox_sampRate->currentIndex()>0 || ui->comboBox_sampSize->currentIndex()>0)
        {
            int ind = ui->comboBox_device->currentIndex();
            currentFormat.deviceName = formats[ind].deviceName;
            currentFormat.channel = formats[ind].channel;
            currentFormat.sampleRates.append(formats[ind].sampleRates[ui->comboBox_sampRate->currentIndex()]);
            currentFormat.sampleSizes.append(formats[ind].sampleSizes[ui->comboBox_sampSize->currentIndex()]);
            currentFormat.codec = formats[ind].codec;

            emit updateAudioFormat(currentFormat);
        }
    });
    //点击取消
    connect(this, &QDialog::rejected,
            [=]()
    {
        close();
    });
}
