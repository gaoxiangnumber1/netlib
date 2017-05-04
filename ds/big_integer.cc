#include <string.h>
#include <stdio.h>

// TODO: 1. Save memory: use char? dynamic allocate array?
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
	void Add(BigInteger &rhs)
	{
		if((is_neg_ == true && rhs.is_neg_ == true) ||
		        (is_neg_ == false && rhs.is_neg_ == false))
		{
			int max_size = size_ > rhs.size_ ? size_ : rhs.size_;
			BigInteger result;
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
		else if(is_neg_ == false && rhs.is_neg_ == true)
		{
			rhs.is_neg_ = false;
			Subtract(rhs);
			rhs.is_neg_ = true;
		}
		else
		{
			is_neg_ = false;
			rhs.Subtract(*this);
			is_neg_ = true;
		}
	}
	void Subtract(BigInteger &rhs)
	{
		if(is_neg_ == false && rhs.is_neg_ == false)
		{
			int comp = Compare(rhs);
			BigInteger result;
			if(comp == 0)
			{
				result.size_ = 1;
			}
			else
			{
				int max_size = size_ > rhs.size_ ? size_ : rhs.size_;
				for(int index = 0; index < max_size; ++index)
				{
					result.num_[index] = comp * (num_[index] - rhs.num_[index]);
				}
				for(int low = 0; low < max_size; ++low)
				{
					if(result.num_[low] < 0)
					{
						int high = low;
						while(result.num_[++high] <= 0);
						--result.num_[high];
						while(--high != low)
						{
							result.num_[high] += 9;
						}
						result.num_[low] += 10;
					}
				}
				result.is_neg_ = comp > 0 ? false : true;
				for(result.size_ = max_size; result.num_[result.size_ - 1] == 0; --result.size_);
			}
			result.ShowContent();
		}
		else if(is_neg_ == true && rhs.is_neg_ == true)
		{
			is_neg_ = rhs.is_neg_ = false;
			rhs.Subtract(*this);
			is_neg_ = rhs.is_neg_ = true;
		}
		else if(is_neg_ == false && rhs.is_neg_ == true)
		{
			rhs.is_neg_ = false;
			Add(rhs);
			rhs.is_neg_ = true;
		}
		else
		{
			rhs.is_neg_ = true;
			Add(rhs);
			rhs.is_neg_ = false;
		}
	}
	int Compare(const BigInteger &rhs)
	{
		int max_size = size_ > rhs.size_ ? size_ : rhs.size_;
		for(int index = max_size - 1; index >= 0; --index)
		{
			if(num_[index] > rhs.num_[index])
			{
				return 1;
			}
			else if(num_[index] < rhs.num_[index])
			{
				return -1;
			}
		}
		return 0;
	}
	void Multiple(const BigInteger &rhs)
	{
		int max_size = size_ > rhs.size_ ? size_ : rhs.size_;
		int result_max_size = max_size * 2 + 1;
		if(result_max_size > kBigIntegerSize)
		{
			printf("Overflow\n");
			return;
		}
		BigInteger result;
		for(int index1 = 0; index1 < max_size; ++index1)
		{
			for(int index2 = 0; index2 < max_size; ++index2)
			{
				result.num_[index1 + index2] += num_[index1] * rhs.num_[index2];
			}
		}
		for(int index = 0; index < result_max_size; ++index)
		{
			result.num_[index + 1] += result.num_[index] / 10;
			result.num_[index] %= 10;
		}
		if((is_neg_ == true && rhs.is_neg_ == false) ||
		        (is_neg_ == false && rhs.is_neg_ == true))
		{
			result.is_neg_ = true;
		}
		for(result.size_ = result_max_size; result.num_[result.size_ - 1] == 0; --result.size_);
		result.ShowContent();
	}
	void Divide(const BigInteger &rhs)
	{
		int comp = Compare(rhs);
		BigInteger result;
		if(comp > 0)
		{
		}
		else
		{
			result.num_[0] = comp == 0 ? 1 : 0;
			result.size_ = 1;
		}
		if((is_neg_ == true && rhs.is_neg_ == false) ||
		        (is_neg_ == false && rhs.is_neg_ == true))
		{
			result.is_neg_ = true;
		}
		result.ShowContent();
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
		bi1.Subtract(bi2);
		bi1.Multiple(bi2);
		bi1.Divide(bi2);
	}
}
/*
512
512
-512
-512
512
480
480
512
-512
-480
512
-480
-512
480
/////////////
1024
0
262144
-1024
0
262144
992
32
245760
992
-32
245760
-992
-32
245760
32
992
-245760
-32
-992
-245760
*/
