#ifndef CPPLIB_DS_VECTOR_H_
#define CPPLIB_DS_VECTOR_H_

#include <stdio.h>
#include <utility>

// TODO: Separate "allocate memory" with "construct object"
// 1.Use std::allocator, see <C++ Primer> Section 13.5
template<typename T>
class Vector
{
public:
	Vector(int capacity = kInitialCapacity):
		size_(0),
		capacity_(capacity),
		data_(new T[capacity_])
	{}
	~Vector()
	{
		delete data_;
	}

	T *data()
	{
		return data_;
	}
	const T *data() const
	{
		return data_;
	}
	bool Empty() const
	{
		return size_ == 0;
	}
	int Size() const
	{
		return size_;
	}
	int Capacity() const
	{
		return capacity_;
	}
	T &operator[](int index)
	{
		return data_[index];
	}
	const T &operator[](int index) const
	{
		return data_[index];
	}

	void PushBack(const T &data_arg);
	void PopBack();
	void Resize(int new_capacity);
	void Swap(Vector &rhs);

private:
	int GetNextCapacity() const
	{
		return capacity_ + capacity_ / 2 + 1;
	}

	static const int kInitialCapacity = 4;
	int size_;
	int capacity_;
	T *data_;
};

template<typename T>
void Vector<T>::PushBack(const T &data_arg)
{
	if(size_ == capacity_)
	{
		Resize(GetNextCapacity());
	}
	data_[size_++] = data_arg;
}
template<typename T>
void Vector<T>::PopBack()
{
	--size_;
}
template<typename T>
void Vector<T>::Resize(int new_capacity)
{
	if(new_capacity < GetNextCapacity())
	{
		new_capacity = GetNextCapacity();
	}
	T *new_data = new T[new_capacity];
	for(int index = 0; index < size_; ++index)
	{
		new_data[index] = data_[index];
	}
	delete data_;
	capacity_ = new_capacity;
	data_ = new_data;
}
template<typename T>
void Vector<T>::Swap(Vector &rhs)
{
	std::swap(size_, rhs.size_);
	std::swap(capacity_, rhs.capacity_);
	std::swap(data_, rhs.data_);
}

#endif // CPPLIB_DS_VECTOR_H_
