#include "spectrumthread.h"
#include <QThread>

SpectrumThread::SpectrumThread(QObject *parent) : QObject(parent)
{
    m_unitByte = 1;

    m_fft_analyser = new SpectrumAnalyser(this);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SpectrumThread::timeout);

    m_timer->start(1000); //1s
}

/*
 * 对实时数据进行类别预测
 */
void SpectrumThread::timeout()
{
    if(m_featMean.rows() == 0)//表示未训练过模型
        return;
    //一次性使用Audio buffer所有数据,当数据未连续充满一个数组长度时程序阻塞，保证数据的连续性
    BufUsed_spectrum.acquire(AudioBufSize);//lock when BufUsed_spectrum==0


    QByteArray data(AudioBuffer, AudioBufSize);
    Tensor testdata = m_fft_analyser->calculateSpectFeature(data, m_unitByte, SPECTRUM_FEAT_DIM);
    Tensor pred = gausianTest(testdata, m_featMean, m_featStd, m_gate, CDF_THRESHOLD, GROUP);
    //由于计算fft时将音频数据分为了多段，所以使用投票机制决定类别
    std::map<int, int> votes;
    for(int i=0; i< pred.rows(); i++)
    {
        if(votes.find(pred(i,0)) == votes.end())
            votes[pred(i,0)] = 0;
        votes[pred(i,0)] += 1;
    }
    int label = 0;//max votes一定>0,这样初始化不影响结果，理论上应该初始化为容器第一个元素
    int max_votes = 0;
    for(std::map<int, int>::iterator it = votes.begin(); it!=votes.end(); it++)
        if(it->second >= max_votes)
        {
            label = it->first;
            max_votes = it->second;
        }
    //通知界面显示
    emit labelCalculated(label);


    BufFree_spectrum.release(AudioBufSize);

}

/*
 * 从wav文件获取音频进行训练
 * 必须保证多次添加的数据类别是连贯的， 如0,1,0,2(0,1,2), 下次添加的数据类别不能跳过某个数字，如4,5（跳过了3）
 */
void SpectrumThread::train_from_file(const QVector<QString> filenames, const QVector<int> label)
{
    qcout << "spectrum thread: " << QThread::currentThreadId();
    Q_ASSERT_X(filenames.size()==label.size(),
               "SpectrumThread::train_from_file",
               "size not equal");
    std::vector<Tensor *> traindata;
    //sparse audio file to arrays
    WavFile file;
    for(QVector<QString>::const_iterator it =filenames.begin();it!=filenames.end();it++)
    {
        qcout << *it << label[0];
        if(file.open(*it, true)==false)
        {
            //error signals

        }
        QByteArray array = file.readAll();
        int unitByte = file.fileFormat().channelCount() * file.fileFormat().sampleSize() / 8;
        file.close();

        if(array.isEmpty())
            continue; //empty wav file
        Tensor data = m_fft_analyser->calculateSpectFeature(array, unitByte, SPECTRUM_FEAT_DIM);
        traindata.push_back(&data);
    }

    if(traindata.size() == 0)
    {
        emit alreadyTrained(m_featMean, m_featStd, m_gate);
        return;
    }

    std::map<int, std::vector<int> > arg_label;
    std::vector<Tensor> traindata_merged;
    //先将标签-索引 映射表计算出来
    deal_label(label, arg_label);
    //相同类的数据合并到一个tensor
    group_by_label(arg_label, traindata, traindata_merged);

    int cls_num = traindata_merged.size();
    //行号不是类别信息，类别信息是label_args的key，第几行就是第几个key
    Tensor featMean(cls_num, SPECTRUM_FEAT_DIM);
    Tensor featStd(cls_num, SPECTRUM_FEAT_DIM);
    Tensor gate(1, cls_num);
    //training...
    gausianTrain(traindata_merged, CDF_THRESHOLD, PROB_THRESHOLD, GROUP, featMean, featStd, gate);
    update_paramsters(featMean, featStd, gate, arg_label);
    emit alreadyTrained(m_featMean, m_featStd, m_gate);

}
/*
 * 从音频流获取音频进行训练
 * NOTE:
 *      在数据处理线程获得字节流最大长度是AudioBuffer数组，必须保证数组长度(2s)足够训练用
 */
