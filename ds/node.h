#ifndef CPPLIB_DS_NODE_H_
#define CPPLIB_DS_NODE_H_
#include <stdio.h>

template<typename T>
struct Node
{
	Node(const T &data, Node<T> *next = nullptr): data_(data), next_(next) {}

	T data_;
	Node<T> *next_;
};
#endif // CPPLIB_DS_NODE_H_
