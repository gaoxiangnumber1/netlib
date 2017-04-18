#include <stdio.h>
#include <set>
#include <stdint.h>
using namespace std;

int main()
{
	uint64_t uid;
	set<uint64_t> s;
	while(scanf("%lu", &uid) == 1)
	{
		if(uid == 0)
		{
			printf("%lu\n", s.size());
			return 0;
		}
		s.insert(uid);
	}
}
