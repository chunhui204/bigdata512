#include "spectrumanalyser.h"
#include <QtGlobal>
#include <qmath.h>
#include <numeric>

SpectrumAnalyser::SpectrumAnalyser(QObject *parent)
    : QObject(parent),
      m_fft(new ffft::FFTReal <double>(SpectrumLengthSamples))

{

}
SpectrumAnalyser::~SpectrumAnalyser()
{
    delete m_fft;
}
/*
 * 做傅里叶变换，计算频域特征作为信号特征
 * 做SpectrumLengthSamples个点的FFT，并分割成20个小区间求平均幅值，也就是20维特征，表示范围[0,fs/2]
 *@input param:
 *  audioStream：音频流
 *  unitBytes: 数据字节数1（8b）,2(16b
 *  featDim: 特征个数
 *@return:
 *  specTensor: 输出参数，二维tensor，（datanum, featdim），（n, SpectrumLengthSamples）与采样字节无关
 */
Tensor SpectrumAnalyser::calculateSpectFeature(const QByteArray &audioStream,
                                         int unitBytes,
                                         int featDim)
{
    //分割索引，分割的区间内音频表示用于训练/测的一个样本，长度为SpectrumLengthSamples
    QVector<int> splitIndex = splitAudioStream(audioStream, unitBytes);

    const int size = *(splitIndex.end() - 1) + SpectrumLengthSamples; //去掉冗余长度后流的数据点长度
    const int sample_num = splitIndex.size();//样本个数
    const int specm_num = SpectrumLengthSamples;//频率个数
    /**
      统一对所有样本进行处理计算，减少运算量
      **/
    double inputs[size]={0};
    Tensor outputs(sample_num, specm_num);
    Tensor specTensor(sample_num, featDim);

    for(int i=0;i< size; i++)
    {
        if(unitBytes == 2)
        {
             const qint16 d16 = *reinterpret_cast<const qint16*>(audioStream.data() + i* unitBytes);
            //scale data to [-1,1]
            inputs[i] = d16/ 32768.0;
        }
        else if(unitBytes == 1)
        {
            const qint8 d8 = *reinterpret_cast<const qint8*>(audioStream.data() + i* unitBytes);
            //scale data to [-1,1]
            inputs[i] = d8/ 128.0;
        }
    }
    //对每个样本fft
    for(int i=0; i < sample_num; i++)
    {
        m_fft->do_fft(outputs.begin() + i*specm_num, inputs + splitIndex[i]);
    }
    //post process
    postProcess(outputs, specTensor);

    return specTensor;
}
/*
 * 对FFT结果求频率对应的幅值，并将[0,fs/2]分割成20个小频率区间求平均幅值，也就是20维特征，
 * @param:
 *  输入tensor ， 输出tensor
 */
void SpectrumAnalyser::postProcess(const Tensor &inputs, Tensor &outputs)
{
    /**
    输入len长度得到也是输出长度（偶数），FFT返回结果是对称的，第一个元素为直流量，后面（1，（len-2）/2）,((len-2)/2+2,len-1)
    是实部一样，虚部相反数的复数，中间那个数也是直流量。ffft库考虑到这一点，
    （1，（len-2）/2）为存储实数部分，((len-2)/2+2,len-1)存储虚数部分，中间那个值不变。
    输出处理：求模后乘2除以len得到真实幅度。
    由乃奎斯特定理，频域N/2两部分对称，所以只用到前面部分表示的频率范围是[0, fs/2],fs: 采样频率
        **/
    double real=0;
    double imag=0;

    Q_ASSERT_X(inputs.rows()==outputs.rows() && inputs.cols()== SpectrumLengthSamples,
               "SpectrumAnalyser::postProcess",
               "inputs shape is different from outputs or inputs cols is wrong!");
    int bin_len = int((SpectrumLengthSamples/2) / outputs.cols());
    int sample_num = inputs.rows();
    double sum =0;

    for(int i=0; i< sample_num;i++)
    {
        sum = qAbs(inputs(i, 0));
        //求摸
        int j = 1;
        for(; j< bin_len * outputs.cols(); i++)//去除冗余后的长度
        {
            real = inputs(i, j);
            imag = inputs(i, j + SpectrumLengthSamples / 2);
            sum += qSqrt(real * real + imag * imag);

            if((j+1) % bin_len == 0)
            {
                outputs(i, j / bin_len) == sum / bin_len;
                sum = 0;
            }
        }
        //剩余元素扔进最后一个bin
        sum = 0;
        for(; j< SpectrumLengthSamples / 2; i++)
        {
            real = inputs(i, j);
            imag = inputs(i, j + SpectrumLengthSamples / 2);
            sum += qSqrt(real * real + imag * imag);
        }
        sum += qAbs(inputs(i, SpectrumLengthSamples/2));
        outputs(i, outputs.cols()-1) = (outputs(i,
                                                outputs.cols()-1)*bin_len + sum)
                                                / (bin_len + (SpectrumLengthSamples/2) % outputs.cols() + 1);

    }
}

/* TESTING...
 *  按照SpectrumLengthSamples * unitBytes分割音频流，分割的区间内音频表示用于训练/测的一个样本
 *@ return:
 *  表示数据点长度（字节长度 / unitByte）
 *  表示数据scale后数据点的起点位置，不是流中的字节数据位置，是与unitByets无关的
 */
QVector<int> SpectrumAnalyser::splitAudioStream(const QByteArray &stream, int unitBytes)
{
    /**
      将音频流分割成多个样本
      stream->size() = (num-1)*dummyLen + sampleLen;
    **/
    int sample_num = int((stream.size() - SpectrumLengthSamples * unitBytes)
                         / (AudioDummyLength* unitBytes)) + 1;//按字节长度取整后的样本个数
    QVector<int> ret;
    ret.reserve(sample_num);

    for(int i=0; i< sample_num; i++)
    {
        ret.push_back(i*AudioDummyLength);
    }
    return ret;
}
