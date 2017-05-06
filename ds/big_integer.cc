#include <string.h>
#include <stdio.h>
#include <utility>

struct BigInteger
{
	BigInteger():
		neg_(false),
		size_(0),
		num_(nullptr)
	{}
	BigInteger(const char *input)
	{
		neg_ = input[0] == '-' ? true : false;
		int input_size = static_cast<int>(strlen(input));
		size_ = neg_ ? input_size - 1 : input_size;
		num_ = new int[size_];
		for(int index = 0; index < size_; ++index)
		{
			num_[index] = input[input_size - 1 - index] - '0';
		}
	}
	BigInteger(bool neg, int size):
		neg_(neg),
		size_(size),
		num_(new int[size_])
	{
		memset(num_, 0, size_ * sizeof(int));
	}
	BigInteger(const BigInteger &rhs):
		neg_(rhs.neg_),
		size_(rhs.size_),
		num_(new int[size_])
	{
		memmove(num_, rhs.num_, size_ * sizeof(int));
	}
	BigInteger(BigInteger &&rhs):
		neg_(rhs.neg_),
		size_(rhs.size_),
		num_(rhs.num_)
	{
		rhs.neg_ = false;
		rhs.size_ = 0;
		rhs.num_ = nullptr;
	}
	BigInteger &operator=(BigInteger rhs)
	{
		Swap(rhs);
		return *this;
	}
	void Swap(BigInteger &rhs)
	{
		std::swap(neg_, rhs.neg_);
		std::swap(size_, rhs.size_);
		std::swap(num_, rhs.num_);
	}
	~BigInteger()
	{
		delete [] num_;
		num_ = nullptr;
	}
	void set_neg(bool neg)
	{
		neg_ = neg;
	}
	void set_size(int new_size)
	{
		size_ = new_size;
		delete [] num_;
		num_ = new int[size_];
		memset(num_, 0, size_ * sizeof(int));
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
			int *more_num = size_ > rhs.size_ ? num_ : rhs.num_;
			for(int index = min_size; index < max_size; ++index)
			{
				result.num_[index] = more_num[index];
			}
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
			result = rhs.Subtract(*this);
			neg_ = true;
		}
		return result;
	}
	BigInteger Subtract(BigInteger &rhs)
	{
		BigInteger result;
		if(neg_ == false && rhs.neg_ == false)
		{
			int comp = CompareAbs(rhs);
			if(comp == 0)
			{
				result.set_size(1);
			}
			else
			{
				result.neg_ = comp > 0 ? false : true;
				int max_size = size_ > rhs.size_ ? size_ : rhs.size_;
				result.set_size(max_size);
				int min_size = size_ < rhs.size_ ? size_ : rhs.size_;
				for(int index = 0; index < min_size; ++index)
				{
					result.num_[index] = comp * (num_[index] - rhs.num_[index]);
				}
				int *more_num = size_ > rhs.size_ ? num_ : rhs.num_;
				for(int index = min_size; index < max_size; ++index)
				{
					result.num_[index] = more_num[index];
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
				for(; result.num_[result.size_ - 1] == 0; --result.size_);
			}
		}
		else if(neg_ == true && rhs.neg_ == true)
		{
			neg_ = rhs.neg_ = false;
			result = rhs.Subtract(*this);
			neg_ = rhs.neg_ = true;
		}
		else if(neg_ == false && rhs.neg_ == true)
		{
			rhs.neg_ = false;
			result = Add(rhs);
			rhs.neg_ = true;
		}
		else
		{
			rhs.neg_ = true;
			result = Add(rhs);
			rhs.neg_ = false;
		}
		return result;
	}
	int CompareAbs(const BigInteger &rhs)
	{
		if(size_ > rhs.size_)
		{
			return 1;
		}
		else if(size_ < rhs.size_)
		{
			return -1;
		}
		for(int index = size_ - 1; index >= 0; --index)
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
	BigInteger Multiple(const BigInteger &rhs)
	{
		BigInteger result;
		if((neg_ == true && rhs.neg_ == false) || (neg_ == false && rhs.neg_ == true))
		{
			result.set_neg(true);
		}
		result.set_size(size_ > rhs.size_ ? size_ * 2 + 1 : rhs.size_ * 2 + 1);
		for(int index1 = 0; index1 < size_; ++index1)
		{
			for(int index2 = 0; index2 < rhs.size_; ++index2)
			{
				result.num_[index1 + index2] += num_[index1] * rhs.num_[index2];
			}
		}
		for(int index = 0; index < result.size_; ++index)
		{
			result.num_[index + 1] += result.num_[index] / 10;
			result.num_[index] %= 10;
		}
		for(; result.num_[result.size_ - 1] == 0; --result.size_);
		return result;
	}
	std::pair<BigInteger, BigInteger> DivideAndMod(const BigInteger &rhs)
	{
		BigInteger division, mod;
		if((neg_ == true && rhs.neg_ == false) || (neg_ == false && rhs.neg_ == true))
		{
			division.set_neg(true);
		}
		int comp = CompareAbs(rhs);
		if(comp <= 0)
		{
			division.set_size(1);
			if(comp == -1)
			{
				division.set_neg(false); // -0 -> 0
				mod = *this;
			}
			else
			{
				division.num_[0] = 1;
				mod.set_size(1);
			}
		}
		else
		{
			int size_diff = size_ - rhs.size_;
			division.set_size(size_diff + 1); // At most x-y+1 digits.
			BigInteger temp_lhs(*this);
			temp_lhs.set_neg(false);
			for(int offset = size_diff; offset >= 0; --offset)
			{
				BigInteger temp_rhs(false, rhs.size_ + offset);
				memmove(temp_rhs.num_ + offset, rhs.num_, rhs.size_ * sizeof(int));
				for(;;)
				{
					BigInteger sub_result = temp_lhs.Subtract(temp_rhs);
					if(sub_result.neg_ == false)
					{
						++division.num_[offset];
						temp_lhs = sub_result;
					}
					else
					{
						break;
					}
				}
			}
			mod = temp_lhs;
			mod.set_neg(neg_);
			if(division.num_[division.size_ - 1] == 0)
			{
				--division.size_;
			}
		}
		return std::pair<BigInteger, BigInteger>(division, mod);
	}

	bool neg_;
	int size_;
	int *num_;
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
		(bi1.Add(bi2)).ShowContent();
		(bi1.Subtract(bi2)).ShowContent();
		(bi1.Multiple(bi2)).ShowContent();
		std::pair<BigInteger, BigInteger> bi_pair = bi1.DivideAndMod(bi2);
		bi_pair.first.ShowContent();
		bi_pair.second.ShowContent();
		printf("\n");
	}
}
/*
512 512
-512 -512
512 408
408 512
992 38
38 992
908 38
38 908
-38 -992
************************
1024 0 262144 1 0
-1024 0 262144 1 0
920 104 208896 1 104
920 -104 208896 0 408
1030 954 37696 26 4
1030 -954 37696 0 38
946 870 34504 23 34
946 -870 34504 0 38
-1030 954 37696 0 -38
*/
