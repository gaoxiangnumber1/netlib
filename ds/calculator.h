/*
*任务：使用栈实现表达式求值
*算法思想：判断运算符的优先级并执行相应的操作
*功能：
1.对负数、浮点数均有效
2.可进行多括号的复杂运算

*主要函数：
*   1.void InitNumberStack(Number &NumberStack);//初始化数字栈
*   2.void InitOperatorStack(Operator &OperatorStack);//初始化运算符栈
*   3.void NumberPush(Number &NumberStack,double number);//压入数字栈
*   4.void NumberPop(Number &NumberStack);//弹出数字栈
*   5.void OperatorPush(Operator &OperatorStack,char op);//压入运算符栈
*   6.void OperatorPop(Operator &OperatorStack);//弹出运算符栈
*   7.char GetTopOperator(Operator &OperatorStack);//得到运算符栈栈顶元素
*   8.double GetTopNumber(Number &NumberStack);//得到数字栈栈顶元素
*   9.double ToNumber(string s);//字符串转换成数字函数
*   10.char Precede(char stacktop,char willpush);//判断运算符的优先级
*   11.double calculate(double a,double b,char op);//计算
*   12.int IsDigit(char ch);//判断字符是否为数字

*运行示例：
请输入运算表达式（表达时应当合法且不含空格且负数必须使用括号括起来且以‘#’结尾，仅输入‘#’表示退出）：
1+2-3+4-5#
运算结果为：-1
请输入运算表达式（表达时应当合法且不含空格且负数必须使用括号括起来且以‘#’结尾，仅输入‘#’表示退出）：
1.23+3.45-6.78+9.01#
运算结果为：6.91
请输入运算表达式（表达时应当合法且不含空格且负数必须使用括号括起来且以‘#’结尾，仅输入‘#’表示退出）：
1+2*3+4/5-6#
运算结果为：1.8
请输入运算表达式（表达时应当合法且不含空格且负数必须使用括号括起来且以‘#’结尾，仅输入‘#’表示退出）：
1+((-3)*3+7.2*5/9)/((-4)*6+25-7/2)+23.9#
运算结果为：26.9
请输入运算表达式（表达时应当合法且不含空格且负数必须使用括号括起来且以‘#’结尾，仅输入‘#’表示退出）：
#

*/
#define MAXSIZE 100
typedef struct
{
	double *top;
	double *base;
} Number; //数字栈类型
typedef struct
{
	char *top;
	char *base;
} Operator;//运算符栈类型
double ToNumber(string s)//字符串转换成数字函数
{
	string s1;//去掉括号的部分
	if(s[0]=='(')
	{
		s1=s.substr(2,s.size()-3);
	}
	else
	{
		s1=s;
	}

	double num=0.0;//返回值
	int i;
	for(i=0; i<s1.size(); i++)
	{
		if(s1[i]=='.')//整数部分转换完成
		{
			break;
		}
		num=num*10+s1[i]-'0';
	}

	//转换小数部分
	double j=0.1;
	for(i=i+1; i<s1.size(); i++)
	{
		num=num+(s1[i]-'0')*j;
		j=j*0.1;
	}

	if(s1==s)//非负数
	{
		return num;
	}
	return -1*num;//负数添负号再返回
}
char Precede(char stacktop,char willpush)//判断运算符的优先级
{
	switch(stacktop)
	{
	case '#':
		if(willpush=='#')
		{
			return '=';
		}
		return '<';

	case '+':
	case '-':
		if(willpush=='+'||willpush=='-'||willpush==')'||willpush=='#')
		{
			return '>';
		}
		return '<';

	case '*':
	case '/':
		if(willpush=='(')
		{
			return '<';
		}
		return '>';

	case '(':
		if(willpush==')')
		{
			return '=';
		}
		return '<';

	case ')':
		return '>';
	}
}
double calculate(double a,double b,char op)//计算
{
	switch(op)
	{
	case '+':
		return a+b;
	case '-':
		return a-b;
	case '*':
		return a*b;
	case '/':
		return a/b;
	}
}
void InitNumberStack(Number &NumberStack)//初始化数字栈
{
	NumberStack.base=(double*)malloc(MAXSIZE*sizeof(double));
	NumberStack.top=NumberStack.base;
}
void InitOperatorStack(Operator &OperatorStack)//初始化运算符栈
{
	OperatorStack.base=(char*)malloc(MAXSIZE*sizeof(char));
	OperatorStack.top=OperatorStack.base;
}
void NumberPush(Number &NumberStack,double number)//压入数字栈
{
	*NumberStack.top=number;
	NumberStack.top++;
}
void  NumberPop(Number &NumberStack)//弹出数字栈
{
	NumberStack.top--;
}
void OperatorPush(Operator &OperatorStack,char op)//压入运算符栈
{
	*OperatorStack.top=op;
	OperatorStack.top++;
}
void OperatorPop(Operator &OperatorStack)//弹出运算符栈
{
	OperatorStack.top--;
}
char GetTopOperator(Operator &OperatorStack)//得到运算符栈栈顶元素
{
	return *(OperatorStack.top-1);
}
double GetTopNumber(Number &NumberStack)//得到数字栈栈顶元素
{
	return *(NumberStack.top-1);
}
int IsDigit(char ch)//判断字符是否为数字
{
	if(ch>='0'&&ch<='9')
	{
		return 1;
	}
	return 0;
}

