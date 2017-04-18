#include <stdio.h>
#include <map>
#include <stdint.h>
using namespace std;

int main()
{
	uint64_t uid;
	map<uint64_t, int> m;
	int cnt = 0;
	while(scanf("%lu", &uid) == 1)
	{
		if(uid == 0)
		{
			printf("%d\n", cnt);
			return 0;
		}
		if(++m[uid] == 1)
		{
			++cnt;
		}
	}
}
set:TLE 70%
map:内存超限 ( Memory Limit Exceeded (MLE) )
说明：
所有测试数据正确率为 10%！
自己实现hash，用链表连起来
