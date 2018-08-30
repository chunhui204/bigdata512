#include "tensor.h"
#include <algorithm>
#include <iterator>
#include <assert.h>

Tensor::Tensor()
{
    row=0;
    col=0;
    buffer=NULL;
}

Tensor::Tensor(const Tensor & obj)
{
	row = obj.rows();
	col = obj.cols();

	buffer = new double[this->row * this->col];

    std::copy(obj.begin(), obj.end(), begin());
	//for (int i = 0; i < row; i++)
	//for (int j = 0; j < col; j++)
	//{
	//	*(buffer + i*col + j) = *(obj.buffer + i*col + j);
	//}

}
Tensor::~Tensor()
{
    row=0;
    col=0;
    delete[] buffer;
    buffer = NULL;
}
Tensor::Tensor(int rows)
{
	this->row = rows;
	this->col = 1;
	buffer = new double[this->row * this->col];
}
Tensor::Tensor(int rows, int cols)
{
    this->row = rows;
    this->col = cols;
    buffer = new double[this->row * this->col];
}

int Tensor::rows() const
{
    return row;
}
int Tensor::cols() const
{
    return col;
}
double & Tensor::operator()(int i, int j) const
{
    return buffer[i*col + j];
}


Tensor & Tensor::operator = (const Tensor& obj)
{
    if (this != &obj)
    {
        row = obj.rows();
        col = obj.cols();

        if(this != NULL)
            delete [] buffer;
        buffer = new double[this->row * this->col];

        std::copy(obj.begin(), obj.end(), begin());
    //    for(int i=0;i<row;i++)
    //        for(int j=0;j<col;j++)
    //        {
				//*(buffer + i*col + j) = *(obj.buffer + i*obj.cols() + j);
    //        }
    }
	
	return *this;
}
/*
*tensor.slice(a,b,c,d)取得元素为tensor[a]:tensor[b],b位置元素取不到，共取b-a个元素
*参数取默认值-1表示取到最结尾tensor.slice(0,-1,0,-1)取全部元素
*/
Tensor Tensor::slice(int start_row=0, int end_row=-1, int start_col=0, int end_col=-1) const
{
	if (start_row<0 || start_col<0 || end_row<-1 || end_col<-1 ||
		(start_row > end_row && end_row != -1) || (start_col > end_col && end_col != -1) ||
		start_row > this->row || start_col > this->col)
	{
        assert(false && "Tensor::slice: illegal paramesters");
	}
	if (end_row == -1)	end_row = this->row;
	if (end_col == -1)	end_col = this->col;

	Tensor obj(end_row - start_row, end_col - start_col);

	for (int m = 0, i = start_row; i < end_row; i++, m++)
	{
		for (int n=0, j = start_col; j < end_col; j++, n++)
		{
			obj(m, n) = (*this)(i, j);
		}
	}
	return obj;
}
Tensor Tensor::row_vector(int pos) const
{
    assert(!(pos <0 || pos >= col) && "Tensor::row_vector: illegal paramesters");
	Tensor ret(1, col);
    std::copy(buffer + pos*col, buffer + pos*col + col, ret.begin());

	return ret;
}

Tensor Tensor::col_vector(int pos) const
{
    assert(!(pos <0 || pos >= col) && "Tensor::col_vector: illegal paramesters");
	Tensor ret(row, 1);
	for (int i = 0; i < row; i++)
		ret(i, 0) = (*this)(i, pos);

	return ret;
}

void print(const Tensor & obj)
{
    int row = obj.rows();
    int col = obj.cols();

    for(int i=0;i<row;i++)
    {
        for(int j=0;j<col;j++)
        {
            std::cout<<obj(i,j)<<"\t";
        }
        std::cout<<std::endl;
    }
}
/*
求均值，返回Tensor
axis=0， 行方向,是对列向量平均得到行向量
axis=1， 列方向，对每个行向量平均得到列向量
*/
Tensor Tensor::mean(int axis) const
{
	double sum = 0;
	if (axis == 0)
	{
		Tensor obj(1, this->col);
		
		for (int i = 0; i < this->col; i++)
		{
			sum = 0;
			for (int j = 0; j < this->row; j++)
				sum += (*this)(j, i);
			obj(0, i) = sum / (this->row);
		}
		return obj;
	}else{
		Tensor obj(row, 1);

		for (int i = 0; i < row; i++)
		{
			sum = 0;
			for (int j = 0; j < col; j++)
				sum += (*this)(i, j);
			obj(i, 0) = sum / col;
		}
		return obj;
	}
}
/*
对一维向量求最大值以及在所在方向上的位置，注意只能是一维矩阵，某个轴上的维度为1。
val，pos均为外部变量。
*/
void Tensor::vector_arg_max(double &val, int &pos) const
{
	val = (*this)(0,0);
	pos = 0;
	for (int i = 0; i < row;i++)
	for (int j = 0; j < col; j++)
	{
		if ((*this)(i, j) > val)
		{
			pos = i*col + j;
			val = (*this)(i, j);
		}
	}
}
/*
* 扩充Tensor，重新分配内存,扩充的tensor列方向必须相同
* (n1, m), (n2, m)
*/
Tensor& Tensor::extend(const Tensor &obj)
{
    if(obj.isEmpty())
        return *this;

    if(this->isEmpty())
    {
        *this = obj;
        return *this;
    }
    assert(obj.cols() == this->cols() &&
               "Tensor::extend" &&
               "different cols shape!");
    double *buf_t = new double[(this->rows()+obj.rows()) * this->cols()];
    std::copy(begin(), end(), buf_t);
    std::copy(obj.begin(), obj.end(), buf_t+ row*col);

    row += obj.rows();
    delete[] buffer;
    buffer = buf_t;

    return *this;
}
Tensor::iterator Tensor::begin()
{
	return buffer;
}
Tensor::const_iterator Tensor::begin() const
{
	return buffer;
}
Tensor::iterator Tensor::end()
{
	return buffer + row * col;
}
Tensor::const_iterator Tensor::end() const
{
	return buffer + row * col;
}
bool Tensor::isEmpty() const
{
	return begin() == end();
}

/***************************common utils************/
Tensor stack_tensors(const std::vector<Tensor*> &ts)
{
    assert(ts.size()>0 && "stack_tensors: tensors vector size must larger than zero!");

    int size = 0;

    std::vector<Tensor *>::const_iterator it = ts.begin();
    int last_col = (*it)->cols();
    for(;it != ts.end(); it++)
    {
        size += (*it)->rows() * (*it)->cols();
        assert((*it)->cols() == last_col &&  "Tensor::extends " && "different cols shape!");
    }

    double *buf_t = 0;
    try
    {
         buf_t = new double[size];
    }catch(std::bad_alloc &e)
    {
        assert("Tensor::extends " && "memory allocate failed!!");
    }

    int used_size = 0;

    for(it = ts.begin(); it != ts.end(); it++)
    {
        std::copy((*it)->begin(), (*it)->end(), buf_t+ used_size);
        used_size += (*it)->rows() * (*it)->cols();
    }

    int col = (*(ts.begin()))->cols();
    int row = used_size / col;
    Tensor ret(row, col);

    ret.buffer = buf_t;

    return ret;
}