int main()
{
	string str;
	cout<<"请输入运算表达式（表达时应当合法且不含空格且负数必须使用括号括起来且以‘#’结尾，仅输入‘#’表示退出）：\n";

	while(getline(cin,str)&&str!="#")
	{
		Operator OperatorStack;
		Number NumberStack;

		InitNumberStack(NumberStack);//初始化两个栈
		InitOperatorStack(OperatorStack);

		OperatorPush(OperatorStack,'#');//运算符栈中预存一个‘#’便于判断运算符的优先级

		for(int i=0; i<static_cast<int>(str.size()); i++)//遍历表达式
		{
			if(IsDigit(str[i]))//若当前字符为数字，将该数字全部截取出来
			{
				int j=i;//数字字符串的起点索引

				while(i<str.size()&&(IsDigit(str[i])||str[i]=='.'))
					//当不越界且字符为数字或小数点时，数字字符串未结束
				{
					i++;
				}
				//利用转换函数将截取的数字字符串转换为数字并压入数字栈中
				NumberPush(NumberStack,ToNumber(str.substr(j,i-j)));
				i--;//索引减1
			}
			else if(str[i]=='('&&str[i+1]=='-')//若遇到负数
			{
				int j=i;

				i++;//细节
				while(i<str.size()&&(IsDigit(str[i])||str[i]=='.'||str[i]=='-'||str[i]==')'))
					//注意连同负数的左右括号一同截取，防止对后续的判断造成影响
				{
					i++;
				}
				NumberPush(NumberStack,ToNumber(str.substr(j,i-j)));
				i--;
			}
			else//不是数字
			{
				switch(Precede(GetTopOperator(OperatorStack),str[i]))
					//Precede函数判断运算符栈栈顶运算符和将要压入的运算符的优先级，以此来决定操作
				{
				case '<'://栈顶运算符优先级低
					OperatorPush(OperatorStack,str[i]);//压入优先级高的运算符
					break;
				case '='://优先级相同：表明括号内运算完成
					OperatorPop(OperatorStack);//弹出栈顶的括号
					break;
				case '>'://栈顶运算符优先级高：执行栈顶运算符代表的运算
					char op=GetTopOperator(OperatorStack);//得到栈顶运算符
					OperatorPop(OperatorStack);//删除运算符栈栈顶运算符

					double b=GetTopNumber(NumberStack);//得到第二个运算数字，注意是后进先出
					NumberPop(NumberStack);//删除数字栈栈顶数字
					double a=GetTopNumber(NumberStack);//得到第一个运算数字
					NumberPop(NumberStack);//删除数字栈栈顶数字
					NumberPush(NumberStack,calculate(a,b,op));//执行运算并将结果压入数字栈

					i--;//推迟压入运算符栈中，避免运算符栈内出现能运算但未运算的运算符
					break;
				}
			}
		}

		cout<<"运算结果为："<<GetTopNumber(NumberStack)<<endl;
		cout<<"请输入运算表达式（表达时应当合法且不含空格且负数必须使用括号括起来且以‘#’结尾，仅输入‘#’表示退出）：\n";
	}
	return 0;
}
