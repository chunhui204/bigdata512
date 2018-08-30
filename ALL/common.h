#ifndef COMMON_H
#define COMMON_H

#include <QDebug>
#include <QStringList>
#include <QSemaphore>
#include <QImage>
#include <QVector>

//const QString
#define qcout qDebug()<<__FILE__<<__LINE__<<": "

/*
 * 命令传递网络端口：8888
 * 音频接受网络端口：8889
 * 视频接受网络端口：8890
 */
const quint16 COMMAND_PORT = 8888;
const quint16 AUDIO_PORT = 8889;
const quint16 VIDEO_PORT = 8890;

const int AudioBufSize = 176400;//44100hz,16bit ~ 2s

//缓冲空闲大小和已使用大小作为信号量
extern QSemaphore AudioBufFree; //==0，接收线程阻塞
extern QSemaphore AudioBufUsed; //==0，绘图线程阻塞
//spectrum线程
extern QSemaphore BufFree_spectrum; //==0，接收线程阻塞
extern QSemaphore BufUsed_spectrum; //==0，sepctrum线程阻塞
//缓冲数组,如果没有缓冲显示方要每次等待接收到数据显示数据，会很卡；如果缓冲太大，延迟会比较高
extern char AudioBuffer[AudioBufSize];
//为麦克风设备信息结构体
struct AudioSettingFormat
{
    QString deviceName;
    QString sampleRates;
    QString channel;
    QString sampleSizes;
    QString codec;
} ;
//摄像头
const int VideoBufferSize = 5;
extern QSemaphore VideoBufUsed;
extern QSemaphore VideoBufFree;
extern QImage VideoBuffer[VideoBufferSize];

/*
 * 文件保存目录
 * root目录下分
 * --root
 *      --AUDIO_PATH
 *          --AUDIO_FILE_PATH
 *          --AUDIO_TRAIN_PATH
 *      --VIDEO_PATH
 *          --VIDEO_AVI_PATH
 *          --VIDEO_IMAGE_PATH
 */
const QString AUDIO_PATH = "audioData";
const QString VIDEO_PATH = "videoData";
const QString AUDIO_FILE_PATH = "wavefiles";
const QString AUDIO_TRAIN_PATH = "trainfiles";
//音频福利叶变换，训练相关
const int FFTLengthPowerOfTwo = 12; //4096点 ~ 0.1s（44100hz采样频率）
const int SpectrumLengthSamples = 1 << FFTLengthPowerOfTwo;
//重叠采样的间隔点数。
const int AudioDummyLength = int(0.01 * 44100); //0.01s ~ 44100hz ~ 8bit 位数具体处理

const double CDF_THRESHOLD = 0.03;
const double PROB_THRESHOLD = 0.01;
const std::vector<int> GROUP={5,3,7};
const int SPECTRUM_FEAT_DIM = 15; //特征维数


#endif // COMMON_H
