#ifndef CPPLIB_DS_VERTEX_H_
#define CPPLIB_DS_VERTEX_H_

#include "queue.h"

struct Vertex
{
	Vertex(int index = 0, int weight = 0, Vertex *next = nullptr):
		index_(index),
		weight_(weight),
		in_degree_(0),
		out_degree_(0),
		visited_(false),
		next_(next),
		pq_index_(0)
	{}

	int index_;
	int weight_;
	int in_degree_;
	int out_degree_;
	bool visited_;
	Vertex *next_;
	int pq_index_; // Index in PriorityQueue.
};

void Visit(const Vertex *root)
{
	printf("%d ", root->index_);
}
void BFS(int src, Vertex *graph)
{
	Queue<Vertex*> queue;
	if(graph[src].visited_ == false)
	{
		queue.Enqueue(&graph[src]);
		graph[src].visited_ = true;
	}
	while(queue.Empty() == false)
	{
		Vertex *src_vertex = queue.Dequeue();
		Visit(src_vertex);
		for(Vertex *vertex = src_vertex->next_; vertex != nullptr; vertex = vertex->next_)
		{
			if(graph[vertex->index_].visited_ == false)
			{
				queue.Enqueue(&graph[vertex->index_]);
				graph[vertex->index_].visited_ = true;
			}
		}
	}
}
void DFS(int src, Vertex *graph)
{
	if(graph[src].visited_ == false)
	{
		Visit(&graph[src]);
		graph[src].visited_ = true;
		for(Vertex *vertex = graph[src].next_; vertex != nullptr; vertex = vertex->next_)
		{
			DFS(vertex->index_, graph);
		}
	}
}
#endif // CPPLIB_DS_VERTEX_H_
