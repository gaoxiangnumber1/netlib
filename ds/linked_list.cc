#include "node.h"

template<typename T>
class LinkedList
{
public:
	LinkedList() :
		first_(nullptr), length_(0)
	{
	}
	~LinkedList();

	void Create();
	void Insert(int index, const T &data);
	void Delete(int index);
	void Reverse();

	bool Empty() const;
	int Size() const;
	void ShowContent() const;

private:
	Node<T> *first_;
	int length_;
};

template<typename T>
LinkedList<T>::~LinkedList()
{
	while(Empty() == false)
	{
		Node<T> *next = first_->next_;
		delete first_;
		first_ = next;
	}
}
template<typename T>
void LinkedList<T>::Create()
{
	int data_number, data;
	scanf("%d", &data_number);
	while(data_number-- > 0)
	{
		scanf("%d", &data);
		Insert(Size(), data);
	}
}
template<typename T>
void LinkedList<T>::Insert(int index, const T &data)
{
	if(index < 0 || index > Size()) // index must be in [0, Size()]
	{
		index = (index < 0 ? 0 : Size());
	}
	Node<T> *new_node = new Node<T>(data);
	if(index == 0) // Insert as the first node.
	{
		new_node->next_ = first_;
		first_ = new_node;
	}
	else
	{
		Node<T> *before_node = first_;
		for(int before_node_index = 0; before_node_index + 1 != index; ++before_node_index)
		{
			before_node = before_node->next_;
		}
		new_node->next_ = before_node->next_;
		before_node->next_ = new_node;
	}
	++length_;
}
template<typename T>
void LinkedList<T>::Delete(int index)
{
	if(Empty() == true) // Can't delete from empty linked list.
	{
		return;
	}
	if(index < 0 || index >= Size()) // index must be in [0, Size() - 1]
	{
		index = (index < 0 ? 0 : Size() - 1);
	}
	if(index == 0)
	{
		Node<T> *new_first = first_->next_;
		delete first_;
		first_ = new_first;
	}
	else
	{
		Node<T> *before_node = first_;
		for(int before_node_index = 0; before_node_index + 1 != index; ++before_node_index)
		{
			before_node = before_node->next_;
		}
		Node<T> *delete_node = before_node->next_;
		before_node->next_ = delete_node->next_;
		delete delete_node;
	}
	--length_;
}
template<typename T>
void LinkedList<T>::Reverse()
{
	if(Empty() == true)
	{
		return;
	}
	Node<T> *before_node = first_, *after_node = first_->next_;
	first_->next_ = nullptr;
	while(after_node != nullptr)
	{
		Node<T> *next_after_node = after_node->next_;
		after_node->next_ = before_node;
		before_node = after_node;
		after_node = next_after_node;
	}
	first_ = before_node;
}
template<typename T>
bool LinkedList<T>::Empty() const
{
	return first_ == nullptr;
}
template<typename T>
int LinkedList<T>::Size() const
{
	return length_;
}
template<typename T>
void LinkedList<T>::ShowContent() const
{
	printf("%02d data:", Size());
	for(Node<T> *node = first_; node != nullptr; node = node->next_)
	{
		printf(" %d", node->data_);
	}
	printf("\n");
}

// TODO: check edge case(only 1/2/3/4/5 elements)
void QuickSort(Node *first, Node *last) // [first, last]
{
	if(first == last)
	{
		return;
	}
	Node *right = first, *left = last;
	Partition(first, last, right, left); // [first, right] <= [divide] < [left, last]
	QuickSort(first, right);
	QuickSort(left, last);
}
void Partition(Node *first, Node *last, Node *&right, Node *&left)
{
	int pivot = last->value;
	Node *divide = first;
	for(Node *node = first; node != last; node = node->next)
	{
		if(node->value <= pivot)
		{
			if(node != divide)
			{
				swap(node->value, divide->value);
			}
			right = divide;
			divide = divide->next;
		}
	}
	if(divide != last)
	{
		swap(divide->value, last->value);
	}
	left = divide->next ? divide->next : left;
}

int main()
{
	LinkedList<int> object; // Stack object's dtor is auto called when scope ends.
	printf("0: Exit\n1: Create\n2: Insert\n3: Delete\n4: Reverse\n");

	int operation, data, index;
	while(scanf("%d", &operation) == 1)
	{
		switch(operation)
		{
		case 0:
			return 0;
		case 1:
			object.Create();
			object.ShowContent();
			break;
		case 2:
			scanf("%d %d", &index, &data);
			object.Insert(index, data);
			object.ShowContent();
			break;
		case 3:
			scanf("%d", &index);
			object.Delete(index);
			object.ShowContent();
			break;
		case 4:
			object.Reverse();
			object.ShowContent();
		}
	}
}
/*
 1 5 1 2 3 4 5
 4
 2 -1 0
 2 0 6
 2 4 7
 2 7 8
 2 9 9
 2 100 10
 4
 3 -1
 3 0
 3 5
 3 7
 3 5
 4
 0
 05 data: 1 2 3 4 5
 05 data: 5 4 3 2 1
 06 data: 0 5 4 3 2 1
 07 data: 6 0 5 4 3 2 1
 08 data: 6 0 5 4 7 3 2 1
 09 data: 6 0 5 4 7 3 2 8 1
 10 data: 6 0 5 4 7 3 2 8 1 9
 11 data: 6 0 5 4 7 3 2 8 1 9 10
 11 data: 10 9 1 8 2 3 7 4 5 0 6
 10 data: 9 1 8 2 3 7 4 5 0 6
 09 data: 1 8 2 3 7 4 5 0 6
 08 data: 1 8 2 3 7 5 0 6
 07 data: 1 8 2 3 7 5 0
 06 data: 1 8 2 3 7 0
 06 data: 0 7 3 2 8 1
 */
