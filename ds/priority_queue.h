#ifndef CPPLIB_DS_PRIORITY_QUEUE_H_
#define CPPLIB_DS_PRIORITY_QUEUE_H_

#include "pair.h"

template<typename Key, typename Value>
void FixUp(Pair<Key, Value> *data, int child_index)
{
	int parent_index = (child_index - 1) / 2;
	while(parent_index >= 0 && data[parent_index] > data[child_index])
	{
		std::swap(data[parent_index], data[child_index]);
		child_index = parent_index;
		parent_index = (child_index - 1) / 2;
	}
}
template<typename Key, typename Value>
void FixDown(Pair<Key, Value> *data, int parent_index, int last)
{
	int min_child_index = parent_index * 2 + 1;
	while(min_child_index < last)
	{
		if(min_child_index < last - 1 &&
		        data[min_child_index] > data[min_child_index + 1])
		{
			++min_child_index;
		}
		if(!(data[parent_index] > data[min_child_index]))
		{
			return;
		}
		std::swap(data[parent_index], data[min_child_index]);
		parent_index = min_child_index;
		min_child_index = parent_index * 2 + 1;
	}
}

template<typename Key, typename Value>
class PriorityQueue
{
public:
	PriorityQueue():
		current_length_(0),
		length_(0),
		data_(nullptr)
	{}
	PriorityQueue(int length):
		current_length_(0),
		length_(length),
		data_(new Pair<Key, Value>[length_])
	{}
	~PriorityQueue();

	void Create();
	void Insert(const Key &key, const Value &value);
	Value ExtractMin();
	bool Empty() const
	{
		return current_length_ == 0;
	}

	void ShowContent() const;

private:
	int current_length_;
	int length_;
	Pair<Key, Value> *data_; // Use Vector instead.
};
template<typename Key, typename Value>
PriorityQueue<Key, Value>::~PriorityQueue()
{
	delete data_;
}
template<typename Key, typename Value>
void PriorityQueue<Key, Value>::Create()
{
	scanf("%d", &length_);
	data_ = new Pair<Key, Value>[length_];
	Key key;
	Value value;
	for(int index = 0; index < length_; ++index)
	{
		scanf("%d %d", &key, &value);
		Insert(key, value);
	}
	ShowContent();
}
template<typename Key, typename Value>
void PriorityQueue<Key, Value>::Insert(const Key &key, const Value &value)
{
	if(current_length_ < length_)
	{
		data_[current_length_++] = Pair<Key, Value>(key, value);
		FixUp(data_, current_length_ - 1);
	}
}
template<typename Key, typename Value>
Value PriorityQueue<Key, Value>::ExtractMin()
{
	if(current_length_ > 0)
	{
		std::swap(data_[0], data_[--current_length_]);
		FixDown(data_, 0, current_length_);
		return data_[current_length_].value_;
	}
	return Value();
}
template<typename Key, typename Value>
void PriorityQueue<Key, Value>::ShowContent() const
{
	for(int index = 0; index < length_; ++index)
	{
		printf("<%d, %d> ", data_[index].key_, data_[index].value_);
	}
	printf("\n");
}
#endif // CPPLIB_DS_PRIORITY_QUEUE_H_
