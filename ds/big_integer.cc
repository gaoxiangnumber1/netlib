#include <string.h>
#include <stdio.h>

struct BigInteger
{
public:
	BigInteger(): is_neg_(false), size_(0)
	{
		memset(num_, 0, sizeof num_);
	}
	void CreateFromInput()
	{
		char input[kBigIntegerSize];
		scanf("%s", input);
		if(input[0] == '-')
		{
			is_neg_ = true;
		}
		int input_size = static_cast<int>(strlen(input));
		size_ = is_neg_ ? input_size - 1 : input_size;
		for(int index = 0; index < size_; ++index)
		{
			num_[index] = input[input_size - 1 - index] - '0';
		}
		//ShowContent();
	}
	void Add(BigInteger rhs)
	{
		int max_size = size_ > rhs.size_ ? size_ : rhs.size_;
		BigInteger result;
		if((is_neg_ == true && rhs.is_neg_ == true) ||
		        (is_neg_ == false && rhs.is_neg_ == false))
		{
			int carry = 0;
			for(int index = 0; index <= max_size; ++index) // `<=`: 09 + 09 = 18
			{
				int value = num_[index] + rhs.num_[index] + carry;
				carry = value / 10;
				value %= 10;
				result.num_[index] = value;
			}
			result.is_neg_ = is_neg_;
			result.size_  = result.num_[max_size] == 0 ? max_size : max_size + 1;
			result.ShowContent();
		}
	}
	void Subtract(BigInteger rhs)
	{
先把大正数减小正数实现
	}
	void ShowContent()
	{
		if(is_neg_ == true)
		{
			printf("-");
		}
		for(int index = size_ - 1; index >= 0; --index)
		{
			printf("%d", num_[index]);
		}
		printf("\n");
	}

	static const int kBigIntegerSize = 128;
	int num_[kBigIntegerSize];
	bool is_neg_;
	int size_;
};

int main()
{
	while(true)
	{
		BigInteger bi1, bi2;
		bi1.CreateFromInput();
		bi2.CreateFromInput();
		bi1.Add(bi2);
	}
}
