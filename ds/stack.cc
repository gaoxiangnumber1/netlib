#include "stack.h"

int main()
{
	Stack<int> object;
	printf("0: Exit\n1: Create\n2: Push\n3: Pop\n4: Top\n");
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
			object.Push(data);
			object.ShowContent();
			break;
		case 3:
			object.Pop();
			object.ShowContent();
			break;
		case 4:
			printf("Top = %d\n", object.Top());
			object.ShowContent();
		}
	}
}
/*
1 5 1 2 3 4 5
2 6 2 7
3 3 3 3 3 3 3 3 3 3
4
2 0
4
0
Input: data_number data_content
05 data: 5 4 3 2 1
06 data: 6 5 4 3 2 1
07 data: 7 6 5 4 3 2 1
06 data: 6 5 4 3 2 1
05 data: 5 4 3 2 1
04 data: 4 3 2 1
03 data: 3 2 1
02 data: 2 1
01 data: 1
00 data:
00 data:
00 data:
00 data:
Top = 0
00 data:
01 data: 0
Top = 0
01 data: 0
*/
