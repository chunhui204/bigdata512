#ifndef TENSOR_H
#define TENSOR_H
#include <iostream>
#include <vector>

class Tensor
{
    friend Tensor stack_tensors(const std::vector<Tensor*> &ts);
//二维数组（比三维更容易处理，实现）
public:

    typedef double* iterator;
    typedef const double* const_iterator;

    Tensor();
    ~Tensor();
	Tensor(const Tensor & obj);
    explicit Tensor(int rows);
	Tensor(int rows, int cols);

    //private数据的getter函数
    int rows() const;
    int cols() const;
    //运算符
    double & operator()(int i, int j) const;
	Tensor & operator = (const Tensor& obj);
    //类似于matlab切片操作，不能对切片数组进行赋值操作
	Tensor slice(int start_row, int end_row, int start_col, int end_col) const;
	//返回索引指定行位置的行向量,(1,n)
	Tensor row_vector(int pos) const;
	//返回索引指定列位置的列向量，（n,1）
	Tensor col_vector(int pos) const;
	//non-sense
//	Tensor concatenate(const vector<Tensor> & vec, int axis) const;
	//mean
	Tensor mean(int axis) const;
	void vector_arg_max(double &val, int &pos) const;
    //扩充Tensor，重新分配内存
    Tensor & extend(const Tensor &obj);

	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;
    bool isEmpty() const;

private:
    iterator buffer;
    int row;
    int col;

};


//void print(const Tensor & obj);

#endif // TENSOR_H
