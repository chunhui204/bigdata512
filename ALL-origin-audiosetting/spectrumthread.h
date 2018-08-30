#ifndef SPECTRUMTHREAD_H
#define SPECTRUMTHREAD_H

#include <QObject>
#include "spectrumanalyser.h"
#include "wavfile.h"
#include "gaussmodel/tensor.h"
#include <gaussmodel/gaussianmg.h>
#include <qglobal.h>
#include <QTimer>

class SpectrumThread : public QObject
{
    Q_OBJECT
public:
    explicit SpectrumThread(QObject *parent = 0);
    ~SpectrumThread();
signals:
    void labelCalculated(int label);
    void alreadyTrained(const Tensor &, const Tensor &, const Tensor &);
public slots:
    void train_from_file(const QVector<QString> filenames, const QVector<int> label);
    void train_from_bytes(const QByteArray &stream, int label);
    void onAudioFormatInit(const QString &rate, const QString &chns, const QString &size);

private slots:
    void timeout();
private :
    //更新模型参数
    void update_paramsters(const Tensor &, const Tensor &,const Tensor &, const std::map< int, std::vector<int> > &);

    /*map: key:label, val: indexs */
    void group_by_label(const std::map< int, std::vector<int> > &label, const std::vector<Tensor*> &src, std::vector<Tensor> &dst);
    void deal_label(const QVector<int> &label, std::map< int, std::vector<int> > &arg_label);

private:
    QTimer *m_timer;
    int m_unitByte;
    //data, 行号是类别
    Tensor m_featMean;
    Tensor m_featStd;
    Tensor m_gate;
    QMap<int, int> m_labelIdx; //key为类别，值为在上述m_featMean中的行索引

    //fft
    SpectrumAnalyser *m_fft_analyser;
};

#endif // SPECTRUMTHREAD_H
