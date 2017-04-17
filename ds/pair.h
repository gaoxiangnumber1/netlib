#ifndef CPPLIB_DS_PAIR_H_
#define CPPLIB_DS_PAIR_H_

#include <stdio.h>
#include <utility>

template<typename Key, typename Value>
struct Pair
{
	Pair(const Key &key = Key(),
	     const Value &value = Value(),
	     int *index_ptr = nullptr):
		key_(key),
		value_(value),
		index_ptr_(index_ptr)
	{}
	Pair(const Pair &rhs):
		key_(rhs.key_),
		value_(rhs.value_),
		index_ptr_(rhs.index_ptr_)
	{}
	Pair &operator=(Pair rhs)
	{
		Swap(rhs);
		return *this;
	}
	void Swap(Pair &rhs)
	{
		std::swap(key_, rhs.key_);
		std::swap(value_, rhs.value_);
		if(index_ptr_ != nullptr && rhs.index_ptr_ != nullptr)
		{
			std::swap(*index_ptr_, *rhs.index_ptr_);
		}
		std::swap(index_ptr_, rhs.index_ptr_);
	}

	Key key_;
	Value value_;
	int *index_ptr_;
};
template<typename Key, typename Value>
inline bool operator>(const Pair<Key, Value> &lhs,
                      const Pair<Key, Value> &rhs)
{
	return lhs.key_ > rhs.key_;
}
namespace std
{
template<typename Key, typename Value>
void swap(Pair<Key, Value> &lhs, Pair<Key, Value> &rhs)
{
	lhs.Swap(rhs);
}
}
#endif // CPPLIB_DS_PAIR_H_
