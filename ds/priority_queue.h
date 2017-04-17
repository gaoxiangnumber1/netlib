#ifndef CPPLIB_DS_PRIORITY_QUEUE_H_
#define CPPLIB_DS_PRIORITY_QUEUE_H_

#include "pair.h"
#include "vector.h"
#include "binary_node.h"

template<typename Key, typename Value>
class PriorityQueue
{
public:
	PriorityQueue(int size = 0): data_(size) {}
	void Create();

	void Insert(const Key &key, const Value &value);
	void InsertWithIndex(const Key &key, const Value &value, int *pq_index);
	void DecreaseKey(int index, const Key &new_key);
	Value Minimum();
	Value ExtractMinimum();

	void HuffmanCode();

	bool Empty() const
	{
		return data_.Empty();
	}
	int Size() const
	{
		return data_.Size();
	}
	int Capacity() const
	{
		return data_.Capacity();
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
void PriorityQueue<Key, Value>::InsertWithIndex(const Key &key,
        const Value &value,
        int *pq_index_ptr)
{
	data_.PushBack(Pair<Key, Value>(key, value, pq_index_ptr));
	*pq_index_ptr = data_.Size() - 1;
	FixUp(*pq_index_ptr);
}
template<typename Key, typename Value>
void PriorityQueue<Key, Value>::DecreaseKey(int index, const Key &new_key)
{
	if(new_key < data_[index].key_)
	{
		data_[index].key_ = new_key;
		FixUp(index);
	}
}
template<typename Key, typename Value>
Value PriorityQueue<Key, Value>::Minimum()
{
	if(Empty() == false)
	{
		return data_[0].value_;
	}
	return Value();
}
template<typename Key, typename Value>
Value PriorityQueue<Key, Value>::ExtractMinimum()
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
void PriorityQueue<Key, Value>::HuffmanCode()
{
	int value_kind;
	scanf("%d", &value_kind);
	for(int index = 0; index < value_kind; ++index)
	{
		BinaryNode<int> *new_node = new BinaryNode<int>(0);
		scanf("%d %d", &new_node->weight_, &new_node->data_);
		Insert(new_node->weight_, new_node);
	}
	for(int cnt = 1; cnt <= value_kind - 1; ++cnt)
	{
		BinaryNode<int> *left_child = ExtractMinimum();
		BinaryNode<int> *right_child = ExtractMinimum();
		BinaryNode<int> *new_node = new BinaryNode<int>(0,
		        left_child,
		        right_child,
		        left_child->weight_ + right_child->weight_);
		Insert(new_node->weight_, new_node);
	}

	BinaryNode<int> *root = ExtractMinimum();
	printf("LevelOrder: ");
	LevelOrder(root);
	Delete(root);
	printf("\n");
}
template<typename Key, typename Value>
void PriorityQueue<Key, Value>::ShowContent() const
{
	for(int index = 0; index < data_.Size(); ++index)
	{
		printf("<%2d,%2d> ", data_[index].key_, data_[index].value_);
	}
	printf("\n");
}
#endif // CPPLIB_DS_PRIORITY_QUEUE_H_
