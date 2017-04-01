#ifndef CPPLIB_DS_VECTOR_H_
#define CPPLIB_DS_VECTOR_H_

template<typename T>
class Vector
{
private:
	T *data_;
	int size_;
	int capacity_;
};

#endif // CPPLIB_DS_VECTOR_H_
