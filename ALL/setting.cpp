#include "setting.h"
#include "ui_setting.h"

Setting::Setting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Setting)
{
    ui->setupUi(this);
    setWindowTitle(tr("请进行配置"));
}

Setting::~Setting()
{
    delete ui;
}

void Setting::designWidget(const QList<AudioSettingFormat> &formats)
{
    if(formats.size() == 0)
        return;

    ui->comboBox_device_audio->clear();

    //加载麦克风设备信息
    foreach(AudioSettingFormat fm, formats)
    {
        ui->comboBox_device_audio->addItem(fm.deviceName);
        ui->label_sampleRate_audio->setText(fm.sampleRates);
        ui->label_sampleSize_audio->setText(fm.sampleSizes);
        ui->label_chns_audio->setText(fm.channel);
        ui->label_codec_audio->setText(fm.codec);
    }
    //其他项只加载第一个设备的信息


    //当设备名发生改变时，采样率等信息随之变化
    connect(ui->comboBox_device_audio, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          [=](int index)
    {
        ui->label_sampleRate_audio->setText(formats[index].sampleRates);
        ui->label_chns_audio->setText(formats[index].channel);
        ui->label_sampleSize_audio->setText(formats[index].sampleSizes);
        ui->label_codec_audio->setText(formats[index].codec);
    });
    //点击确认
    connect(this, &QDialog::accepted,
            [=]()
    {
        //如果改变了配置，重置音频设备，并停止终端采样工作。
        //如果在录音过程或采集训练样本过程中打开dialog，破坏数据，待处理。
        if(ui->comboBox_device_audio->currentIndex()>0)
        {
            int ind = ui->comboBox_device_audio->currentIndex();
            currentFormat.deviceName = formats[ind].deviceName;
            currentFormat.channel = formats[ind].channel;
            currentFormat.sampleRates= formats[ind].sampleRates;
            currentFormat.sampleSizes = formats[ind].sampleSizes;
            currentFormat.codec = formats[ind].codec;
            emit audioFormatChanged(currentFormat);
        }
    });
    //点击取消
    connect(this, &QDialog::rejected,
            [=]()
    {
        close();
    });
}
