#ifndef CPPLIB_DS_PRIORITY_QUEUE_H_
#define CPPLIB_DS_PRIORITY_QUEUE_H_

#include "pair.h"
#include "vector.h"

template<typename Key, typename Value>
class PriorityQueue
{
public:
	PriorityQueue(int size = 0): data_(size) {}

	void Create();
	void Insert(const Key &key, const Value &value);
	Value ExtractMin();
	bool Empty() const
	{
		return data_.Empty();
	}

	void ShowContent() const;

private:
	void FixUp(int child_index);
	void FixDown(int parent_index, int last);

	Vector<Pair<Key, Value>> data_; // Use Vector instead.
};
template<typename Key, typename Value>
void PriorityQueue<Key, Value>::Create()
{
	int size;
	scanf("%d", &size);
	data_.Resize(size);
	Key key;
	Value value;
	for(int index = 0; index < size; ++index)
	{
		scanf("%d %d", &key, &value);
		Insert(key, value);
	}
	ShowContent();
}
template<typename Key, typename Value>
void PriorityQueue<Key, Value>::Insert(const Key &key, const Value &value)
{
	data_.PushBack(Pair<Key, Value>(key, value));
	FixUp(data_.Size() - 1);
}
template<typename Key, typename Value>
Value PriorityQueue<Key, Value>::ExtractMin()
{
	if(Empty() == false)
	{
		std::swap(data_[0], data_[data_.Size() - 1]);
		data_.PopBack();
		FixDown(0, data_.Size());
		return data_[data_.Size()].value_;
	}
	return Value();
}
template<typename Key, typename Value>
void PriorityQueue<Key, Value>::FixUp(int child_index)
{
	int parent_index = (child_index - 1) / 2;
	while(parent_index >= 0 && data_[parent_index] > data_[child_index])
	{
		std::swap(data_[parent_index], data_[child_index]);
		child_index = parent_index;
		parent_index = (child_index - 1) / 2;
	}
}
template<typename Key, typename Value>
void PriorityQueue<Key, Value>::FixDown(int parent_index, int last)
{
	int min_child_index = parent_index * 2 + 1;
	while(min_child_index < last)
	{
		if(min_child_index < last - 1 &&
		        data_[min_child_index] > data_[min_child_index + 1])
		{
			++min_child_index;
		}
		if(!(data_[parent_index] > data_[min_child_index]))
		{
			return;
		}
		std::swap(data_[parent_index], data_[min_child_index]);
		parent_index = min_child_index;
		min_child_index = parent_index * 2 + 1;
	}
}
template<typename Key, typename Value>
void PriorityQueue<Key, Value>::ShowContent() const
{
	for(int index = 0; index < data_.Capacity(); ++index)
	{
		printf("<%d, %d> ", data_[index].key_, data_[index].value_);
	}
	printf("\n");
}
#endif // CPPLIB_DS_PRIORITY_QUEUE_H_
