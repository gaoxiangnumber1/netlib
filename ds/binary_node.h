#ifndef CPPLIB_DS_BINARY_NODE_H_
#define CPPLIB_DS_BINARY_NODE_H_

#include "stack.h"
#include "queue.h"

template<typename T>
struct BinaryNode
{
	BinaryNode(const T &data,
	           BinaryNode<T> *left = nullptr,
	           BinaryNode<T> *right = nullptr,
	           int weight = 0):
		data_(data),
		left_(left),
		right_(right),
		weight_(weight)
	{}

	T data_;
	BinaryNode<T> *left_;
	BinaryNode<T> *right_;
	int weight_;
};

template<typename T>
void Delete(BinaryNode<T> *&root)
{
	if(root != nullptr)
	{
		Delete(root->left_);
		Delete(root->right_);
		delete root;
		root = nullptr;
	}
}
template<typename T>
void CreateTreeByPreAndIn(BinaryNode<T> *&root,
                          T *pre,
                          int &pre_index, // MUST &!
                          T *in,
                          int in_first,
                          int in_last) // [in_last, in_last)
{
	if(in_first < in_last)
	{
		int index = in_first;
		for(; index < in_last && pre[pre_index] != in[index]; ++index);
		root = new BinaryNode<T>(pre[pre_index++]);
		CreateTreeByPreAndIn(root->left_, pre, pre_index, in, in_first, index);
		CreateTreeByPreAndIn(root->right_, pre, pre_index, in, index + 1, in_last);
	}
}
template<typename T>
void CreateTreeByPostAndIn(BinaryNode<T> *&root,
                           T *post,
                           int &post_index,
                           T *in,
                           int in_first,
                           int in_last) // [in_last, in_last)
{
	if(in_first < in_last)
	{
		int index = in_first;
		for(; index < in_last && post[post_index] != in[index]; ++index);
		root = new BinaryNode<T>(post[post_index--]);
		CreateTreeByPostAndIn(root->right_, post, post_index, in, index + 1, in_last);
		CreateTreeByPostAndIn(root->left_, post, post_index, in, in_first, index);
	}
}
template<typename T>
void PreOrderRecursive(BinaryNode<T> *root)
{
	if(root != nullptr)
	{
		Visit(root);
		PreOrderRecursive(root->left_);
		PreOrderRecursive(root->right_);
	}
}
template<typename T>
void PreOrderLoop(BinaryNode<T> *root)
{
	Stack<BinaryNode<T>*> stack;
	while(root != nullptr || stack.Empty() == false)
	{
		while(root != nullptr)
		{
			Visit(root);
			stack.Push(root);
			root = root->left_;
		}
		root = stack.Top()->right_;
		stack.Pop();
	}
}
template<typename T>
void InOrderRecursive(BinaryNode<T> *root)
{
	if(root != nullptr)
	{
		InOrderRecursive(root->left_);
		Visit(root);
		InOrderRecursive(root->right_);
	}
}
template<typename T>
void InOrderLoop(BinaryNode<T> *root)
{
	Stack<BinaryNode<T>*> stack;
	while(root != nullptr || stack.Empty() == false)
	{
		while(root != nullptr)
		{
			stack.Push(root);
			root = root->left_;
		}
		root = stack.Top();
		stack.Pop();
		Visit(root);
		root = root->right_;
	}
}
template<typename T>
void PostOrderRecursive(BinaryNode<T> *root)
{
	if(root != nullptr)
	{
		PostOrderRecursive(root->left_);
		PostOrderRecursive(root->right_);
		Visit(root);
	}
}
template<typename T>
void PostOrderLoop(BinaryNode<T> *root)
{
	Stack<BinaryNode<T>*> stack;
	while(root != nullptr || stack.Empty() == false)
	{
		while(root != nullptr)
		{
			stack.Push(root);
			root = root->left_;
		}
		if(stack.Top()->right_ == nullptr) // Leaf
		{
			root = stack.Top();
			stack.Pop();
			Visit(root);
			while(stack.Empty() == false &&
			        (stack.Top()->right_ == nullptr || stack.Top()->right_ == root))
			{
				root = stack.Top();
				stack.Pop();
				Visit(root);
			}
			if(stack.Empty() == false)
			{
				root = stack.Top()->right_;
			}
			else
			{
				root = nullptr;
			}
		}
		else
		{
			root = stack.Top()->right_;
		}
	}
}
template<typename T>
void LevelOrder(BinaryNode<T> *root)
{
	Queue<BinaryNode<T>*> queue;
	queue.Enqueue(root);
	while(queue.Empty() == false)
	{
		BinaryNode<T> *node = queue.Dequeue();
		Visit(node);
		if(node->left_ != nullptr)
		{
			queue.Enqueue(node->left_);
		}
		if(node->right_ != nullptr)
		{
			queue.Enqueue(node->right_);
		}
	}
}
template<typename T>
void Visit(BinaryNode<T> *node)
{
	printf("%d ", node->data_);
}
template<typename T>
int Height(BinaryNode<T> *root)
{
	if(root == nullptr)
	{
		return 0;
	}
	int left_height = Height(root->left_);
	int right_height = Height(root->right_);
	return left_height > right_height ? left_height + 1 : right_height + 1;
}
template<typename T>
int NodeCount(BinaryNode<T> *root)
{
	if(root == nullptr)
	{
		return 0;
	}
	return NodeCount(root->left_) + NodeCount(root->right_) + 1;
}
#endif // CPPLIB_DS_BINARY_NODE_H_
