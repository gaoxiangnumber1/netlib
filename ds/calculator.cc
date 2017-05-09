#include <stack>
#include <string>
#include <string.h>
using namespace std;

bool IsOp(const char ch)
{
	return ch == '+' || ch == '-' || ch == '*' || ch == '/';
}
bool IsEqualOrGreat(char ch1, char ch2)
{
	if(ch1 == '+' || ch1 == '-')
	{
		if(ch2 == '+' || ch2 == '-')
		{
			return true;
		}
		return false;
	}
	return true;
}

int main()
{
	const int kInputSize = 1024;
	char input[kInputSize];
	while(scanf("%s", input) == 1)
	{
		stack<char> op;
		stack<string> prefix;
		int input_size = static_cast<int>(strlen(input));
		for(int index = input_size - 1; index >= 0; )
		{
			if(isdigit(input[index]))
			{
				int begin_index = index - 1;
				for(; begin_index >= 0 && isdigit(input[begin_index]); --begin_index);
				if(begin_index < 0)
				{
					prefix.push(string(input, index + 1));
					break;
				}
				else if(begin_index == 0)
				{
					if(input[0] != '-')
					{
						prefix.push(string(input + 1, index));
						index = 0;
					}
					else
					{
						prefix.push(string(input, index + 1));
						break;
					}
				}
				else
				{
					if(IsOp(input[begin_index - 1]) && input[begin_index] == '-')
					{
						prefix.push(string(input + begin_index, index - begin_index + 1));
						index = begin_index - 1;
					}
					else
					{
						prefix.push(string(input + begin_index + 1, index - begin_index));
						index = begin_index;
					}
				}
			}
			else if(IsOp(input[index]))
			{
				if(op.empty() == true || op.top() == ')' || IsEqualOrGreat(input[index], op.top()))
				{
					op.push(input[index]);
					--index;
				}
				else
				{
					prefix.push(string(&op.top(), 1));
					op.pop();
					continue;
				}
			}
			else
			{
				if(input[index] == ')')
				{
					op.push(input[index]);
				}
				else
				{
					while(op.top() != ')')
					{
						prefix.push(string(&op.top(), 1));
						op.pop();
					}
					op.pop();
				}
				--index;
			}
		}
		while(op.empty() == false)
		{
			prefix.push(string(&op.top(), 1));
			op.pop();
		}
		while(prefix.empty() == false)
		{
			printf("%s", prefix.top().data());
			prefix.pop();
		}
		printf("\n");
	}
}
