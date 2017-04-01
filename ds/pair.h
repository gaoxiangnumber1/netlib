#ifndef CPPLIB_DS_PAIR_H_
#define CPPLIB_DS_PAIR_H_

#include <stdio.h>
#include <utility>

template<typename Key, typename Value>
struct Pair
{
	Pair(): key_(Key()), value_(Value()) {}
	Pair(const Key &key, const Value &value): key_(key), value_(value) {}
	Pair &operator=(Pair rhs)
	{
		Swap(rhs);
		return *this;
	}
	void Swap(Pair &rhs)
	{
		std::swap(key_, rhs.key_);
		std::swap(value_, rhs.value_);
	}

	Key key_;
	Value value_;
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
