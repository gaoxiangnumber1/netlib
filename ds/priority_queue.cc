#include "priority_queue.h"

int main()
{
	printf("0: Exit\n1: Create\n2: ExtractMin\n");
	int op;
	PriorityQueue<int, int> priority_queue;
	while(scanf("%d", &op) == 1)
	{
		switch(op)
		{
		case 0:
			return 0;
		case 1:
			priority_queue.Create();
			break;
		case 2:
			priority_queue.ExtractMin();
			priority_queue.ShowContent();
			break;
		}
	}
}
/*
1 7 7 7 6 6 5 5 4 4 3 3 2 2 1 1
2
2
2
2
2
2
2
0
<1, 1> <4, 4> <2, 2> <7, 7> <5, 5> <6, 6> <3, 3>
<2, 2> <4, 4> <3, 3> <7, 7> <5, 5> <6, 6> <1, 1>
<3, 3> <4, 4> <6, 6> <7, 7> <5, 5> <2, 2> <1, 1>
<4, 4> <5, 5> <6, 6> <7, 7> <3, 3> <2, 2> <1, 1>
<5, 5> <7, 7> <6, 6> <4, 4> <3, 3> <2, 2> <1, 1>
<6, 6> <7, 7> <5, 5> <4, 4> <3, 3> <2, 2> <1, 1>
<7, 7> <6, 6> <5, 5> <4, 4> <3, 3> <2, 2> <1, 1>
<7, 7> <6, 6> <5, 5> <4, 4> <3, 3> <2, 2> <1, 1>
*/
