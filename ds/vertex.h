#ifndef CPPLIB_DS_VERTEX_H_
#define CPPLIB_DS_VERTEX_H_

#include "queue.h"

struct Vertex
{
	Vertex():
		index_(),
		weight_(0),
		in_degree_(0),
		out_degree_(0),
		visited_(false),
		next_(nullptr)
	{}
	Vertex(int index, int weight = 0, Vertex *next = nullptr):
		index_(index),
		weight_(weight),
		in_degree_(0),
		out_degree_(0),
		visited_(false),
		next_(next)
	{}

	int index_;
	int weight_;
	int in_degree_;
	int out_degree_;
	bool visited_;
	Vertex *next_;
};

void Visit(const Vertex *root)
{
	printf("%d ", root->index_);
}
void DFS(int index, Vertex *graph)
{
	if(graph[index].visited_ == false)
	{
		Visit(&graph[index]);
		graph[index].visited_ = true;
		for(Vertex *vertex = graph[index].next_; vertex != nullptr; vertex = vertex->next_)
		{
			DFS(vertex->index_, graph);
		}
	}
}
void BFS(int index, Vertex *graph)
{
	Queue<Vertex*> queue;
	if(graph[index].visited_ == false)
	{
		queue.PushBack(&graph[index]);
		graph[index].visited_ = true;
	}
	while(queue.Empty() == false)
	{
		Vertex *root = queue.Front();
		queue.PopFront();
		Visit(root);
		for(Vertex *vertex = root->next_; vertex != nullptr; vertex = vertex->next_)
		{
			if(graph[vertex->index_].visited_ == false)
			{
				queue.PushBack(&graph[vertex->index_]);
				graph[vertex->index_].visited_ = true;
			}
		}
	}
}
#endif // CPPLIB_DS_VERTEX_H_