void SpectrumThread::train_from_bytes(const QByteArray &stream, int label)
{
    if(stream.size() == 0)
    {
        emit alreadyTrained(m_featMean, m_featStd, m_gate);
        return;
    }
    std::vector<Tensor> traindata;
    Tensor data = m_fft_analyser->calculateSpectFeature(stream, m_unitByte, SPECTRUM_FEAT_DIM);
    traindata.push_back(data);

    std::map<int, std::vector<int> > arg_label;
    //先将标签-索引 映射表计算出来
    arg_label[label] = std::vector<int>();
    arg_label[label].push_back(0);

    int cls_num = traindata.size();
    //行号不是类别信息，类别信息是label_args的key，第几行就是第几个key
    Tensor featMean(cls_num, SPECTRUM_FEAT_DIM);
    Tensor featStd(cls_num, SPECTRUM_FEAT_DIM);
    Tensor gate(1, cls_num);
    //training...
    gausianTrain(traindata, CDF_THRESHOLD, PROB_THRESHOLD, GROUP, featMean, featStd, gate);
    update_paramsters(featMean, featStd, gate, arg_label);

    emit alreadyTrained(m_featMean, m_featStd, m_gate);
}
/*
 * 从音频流获取音频进行测试
 * NOTE:
 *      在数据处理线程获得字节流最大长度是AudioBuffer数组，必须保证数组长度(2s)足够测试用
 */
//Tensor gausianTest(
//    const Tensor & testdata,
//    const Tensor & featmean,
//    const Tensor & featstan,
//    const Tensor & gate,
//    double cdfthreshold,
//    const std::vector<int> &group)
/*
 * 不需要以此提供全部类别的数据，可以增加新类数据的模式进行训练
 * 为模型参数提供更新操作
 * 实质上是动态数组释放再分配的过程，所以尽量得到所有类别的样本后在训练，
 * 而且产生内存碎片
 */

void SpectrumThread::update_paramsters(const Tensor &featMean,
                       const Tensor &featStd,
                       const Tensor &gate,
                       const std::map< int, std::vector<int> > &arg_label)
{
    int idx = 0;
    std::map< int, std::vector<int> >::const_iterator map_it = arg_label.begin();
    for(;map_it!=arg_label.end();map_it++)
    {
        int label = map_it->first;
        //应经存在类别的模型参数怎么处理？

        //未存在类别的模型参数添加
        if(label >= m_featMean.rows())
            m_featMean.extend(featMean.row_vector(idx));
        else
            ;
        if(label >= m_featStd.rows())
            m_featStd.extend(featStd.row_vector(idx));
        else
            ;
        if(label >= m_gate.rows())
            m_gate.extend(gate.row_vector(idx));
        else
            ;
    }
}

SpectrumThread::~SpectrumThread()
{
    delete m_fft_analyser;

    if(m_timer->isActive())
        m_timer->stop();

}
void SpectrumThread::deal_label(const QVector<int> &label, std::map< int, std::vector<int> > &arg_label)
{
    for(int i=0;i< label.size(); i++)
    {
        if(arg_label.find(label[i]) == arg_label.end())
        {
            arg_label[label[i]] = std::vector<int>();
        }

        arg_label[label[i]].push_back(i);
    }
}
void SpectrumThread::group_by_label(const std::map< int, std::vector<int> > &arg_label,
                                    const std::vector<Tensor*> &src,
                                    std::vector<Tensor> &dst)
{
    std::map< int, std::vector<int> >::const_iterator map_it = arg_label.begin();
    for(;map_it!=arg_label.end();map_it++)
    {
        std::vector<int>::const_iterator vec_it=map_it->second.begin();
        std::vector<Tensor*> grp;
        for(;vec_it!=map_it->second.end();vec_it++)
            grp.push_back(src[*vec_it]);

        dst.push_back(stack_tensors(grp));
    }

}
void SpectrumThread::onAudioFormatInit(const QString &rate, const QString &chns, const QString &size)
{
    m_unitByte = chns.toInt() * size.toInt() / 8 ;
}
