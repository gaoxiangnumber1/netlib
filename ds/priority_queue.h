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
	// TODO: How to implement DecreaseKey() in O(logn)?

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
	data_.Enqueue(Pair<Key, Value>(key, value));
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

/*
void Graph::DijkstraShortestPath(int src)
{
	int pre_index[size_], path_cost[size_];
	const int kMax = 0x7fffffff;
	for(int index = 0; index < size_; ++index)
	{
		pre_index[index] = -1;
		path_cost[index] = kMax;
	}

	pre_index[src] = src;
	path_cost[src] = 0;
	PriorityQueue<int, int> priority_queue(size_);
	for(Vertex *vertex = graph_[src].next_; vertex != nullptr; vertex = vertex->next_)
	{
		pre_index[vertex->index_] = src;
		path_cost[vertex->index_] = vertex->weight_;
		priority_queue.Insert(path_cost[vertex->index_], vertex->index_);
	}
	while(priority_queue.Empty() == false)
	{
		int min_cost_index = priority_queue.ExtractMin();
		for(Vertex *vertex = graph_[min_cost_index].next_;
		        vertex != nullptr;
		        vertex = vertex->next_)
		{
			if(path_cost[vertex->index_] > path_cost[min_cost_index] + vertex->weight_)
			{
				path_cost[vertex->index_] = path_cost[min_cost_index] + vertex->weight_;
				if(pre_index[vertex->index_] == -1)
				{
					priority_queue.Insert(path_cost[vertex->index_], vertex->index_);
				}
				else
				{
					// TODO: How implement DecreaseKey() in O(logn)???
					priority_queue.DecreaseKey(path_cost[vertex->index_], vertex->index_);
				}
				pre_index[vertex->index_] = min_cost_index;
			}
		}
	}

	for(int index = 0; index < size_; ++index)
	{
		int temp_index = index, path[size_], edge_number = 0;
		while(pre_index[temp_index] != src)
		{
			path[edge_number++] = pre_index[temp_index];
			temp_index = pre_index[temp_index];
		}
		printf("(%d, %d) cost = %d edge = %d: %d",
		       src,
		       index,
		       path_cost[index],
		       index == src ? 0 : edge_number + 1,
		       src);
		for(int cnt = edge_number - 1; cnt >= 0; --cnt)
		{
			printf(" -> %d", path[cnt]);
		}
		printf(" -> %d\n", index);
	}
	Refresh();
}

*/
