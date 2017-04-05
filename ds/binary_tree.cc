#include "binary_node.h"

template<typename T>
class BinaryTree
{
public:
	BinaryTree(): root_(nullptr) {}
	~BinaryTree();

	void CreateCompleteBinaryTreeByLevel();
	void CreateTreeByPreAndIn();
	void CreateTreeByPostAndIn();
	// No `CreateTreeByPreAndPost` because we must need In to know scope [first, last),
	// otherwise the recursion won't stop.
	void PreOrderRecursive() const;
	void PreOrderLoop() const;
	void InOrderRecursive() const;
	void InOrderLoop() const;
	void PostOrderRecursive() const;
	void PostOrderLoop() const;
	void LevelOrder() const;
	void Height() const;
	void NodeCount() const;

private:
	BinaryNode<T> *root_;
};

template<typename T>
BinaryTree<T>::~BinaryTree()
{
	::Delete(root_);
}
template<typename T>
void BinaryTree<T>::CreateCompleteBinaryTreeByLevel()
{
	int data_number;
	scanf("%d", &data_number);
	T data_array[data_number];
	for(int index = 0; index < data_number; ++index)
	{
		scanf("%d", &data_array[index]);
	}

	int data_index = 0;
	root_ = new BinaryNode<T>(data_array[data_index++]);
	Queue<BinaryNode<T>*> queue;
	queue.Enqueue(root_);
	while(data_index < data_number)
	{
		BinaryNode<T> *node = queue.Dequeue();
		node->left_ = new BinaryNode<T>(data_array[data_index++]);
		queue.Enqueue(node->left_);
		if(data_index < data_number)
		{
			node->right_ = new BinaryNode<T>(data_array[data_index++]);
			queue.Enqueue(node->right_);
		}
	}
}
template<typename T>
void BinaryTree<T>::CreateTreeByPreAndIn()
{
	int data_number;
	scanf("%d", &data_number);
	T pre[data_number], in[data_number];
	for(int index = 0; index < data_number; ++index)
	{
		scanf("%d", &pre[index]);
	}
	for(int index = 0; index < data_number; ++index)
	{
		scanf("%d", &in[index]);
	}
	int pre_index = 0;
	::CreateTreeByPreAndIn(root_, pre, pre_index, in, 0, data_number);
}
template<typename T>
void BinaryTree<T>::CreateTreeByPostAndIn()
{
	int data_number;
	scanf("%d", &data_number);
	T post[data_number], in[data_number];
	for(int index = 0; index < data_number; ++index)
	{
		scanf("%d", &post[index]);
	}
	for(int index = 0; index < data_number; ++index)
	{
		scanf("%d", &in[index]);
	}
	int post_index = data_number - 1;
	::CreateTreeByPostAndIn(root_, post, post_index, in, 0, data_number);
}
template<typename T>
void BinaryTree<T>::PreOrderRecursive() const
{
	printf("PreOrder:   ");
	::PreOrderRecursive(root_);
	printf("\n");
}
template<typename T>
void BinaryTree<T>::PreOrderLoop() const
{
	printf("PreOrder:   ");
	::PreOrderLoop(root_);
	printf("\n");
}
template<typename T>
void BinaryTree<T>::InOrderRecursive() const
{
	printf("InOrder:    ");
	::InOrderRecursive(root_);
	printf("\n");
}
template<typename T>
void BinaryTree<T>::InOrderLoop() const
{
	printf("InOrder:    ");
	::InOrderLoop(root_);
	printf("\n");
}
template<typename T>
void BinaryTree<T>::PostOrderRecursive() const
{
	printf("PostOrder:  ");
	::PostOrderRecursive(root_);
	printf("\n");
}
template<typename T>
void BinaryTree<T>::PostOrderLoop() const
{
	printf("PostOrder:  ");
	::PostOrderLoop(root_);
	printf("\n");
}
template<typename T>
void BinaryTree<T>::LevelOrder() const
{
	printf("LevelOrder: ");
	::LevelOrder(root_);
	printf("\n");
}
template<typename T>
void BinaryTree<T>::Height() const
{
	printf("Height:     %d\n", ::Height(root_));
}
template<typename T>
void BinaryTree<T>::NodeCount() const
{
	printf("NodeCount:  %d\n", ::NodeCount(root_));
}

int main()
{
	printf("0: Exit\n"
	       "1: CreateCompleteBinaryTreeByLevel\n"
	       "2: CreateTreeByPreAndIn\n"
	       "3: CreateTreeByPostAndIn\n");
	int operation;
	while(scanf("%d", &operation) == 1)
	{
		BinaryTree<int> tree;
		switch(operation)
		{
		case 0:
			return 0;
		case 1:
			tree.CreateCompleteBinaryTreeByLevel();
			break;
		case 2:
			tree.CreateTreeByPreAndIn();
			break;
		case 3:
			tree.CreateTreeByPostAndIn();
			break;
		default:
			return 0;
		}
		tree.PreOrderRecursive();
		tree.PreOrderLoop();
		tree.InOrderRecursive();
		tree.InOrderLoop();
		tree.PostOrderRecursive();
		tree.PostOrderLoop();
		tree.LevelOrder();
		tree.Height();
		tree.NodeCount();
	}
}
/*
1 11 1 2 3 4 5 6 7 8 9 10 11
2 11 1 2 4 5 6 7 3 8 10 9 11 4 2 6 7 5 1 8 10 3 9 11
3 11 4 7 6 5 2 10 8 11 9 3 1 4 2 6 7 5 1 8 10 3 9 11
PreOrder:   1 2 4 8 9 5 10 11 3 6 7
InOrder:    8 4 9 2 10 5 11 1 6 3 7
PostOrder:  8 9 4 10 11 5 2 6 7 3 1
LevelOrder: 1 2 3 4 5 6 7 8 9 10 11
Height:     4
NodeCount:  11
PreOrder:   1 2 4 5 6 7 3 8 10 9 11
InOrder:    4 2 6 7 5 1 8 10 3 9 11
PostOrder:  4 7 6 5 2 10 8 11 9 3 1
LevelOrder: 1 2 3 4 5 8 9 6 10 11 7
Height:     5
NodeCount:  11
PreOrder:   1 2 4 5 6 7 3 8 10 9 11
InOrder:    4 2 6 7 5 1 8 10 3 9 11
PostOrder:  4 7 6 5 2 10 8 11 9 3 1
LevelOrder: 1 2 3 4 5 8 9 6 10 11 7
Height:     5
NodeCount:  11
*/
