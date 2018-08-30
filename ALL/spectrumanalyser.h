#ifndef SPECTRUMANALYSER_H
#define SPECTRUMANALYSER_H

#include <QObject>
#include <ffft/FFTReal.h>
#include <gaussmodel/gaussianmg.h>
#include <gaussmodel/tensor.h>
#include "common.h"
#include <QVector>

class SpectrumAnalyser : public QObject
{
    Q_OBJECT
public:
    explicit SpectrumAnalyser(QObject *parent=0);
    ~SpectrumAnalyser();

public:
    Tensor calculateSpectFeature(const QByteArray & ,int ,int );
private:
    QVector<int> splitAudioStream(const QByteArray &, int);
    void postProcess(const Tensor &inputs, Tensor &outputs);

private:
    ffft::FFTReal <double> *m_fft;
};

#endif // SPECTRUMANALYSER_H
