#ifndef CPPLIB_DS_QUEUE_H_
#define CPPLIB_DS_QUEUE_H_
#include "node.h"

template<typename T>
class Queue
{
public:
	Queue(): front_(nullptr), back_(nullptr), length_(0) {}
	~Queue();

	void Create();
	void Enqueue(const T &data);
	T Dequeue();

	bool Empty() const;
	int Size() const;
	void ShowContent() const;

private:
	Node<T> *front_;
	Node<T> *back_;
	int length_;
};

template<typename T>
Queue<T>::~Queue()
{
	while(Empty() == false)
	{
		Node<T> *next = front_->next_;
		delete front_;
		front_ = next;
	}
	back_ = front_;
}
template<typename T>
void Queue<T>::Create()
{
	printf("Input: data_number data_content\n");
	int data_number;
	scanf("%d", &data_number);
	while(data_number-- > 0)
	{
		int data;
		scanf("%d", &data);
		Enqueue(data);
	}
}
template<typename T>
void Queue<T>::Enqueue(const T &data)
{
	Node<T> *new_node = new Node<T>(data);
	if(Empty() == true)
	{
		front_ = new_node;
	}
	else
	{
		back_->next_ = new_node;
	}
	back_ = new_node;
	++length_;
}
template<typename T>
T Queue<T>::Dequeue()
{
	if(Empty() == true)
	{
		return T();
	}

	T data = front_->data_;
	if(Size() == 1)
	{
		back_ = nullptr;
	}
	Node<T> *new_front = front_->next_;
	delete front_;
	front_ = new_front;
	--length_;
	return data;
}
template<typename T>
bool Queue<T>::Empty() const
{
	return front_ == nullptr;
}
template<typename T>
int Queue<T>::Size() const
{
	return length_;
}
template<typename T>
void Queue<T>::ShowContent() const
{
	printf("%02d data:", Size());
	for(Node<T> *temp = front_; temp != nullptr; temp = temp->next_)
	{
		printf(" %d", temp->data_);
	}
	printf("\n");
}
#endif // CPPLIB_DS_QUEUE_H_
