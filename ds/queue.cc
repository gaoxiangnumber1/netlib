#include "queue.h"

int main()
{
	Queue<int> object;
	printf("0: Exit\n1: Create\n2: Enqueue\n3: Dequeue\n");
	int operation, data;
	while(scanf("%d", &operation) == 1)
	{
		switch(operation)
		{
		case 0:
			return 0;
		case 1:
			object.Create();
			object.ShowContent();
			break;
		case 2:
			scanf("%d", &data);
			object.Enqueue(data);
			object.ShowContent();
			break;
		case 3:
			object.Dequeue();
			object.ShowContent();
		}
	}
}
/*
1 5 1 2 3 4 5
2 6 2 7
3 3 3 3 3 3 3 3
2 0
3
0
Input: data_number data_content
05 data: 1 2 3 4 5
06 data: 1 2 3 4 5 6
07 data: 1 2 3 4 5 6 7
06 data: 2 3 4 5 6 7
05 data: 3 4 5 6 7
04 data: 4 5 6 7
03 data: 5 6 7
02 data: 6 7
01 data: 7
00 data:
00 data:
01 data: 0
00 data:
*/
