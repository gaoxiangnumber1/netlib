#include <string.h>
#include <stdio.h>

struct BigInteger
{
public:
	BigInteger(): neg_(false), size_(0), num_(nullptr) {}
	BigInteger(const char *input)
	{
		if(input[0] == '-')
		{
			neg_ = true;
		}
		int input_size = static_cast<int>(strlen(input));
		size_ = neg_ ? input_size - 1 : input_size;
		num_ = new short[size_];
		for(int index = 0; index < size_; ++index)
		{
			num_[index] = static_cast<short>(input[input_size - 1 - index] - '0');
		}
		ShowContent();
	}
	BigInteger &operator=(BigInteger rhs)
	{
		neg_ = rhs.neg_;
		size_ = rhs.size_;
		delete [] num_;
		num_ = new short[size_];
		memmove(num_, rhs.num_, size_ * sizeof(short));
	}
	~BigInteger()
	{
		delete [] num_;
	}
	void set_neg(bool neg)
	{
		neg_ = neg;
	}
	void set_size(int new_size)
	{
		size_ = new_size;
		delete [] num_;
		num_ = new short[new_size];
	}
	void ShowContent()
	{
		if(neg_ == true)
		{
			printf("-");
		}
		for(int index = size_ - 1; index >= 0; --index)
		{
			printf("%d", num_[index]);
		}
		printf("\n");
	}

	BigInteger Add(BigInteger &rhs)
	{
		BigInteger result;
		if((neg_ == true && rhs.neg_ == true) ||
		        (neg_ == false && rhs.neg_ == false))
		{
			int max_size = size_ > rhs.size_ ? size_ : rhs.size_;
			result.set_neg(neg_);
			result.set_size(max_size + 1);
			int min_size = size_ < rhs.size_ ? size_ : rhs.size_;
			for(int index = 0; index < min_size; ++index)
			{
				result.num_[index] = num_[index] + rhs.num_[index];
			}
			short *more_num = size_ > rhs.size_ ? num_ : rhs.num_;
			for(int index = min_size; index < max_size; ++index);
			{
				result.num_[index] = more_num[index];
			}
			result.num_[max_size] = 0;
			for(int index = 0; index < max_size; ++index)
			{
				result.num_[index + 1] += result.num_[index] / 10;
				result.num_[index] %= 10;
			}
			if(result.num_[max_size] == 0)
			{
				--result.size_;
			}
		}
		else if(neg_ == false && rhs.neg_ == true)
		{
			rhs.neg_ = false;
			result = Subtract(rhs);
			rhs.neg_ = true;
		}
		else
		{
			neg_ = false;
			result = Subtract(*this);
			neg_ = true;
		}
		return result;
	}
	BigInteger Subtract(BigInteger &rhs)
	{
		if(neg_ == false && rhs.neg_ == false)
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
				result.neg_ = comp > 0 ? false : true;
				for(result.size_ = max_size; result.num_[result.size_ - 1] == 0; --result.size_);
			}
			result.ShowContent();
		}
		else if(neg_ == true && rhs.neg_ == true)
		{
			neg_ = rhs.neg_ = false;
			rhs.Subtract(*this);
			neg_ = rhs.neg_ = true;
		}
		else if(neg_ == false && rhs.neg_ == true)
		{
			rhs.neg_ = false;
			Add(rhs);
			rhs.neg_ = true;
		}
		else
		{
			rhs.neg_ = true;
			Add(rhs);
			rhs.neg_ = false;
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
	/*
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
		if((neg_ == true && rhs.neg_ == false) ||
		        (neg_ == false && rhs.neg_ == true))
		{
			result.neg_ = true;
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
		if((neg_ == true && rhs.neg_ == false) ||
		        (neg_ == false && rhs.neg_ == true))
		{
			result.neg_ = true;
		}
		result.ShowContent();
	}
	*/

	bool neg_;
	int size_;
	short *num_;
};

int main()
{
	const int kBufferSize = 64 * 1024; // 64kB
	char buffer[kBufferSize];
	while(scanf("%s", buffer) == 1)
	{
		BigInteger bi1(buffer);
		scanf("%s", buffer);
		BigInteger bi2(buffer);
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
