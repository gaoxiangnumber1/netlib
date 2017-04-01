#include "binary_node.h"

template<typename T>
class BinarySearchTree
{
public:
	BinarySearchTree(): root_(nullptr) {}
	~BinarySearchTree();

	void Create();
	// Return nullptr if fails, otherwise return the new node's pointer.
	BinaryNode<T> *Insert(const T &data);
	// Return nullptr if fails, otherwise return the match node's pointer.
	BinaryNode<T> *Search(const T &data, BinaryNode<T>* &parent_node);
	// Return nullptr if fails, otherwise return the node that replaces deleted node.
	BinaryNode<T> *Delete(const T &data);

	void LevelOrder() const;
	void Height() const;
	void NodeCount() const;

private:
	BinaryNode<T> *root_;
};

template<typename T>
BinarySearchTree<T>::~BinarySearchTree()
{
	::Delete(root_);
}
template<typename T>
void BinarySearchTree<T>::Create()
{
	int data_number;
	scanf("%d", &data_number);
	while(data_number-- > 0)
	{
		int data;
		scanf("%d", &data);
		Insert(data);
	}
}
template<typename T>
BinaryNode<T>* BinarySearchTree<T>::Insert(const T &data)
{
	BinaryNode<T> *parent_node = nullptr;
	if(Search(data, parent_node) != nullptr)
	{
		return nullptr;
	}
	BinaryNode<T> *new_node = new BinaryNode<T>(data);
	if(parent_node == nullptr) // BST is empty.
	{
		root_ = new_node;
	}
	else
	{
		if(data < parent_node->data_)
		{
			parent_node->left_ = new_node;
		}
		else
		{
			parent_node->right_ = new_node;
		}
	}
	return new_node;
}
template<typename T>
BinaryNode<T>* BinarySearchTree<T>::Search(const T &data, BinaryNode<T>* &parent_node)
{
	BinaryNode<T> *search_node = root_;
	while(search_node != nullptr)
	{
		if(data == search_node->data_)
		{
			return search_node;
		}
		parent_node = search_node; // Must after `if(==)` condition.
		if(data < search_node->data_)
		{
			search_node = search_node->left_;
		}
		else
		{
			search_node = search_node->right_;
		}
	}
	return nullptr;
}
template<typename T>
BinaryNode<T>* BinarySearchTree<T>::Delete(const T &data)
{
	BinaryNode<T> *parent_node = nullptr;
	BinaryNode<T> *delete_node = Search(data, parent_node);
	if(delete_node == nullptr) // Must FIRST check data isn't in tree.
	{
		return nullptr;
	}
	// Assume delete_node has 2 subnodes, change it to left_max_node,
	// then it has 1 or 0 subnodes.
	if(delete_node->left_ != nullptr && delete_node->right_ != nullptr)
	{
		BinaryNode<T> *left_max_parent = delete_node, *left_max = delete_node->left_;
		while(left_max->right_ != nullptr)
		{
			left_max_parent = left_max;
			left_max = left_max->right_;
		}
		delete_node->data_ = left_max->data_;
		parent_node = left_max_parent;
		delete_node = left_max;
	}
	BinaryNode<T> *child_node =
	    (delete_node->left_ ? delete_node->left_ : delete_node->right_);
	if(parent_node == nullptr)
	{
		root_ = child_node;
	}
	else
	{
		if(delete_node == parent_node->left_)
		{
			parent_node->left_ = child_node;
		}
		else
		{
			parent_node->right_ = child_node;
		}
	}
	delete delete_node;
	return child_node;
}
template<typename T>
void BinarySearchTree<T>::LevelOrder() const
{
	printf("LevelOrder: ");
	::LevelOrder(root_);
	printf("\n");
}
template<typename T>
void BinarySearchTree<T>::Height() const
{
	printf("Height:     %d\n", ::Height(root_));
}
template<typename T>
void BinarySearchTree<T>::NodeCount() const
{
	printf("NodeCount:  %d\n", ::NodeCount(root_));
}

int main()
{
	BinarySearchTree<int> tree;
	printf("0: Exit\n1: Create\n2: Insert\n3: Search\n4: Delete\n");
	int operation, data;
	BinaryNode<int> *temp = nullptr;
	while(scanf("%d", &operation) == 1)
	{
		switch(operation)
		{
		case 0:
			return 0;
		case 1:
			tree.Create();
			break;
		case 2:
			scanf("%d", &data);
			tree.Insert(data);
			break;
		case 3:
			scanf("%d", &data);
			if(tree.Search(data, temp) != nullptr)
			{
				printf("Found\n");
			}
			else
			{
				printf("Not Found\n");
			}
			break;
		case 4:
			scanf("%d", &data);
			if(tree.Delete(data) != nullptr)
			{
				printf("Deleted\n");
			}
			else
			{
				printf("Not Deleted\n");
			}
			break;
		}
		tree.LevelOrder();
		tree.Height();
		tree.NodeCount();
	}
}
/*
1 11 6 10 9 0 1 5 7 2 8 3 4
2 6
2 2
2 100
3 -1
3 0
3 100
3 101
4 -1
4 101
4 4
4 2
4 10
4 6
0

LevelOrder: 6 0 10 1 9 5 7 2 8 3 4
Height:     7
NodeCount:  11
LevelOrder: 6 0 10 1 9 5 7 2 8 3 4
Height:     7
NodeCount:  11
LevelOrder: 6 0 10 1 9 5 7 2 8 3 4
Height:     7
NodeCount:  11
LevelOrder: 6 0 10 1 9 100 5 7 2 8 3 4
Height:     7
NodeCount:  12
Not Found
LevelOrder: 6 0 10 1 9 100 5 7 2 8 3 4
Height:     7
NodeCount:  12
Found
LevelOrder: 6 0 10 1 9 100 5 7 2 8 3 4
Height:     7
NodeCount:  12
Found
LevelOrder: 6 0 10 1 9 100 5 7 2 8 3 4
Height:     7
NodeCount:  12
Not Found
LevelOrder: 6 0 10 1 9 100 5 7 2 8 3 4
Height:     7
NodeCount:  12
Not Deleted
LevelOrder: 6 0 10 1 9 100 5 7 2 8 3 4
Height:     7
NodeCount:  12
Not Deleted
LevelOrder: 6 0 10 1 9 100 5 7 2 8 3 4
Height:     7
NodeCount:  12
Not Deleted
LevelOrder: 6 0 10 1 9 100 5 7 2 8 3
Height:     6
NodeCount:  11
Deleted
LevelOrder: 6 0 10 1 9 100 5 7 3 8
Height:     5
NodeCount:  10
Deleted
LevelOrder: 6 0 9 1 7 100 5 8 3
Height:     5
NodeCount:  9
Deleted
LevelOrder: 5 0 9 1 7 100 3 8
Height:     4
NodeCount:  8
*/
