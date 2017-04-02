#include "vertex.h"

class Graph
{
public:
	Graph(): size_(0), graph_(nullptr) {}
	~Graph();

	void Create();
	void Insert(int src, int dest, int weight);
	void BFS(int src);
	void DFS(int src);
	void TopologicalSort();
	void DijkstraShortestPath(int src);

private:
	void Refresh();

	int size_;
	Vertex *graph_;
};

Graph::~Graph()
{
	for(int index = 0; index < size_; ++index)
	{
		Vertex *vertex = graph_[index].next_;
		while(vertex != nullptr)
		{
			Vertex *next = vertex->next_;
			delete vertex;
			vertex = next;
		}
	}
	delete [] graph_;
}
void Graph::Create()
{
	scanf("%d", &size_);
	graph_ = new Vertex[size_];
	for(int index = 0; index < size_; ++index)
	{
		graph_[index].index_ = index;
	}
	int edge_number;
	scanf("%d", &edge_number);
	while(edge_number-- > 0)
	{
		int src, dest;
		int weight;
		scanf("%d %d %d", &src, &dest, &weight);
		Insert(src, dest, weight);
	}
}
void Graph::Insert(int src, int dest, int weight)
{
	Vertex *new_vertex = new Vertex(dest, weight);
	Vertex *vertex = graph_[src].next_;
	if(vertex == nullptr || vertex->index_ > new_vertex->index_) // Insert at first position.
	{
		graph_[src].next_ = new_vertex;
		new_vertex->next_ = vertex;
	}
	else if(vertex->index_ == new_vertex->index_) // Same as first vertex, update it.
	{
		vertex->weight_ = new_vertex->weight_;
		delete new_vertex;
		return;
	}
	else
	{
		for(Vertex *next_vertex;
		        vertex->index_ < new_vertex->index_;
		        vertex = next_vertex)
		{
			next_vertex = vertex->next_;
			// Insert between: vertex -> new_vertex -> next_vertex.
			if(next_vertex == nullptr || next_vertex->index_ > new_vertex->index_)
			{
				vertex->next_ = new_vertex;
				new_vertex->next_ = next_vertex;
				break;
			}
			else if(next_vertex->index_ == new_vertex->index_) // Update existing vertex.
			{
				next_vertex->weight_ = new_vertex->weight_;
				delete new_vertex;
				return;
			}
		}
	}
	++graph_[src].out_degree_;
	++graph_[dest].in_degree_;
}
void Graph::BFS(int src)
{
	printf("BFS: ");
	::BFS(src, graph_);
	Refresh();
}
void Graph::DFS(int src)
{
	printf("DFS: ");
	::DFS(src, graph_);
	Refresh();
}
void Graph::TopologicalSort()
{
	printf("TLS: ");
	int topo_index[size_], sorted_index = -1;
	for(int index = 0; index < size_; ++index)
	{
		// No need check visited_ because after we record vertex that in_degree is 0,
		// we can't reach it again since its in_degree_ is 0.
		if(graph_[index].in_degree_ == 0)
		{
			topo_index[++sorted_index] = graph_[index].index_;
		}
	}
	int process_index = 0;
	while(sorted_index < size_ - 1)
	{
		for(; process_index <= sorted_index; ++process_index)
		{
			int index = topo_index[process_index];
			for(Vertex *vertex = graph_[index].next_;
			        vertex != nullptr;
			        vertex = vertex->next_)
			{
				--graph_[index].out_degree_;
				if(--graph_[vertex->index_].in_degree_ == 0)
				{
					topo_index[++sorted_index] = graph_[vertex->index_].index_;
				}
			}
		}
	}
	for(int index = 0; index < size_; ++index)
	{
		printf("%d ", topo_index[index]);
	}
	Refresh();
}
void Graph::DijkstraShortestPath(int src)
{
	int pre_index[size_], path_cost[size_ + 1];
	bool flag[size_];
	int flag_true_number = 0;
	const int kMax = 0x7fffffff;
	for(int index = 0; index < size_; ++index)
	{
		pre_index[index] = -1;
		path_cost[index] = kMax;
		flag[index] = false;
	}
	path_cost[size_] = kMax; // Initialize min_cost_index as size_.

	pre_index[src] = src;
	path_cost[src] = 0;
	for(Vertex *vertex = graph_[src].next_; vertex != nullptr; vertex = vertex->next_)
	{
		pre_index[vertex->index_] = src;
		path_cost[vertex->index_] = vertex->weight_;
		flag[vertex->index_] = true; // Insert: O(1)
		++flag_true_number;
	}
	while(flag_true_number > 0)
	{
		int min_cost_index = size_;
		for(int index = 0; index < size_; ++index) // ExtractMin: O(V)
		{
			if(flag[index] == true && path_cost[min_cost_index] > path_cost[index])
			{
				min_cost_index = index;
			}
		}
		flag[min_cost_index] = false;
		--flag_true_number;
		for(Vertex *vertex = graph_[min_cost_index].next_;
		        vertex != nullptr;
		        vertex = vertex->next_)
		{
			if(path_cost[vertex->index_] > path_cost[min_cost_index] + vertex->weight_)
			{
				// DecreaseKey: O(1)
				path_cost[vertex->index_] = path_cost[min_cost_index] + vertex->weight_;
				if(pre_index[vertex->index_] == -1)
				{
					flag[vertex->index_] = true; // Insert: O(1)
					++flag_true_number;
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
void Graph::Refresh()
{
	for(int index = 0; index < size_; ++index)
	{
		graph_[index].visited_ = false;
		graph_[index].in_degree_ = 0;
		graph_[index].out_degree_ = 0;
	}
	for(int index = 0; index < size_; ++index)
	{
		for(Vertex *vertex = graph_[index].next_; vertex != nullptr; vertex = vertex->next_)
		{
			++graph_[index].out_degree_;
			++graph_[vertex->index_].in_degree_;
		}
	}
	printf("\n");
}

int main()
{
	printf("0: Exit\n1: Create\n");
	Graph graph;
	int op;
	while(scanf("%d", &op) == 1)
	{
		switch(op)
		{
		case 0:
			return 0;
		case 1:
			graph.Create();
			break;
		}
		graph.DFS(0);
		graph.BFS(0);
		graph.TopologicalSort();
		graph.DijkstraShortestPath(0);
	}
}
/*
1 5 14
0 1 100
0 3 50
1 2 10
3 2 90
3 4 20
3 1 30
4 2 60
0 1 10
0 3 5
1 2 1
3 2 9
3 4 2
3 1 3
4 2 6
0
DFS: 0 1 2 3 4
BFS: 0 1 3 2 4
TLS: 0 3 1 4 2
(0, 0) cost = 0 edge = 0: 0 -> 0
(0, 1) cost = 8 edge = 2: 0 -> 3 -> 1
(0, 2) cost = 9 edge = 3: 0 -> 3 -> 1 -> 2
(0, 3) cost = 5 edge = 1: 0 -> 3
(0, 4) cost = 7 edge = 2: 0 -> 3 -> 4
*/
