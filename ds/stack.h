#ifndef CPPLIB_DS_STACK_H_
#define CPPLIB_DS_STACK_H_

#include "node.h"

template<typename T>
class Stack
{
public:
	Stack(): top_(nullptr), length_(0) {}
	~Stack();

	void Create();
	void Push(const T &data);
	void Pop();
	T Top() const;

	bool Empty() const;
	int Size() const;
	void ShowContent() const;

private:
	Node<T> *top_;
	int length_;
};

template<typename T>
Stack<T>::~Stack()
{
	while(Empty() == false)
	{
		Node<T> *next = top_->next_;
		delete top_;
		top_ = next;
	}
}

template<typename T>
void Stack<T>::Create()
{
	printf("Input: data_number data_content\n");
	int data_number;
	scanf("%d", &data_number);
	while(data_number-- > 0)
	{
		int data;
		scanf("%d", &data);
		Push(data);
	}
}
template<typename T>
void Stack<T>::Push(const T &data)
{
	Node<T> *new_node = new Node<T>(data, top_);
	top_ = new_node;
	++length_;
}
template<typename T>
void Stack<T>::Pop()
{
	if(Empty() == true)
	{
		return;
	}
	Node<T> *new_top = top_->next_;
	delete top_;
	top_ = new_top;
	--length_;
}
template<typename T>
T Stack<T>::Top() const
{
	if(Empty() == true)
	{
		return T();
	}
	return top_->data_;
}
template<typename T>
bool Stack<T>::Empty() const
{
	return top_ == nullptr;
}
template<typename T>
int Stack<T>::Size() const
{
	return length_;
}
template<typename T>
void Stack<T>::ShowContent() const
{
	printf("%02d data:", Size());
	for(Node<T> *temp = top_; temp != nullptr; temp = temp->next_)
	{
		printf(" %d", temp->data_);
	}
	printf("\n");
}
#endif // CPPLIB_DS_STACK_H_
