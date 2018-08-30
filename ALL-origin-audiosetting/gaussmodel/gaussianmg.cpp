#include "gaussianmg.h"
#include <math.h>
#include <algorithm>
#include <assert.h>
#include <numeric>
/*
    reference:
    [1] http://www.cplusplus.com/reference/cmath/erf/
    [2] https://zh.wikipedia.org/wiki/%E6%AD%A3%E6%80%81%E5%88%86%E5%B8%83
*/
double normpdf(const double miu, const double sigma, const double x)
{
	return exp(-(x - miu)*(x - miu)*0.5 / (sigma*sigma))*RSQRT2PI / sigma;
}
double normcdf(const double miu, const double sigma, const double x)
{
	return 0.5 + 0.5*erf((x - miu)*RSQRT2 / sigma);
}
/*
*正态分布拟合
*data必须是一维列向量(n,1)
*miu，sigma是外部变量传入，返回拟合高斯的参数
*/
void normfit(const Tensor & data, double & miu, double & sigma)
{
	//miu
	double sum = 0;
	int N = data.rows()*data.cols();
	for (int i = 0; i < data.rows(); i++)
	for (int j = 0; j < data.cols(); j++)
		sum += data(i, j);
	miu = sum / N;
	//sigma
	sum = 0;
	for (int i = 0; i < data.rows(); i++)
	for (int j = 0; j < data.cols(); j++)
		sum += (data(i, j) - miu)*(data(i, j) - miu);
	sigma = sqrt(sum / (N - 1));
}
/*
* 只适用于横轴值在[0,1]
* 二分法查找，时间复杂度O(log2(resolution))
*/
double norminv(double miu, double sigma, double prob)
{
	assert(prob <= 1 && prob >=0 && "prob must lager than 0 and little than 1");
	//将[0,1]分为resolution部分，返回值精度为1e-4
	const int resolution = 10000;
	int low = 0, high = resolution;
	int mid = (low + high) / 2;
	while (low <= high)
	{
		mid = (low + high) / 2;
		double val = normcdf(miu, sigma, mid / 1.0 / resolution);
		double next_val = normcdf(miu, sigma, (mid + 1) / 1.0 / resolution);
		if (val == prob || (val - prob)*(next_val - prob) < 0)
			return mid / 1.0 / resolution;
		else if (val < prob)
			low = mid + 1;
		else
			high = mid - 1;
	}
    return -1; //never run here
}
/*
*模型训练
*根据已知数据找出每类数据的每个维度特征的均值和标准差
*@input params
	trainData : 训练数据，三个维度一次是样本数量、特征维度、类别维度
	cdfThreshold: 设定样本cdf的阈值 令cdf小于cdfgate和大于1-cdfgate的点的pdf置零     例如取0.03
	probThreshold： 设定总的概率的容错率  将会有probthreshold的正样本可能被分为负样本，设为0则有可能提高误检率 例如取0.01
	group ： 向量，用来划定哪几维的特征是一个组合,如[5 3 7]表示从左到右的特征共有三组，维度依次为5维 3维
	7维，特征维度为5+3+7=15维
*@output params:
	一定要传入外部对象，非临时对象
	featMean: 特征每个维度的均值 每行为一类数据，每列为特征的一个维度
	featStd: 特征每个维度的标准差
	gate: 概率（pdf）均值的门限阈值 行向量
*
*/
void gausianTrain(
    const std::vector<Tensor> &trainData,
    double cdfThreshold,
	double probThreshold, 
    const std::vector<int> &group,
	Tensor &featMean,
	Tensor &featStd,
	Tensor &gate)
{
	int classNum = trainData.size();
    int dataNum = trainData[0].rows();
    int featDim = trainData[0].cols();

    assert(trainData.size()>0 && "训练数据为空") ;

    assert((featMean.rows() == classNum && featMean.cols() == featDim) && "gaussianTrain value err: featMean shape is not right!");
    assert((featStd.rows() == classNum && featStd.cols() == featDim) && "gaussianTrain value err:  featStd shape is not right!");
    assert((gate.rows() == 1 && gate.cols() == classNum) && "gaussianTrain value err: gate shape is not right!");

    int s = 0;
    std::accumulate(group.begin(), group.end(), s);
    assert(s == featDim && "gaussianTrain value err: group elements not match with featDIm");

    for(int cn = 0;cn< classNum; cn++)  //处理第cn类数据
    {
        Tensor data = trainData[cn];
		Tensor probp(dataNum, featDim);
        //根据已知数据找出每类数据的每个维度特征的均值和标准差
		for (int dim = 0; dim < featDim; dim++)
		{
			double miu = 0;
			double sigma = 0;
			
			normfit(data.col_vector(dim), miu, sigma);
			featMean(cn, dim) = miu;
			featStd(cn, dim) = sigma;
			//通过训练数据找到pdf的门限阈值
			for (int num = 0; num < dataNum; num++)
			{
				double cdf = normcdf(miu, sigma, data(num, dim)); //calc cdf
                if (cdf < cdfThreshold || cdf > 1 - cdfThreshold)
					probp(num, dim) = 0;
				else
					probp(num, dim) = normpdf(miu, sigma, data(num, dim));//calc pdf
			}
		}
		//对每个特征组内的概率求均值，作为该组所有特征维度的概率值
		Tensor probpt(dataNum, group.size());
		int last_index = 0;
		for (int g = 0; g < group.size(); g++)
		{
			Tensor temp = probp.slice(0, -1, last_index, group[g]).mean(1);
			for (int num = 0; num < dataNum; num++)
				probpt(num, g) = temp(num, 0);
			last_index = group[g];
		}
		Tensor meanProbpt = probpt.mean(1);
		double probptMiu = 0; 
		double probptSigma = 0;
		normfit(meanProbpt, probptMiu, probptSigma);

		gate(0, cn) = norminv(probptMiu, probptSigma, probThreshold);
    }

}
/*
* 
@param:
	testdata: 测试数据,两个维度，第一个维度表示样本数量，第二个表示特征维度
	featmean：特征均值，类别维度和特征维度,		由训练函数接入
	featstan：特征标准差，维度同上				由训练函数接入
	gate： 行向量(1, n),						由训练函数接入
	cdfthreshold:  pdf均值阈值				由训练函数接入
	group: 向量，用来划定哪几维的特征是一个组合,如[5 3 7]表示从左到右的特征共有三组，维度依次为5维 3维
	            7维，特征维度为5+3+7=15维
@return:
    result   每行为一个样本，就一个数字，预测的标签，顺序为训练时的页的顺序,未见过的样本为输出标签为-1
*/
Tensor gausianTest(
	const Tensor & testdata,
	const Tensor & featmean, 
	const Tensor & featstan,
	const Tensor & gate,
	double cdfthreshold, 
    const std::vector<int> &group)
{
    assert(testdata.isEmpty() == false && "测试数据为空") ;

	int classNum = featmean.rows();
	int dataNum = testdata.rows();
	int featDim = testdata.cols();

	Tensor resultpb(dataNum, classNum);
	Tensor result(dataNum, 1);
	for (int cn = 0; cn< classNum; cn++)  //按每类数据测试
	{
		Tensor probp(dataNum, featDim);
		//按每个维度测试
		for (int dim = 0; dim < featDim; dim++)
		{
			for (int num = 0; num < dataNum; num++)
			{
				double cdf = normcdf(featmean(cn, dim), featstan(cn, dim), testdata(num, dim)); //calc cdf

				//概率密度在概率分布两端的点被设置为0，最后以概率密度为基准 
				if (cdf < cdfthreshold || cdf > 1 - cdfthreshold)
					probp(num, dim) = 0;
				else
					probp(num, dim) = normpdf(featmean(cn, dim), featstan(cn, dim), testdata(num, dim));//calc pdf
			}
		}
		//print(probp.slice(0, -1, 0,-1));
		//对每个特征组内的概率求均值，作为该组所有特征维度的概率值
		Tensor probpt(dataNum, group.size());
		int last_index = 0;
		for (int g = 0; g < group.size(); g++)
		{
			Tensor temp = probp.slice(0, -1, last_index, last_index+group[g]).mean(1);
			
			for (int num = 0; num < dataNum; num++)
				probpt(num, g) = temp(num, 0);
			//refresh last_index
			last_index = group[g];
		}
//		cout << "probpt:" << endl;
//		print(probpt);
		Tensor meanProbpt = probpt.mean(1);
		for (int num = 0; num < dataNum; num++)
		{
			if (meanProbpt(num, 0) < gate(0, cn))
				meanProbpt(num, 0) = 0;
			//属于每一类的pdf均值，每行为一个样本，每列为一类
			resultpb(num, cn) = meanProbpt(num, 0);
		}
		
	}

//	cout << "resultpb" << endl;
//	print(resultpb);
	//当一个样本属于所有类别的概率均为0（或最大值为0）时认为是异常点
	for (int num = 0; num < dataNum; num++)
	{
		double maxval = 0;
		int maxpos = 0;
		resultpb.row_vector(num).vector_arg_max(maxval, maxpos);
		if (maxval <= 0)
            result(num, 0) = -1;
		else
			result(num, 0) = maxpos;
	}
	return result;
}
