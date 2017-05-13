#include <stack>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
using namespace std;

int StringToInt(const char *str, int length)
{
	char buf[length + 1];
	memmove(buf, str, length);
	buf[length] = 0;
	return atoi(buf);
}
void CalculateInfix(const char *infix)
{
	int length = static_cast<int>(strlen(infix));
	stack<int> num;
	for(int index = 0; index < length; )
	{
		if(isdigit(infix[index]) || // Non-negative number
		        (infix[index] == '(' && infix[index + 1] == '-')) // Negative number
		{
			index = infix[index] == '(' ? index + 1 : index;
			int last = index + 1;
			for(; last < length && isdigit(infix[last]); ++last);
			num.push(StringToInt(infix + index, last - index));
			index = infix[last] == ')' ? last + 1 : last;
			printf("%d\n", num.top());
		}
		else
		{
			++index;
		}
	}
}

int main()
{
	const int kInputSize = 1024;
	char infix[kInputSize];
	while(scanf("%s", infix) == 1)
	{
		CalculateInfix(infix);
	}
}
/*
(-23)+(4+56)*78+(-5)+6*(-78)+123#
*************************************
4307
*/
/*
bool IsOp(const char ch)
{
	return ch == '+' || ch == '-' || ch == '*' || ch == '/';
}
bool IsOp(const string &str)
{
	return str == "+" || str == "-" || str == "*" || str == "/";
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
void CalculatePrefix(stack<string> &prefix)
{
	stack<int> cal;
	while(prefix.empty() == false)
	{
		string str = prefix.top();
		prefix.pop();
		if(IsOp(str) == false)
		{
			cal.push(atoi(str.data()));
		}
		else
		{
			int lhs = cal.top();
			cal.pop();
			int rhs = cal.top();
			cal.pop();
			switch(*str.data())
			{
			case '+':
				lhs += rhs;
				break;
			case '-':
				lhs -= rhs;
				break;
			case '*':
				lhs *= rhs;
				break;
			case '/':
				lhs /= rhs;
			}
			cal.push(lhs);
		}
	}
	printf("\nResult is: %d\n", cal.top());
}
void InfixToPrefix(const char *infix)
{
	stack<char> op;
	stack<string> prefix;
	int input_size = static_cast<int>(strlen(infix));
	for(int index = input_size - 1; index >= 0; )
	{
		if(isdigit(infix[index]))
		{
			int begin_index = index - 1;
			for(; begin_index >= 0 && isdigit(infix[begin_index]); --begin_index);
			if(begin_index < 0)
			{
				prefix.push(string(infix, index + 1));
				break;
			}
			else if(begin_index == 0)
			{
				if(infix[0] != '-')
				{
					prefix.push(string(infix + 1, index));
					index = 0;
				}
				else
				{
					prefix.push(string(infix, index + 1));
					break;
				}
			}
			else
			{
				if(IsOp(infix[begin_index - 1]) && infix[begin_index] == '-')
				{
					prefix.push(string(infix + begin_index, index - begin_index + 1));
					index = begin_index - 1;
				}
				else
				{
					prefix.push(string(infix + begin_index + 1, index - begin_index));
					index = begin_index;
				}
			}
		}
		else if(IsOp(infix[index]))
		{
			if(op.empty() == true || op.top() == ')' || IsEqualOrGreat(infix[index], op.top()))
			{
				op.push(infix[index]);
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
			if(infix[index] == ')')
			{
				op.push(infix[index]);
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
	stack<string> temp;
	printf("Prefix is: ");
	while(prefix.empty() == false)
	{
		temp.push(prefix.top());
		printf("%s,", prefix.top().data());
		prefix.pop();
	}
	CalculatePrefix(temp);
}
*/
//
