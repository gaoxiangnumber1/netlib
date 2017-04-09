#include <stdio.h>
#include <string.h> // str*()
#include <vector>
#include <utility> // swap()
#include <queue>

// Review: ~String()

class String
{
public:
	// Default ctor
	String(): data_(new char[1]), length_(0)
	{
		*data_ = '\0';
		printf("Default_ctor    ");
	}
	String(const char *data, size_t length):
		data_(new char[length + 1]),
		length_(length)
	{
		memcpy(data_, data, length_);
		data_[length] = '\0';
		printf("Const_char_*length_ctor    ");
	}
	String(const char *data): String(data, strlen(data)) // Delegating ctor
	{
		printf("Const_char_*ctor    ");
	}
	// Copy ctor
	String(const String &rhs): String(rhs.data_, rhs.length_)
	{
		printf("Copy_ctor    ");
	}
	// Move ctor
	String(String &&rhs): data_(rhs.data_), length_(rhs.length_)
	{
		rhs.data_ = nullptr;
		rhs.length_ = 0;
		printf("Move_ctor    ");
	}

	// Non-throwing swap
	void Swap(String &rhs) noexcept
	{
		std::swap(data_, rhs.data_);
		std::swap(length_, rhs.length_);
		printf("Swap    ");
	}
	// Unifying assignment operator: no need to write copy/move assignment operator.
	String &operator=(String rhs) // Copy and Swap idiom.
	{
		Swap(rhs);
		printf("Unifying-AO    ");
		return *this;
	}
	/*
	// Traditional copy assignment operator
	String &operator=(const String &rhs)
	{
		if(this != &rhs) // If not self assignment.
		{
			printf("Enter if    ");
			String temp(rhs); // RAII acquire new resource.
			Swap(temp); // Non-throwing swap.
		}
		// RAII auto release old resource.
		printf("Copy-AO    ");
		return *this;
	}
	// Move-assignment operator
	String &operator=(String &&rhs)
	{
		Swap(rhs);
		printf("Move_AO    ");
		return *this;
	}
	*/

	// Dtor
	~String()
	{
		delete [] data_;
		printf("Dtor    ");
	}

	size_t Size() const
	{
		return length_;
	}
	const char *c_str() const
	{
		return data_;
	}

private:
	char *data_;
	size_t length_; // long unsigned int
};
namespace std
{
template<>
void swap(String &lhs, String &rhs) noexcept
{
	lhs.Swap(rhs);
}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PassByValue(String s)
{
	printf("passed = %s    ", s.c_str());
}
void PassByConstReference(const String &s)
{
	printf("\npassed = %s\n", s.c_str());
}
String ReturnByValue(const char *data)
{
	String ret(data);
	printf("ret = %s\t", ret.c_str());
	return ret;
}

int main()
{
//	Test copy-elision between (String) and (const String&)
	String s;
	s = ReturnByValue("gao");
	printf("%s\n", s.c_str());

	printf("----------Test Ctor----------\n");
	String s0;
	printf("s0 = %s, s0.Size() = %d\n", s0.c_str(), static_cast<int>(s0.Size()));
	String s1("gao");
	printf("s1 = %s\n", s1.c_str());
	String s2 = "xiang"; // Equivalent to `String s3("xiang");`
	printf("s2 = %s\n", s2.c_str());
	String s3(s1);
	printf("s3 = %s\n", s3.c_str());
	String s4(std::move(String("number1")));
	printf("s4 = %s\n", s4.c_str());

	printf("----------Test Copy/Move Assignment Operator----------\n");
	// The following two assignment are errors if define move ctor/assignment_operator.
	// error: use of deleted function ‘String& String::operator=(const String&)’
	// note: ‘String& String::operator=(const String&)’ is implicitly declared as deleted
	// because ‘String’ declares a move constructor or move assignment operator
	s2 = s4;
	printf("s2 = %s\n", s2.c_str());
	s2 = s2;
	printf("s2 = %s\n", s2.c_str());
	s2 = std::move(s1);
	printf("s2 = %s, s1 = %s\n", s2.c_str(), s1.c_str());
	s2 = std::move(s2);
	printf("s2 = %s\n", s2.c_str());
	s1 = "xiang";
	printf("s1 = %s\n", s1.c_str());

	printf("----------Test as Parameter/Return type----------\n");
	PassByValue(s1);
	PassByConstReference(s1);
	String s5(ReturnByValue("hello")); // The returned temporary object is moved to s5.
	printf("s5 = %s\n", s5.c_str());
	s5 = ReturnByValue("world");
	printf("s5 = %s\n", s5.c_str());

	printf("----------Test as value_type of STL container----------\n");
	std::vector<String> vec;
	vec.push_back(s1);
	printf("vec[0] = %s\n", vec[0].c_str());
	vec.push_back(s2);
	printf("vec[0] = %s\tvec[1] = %s\n", vec[0].c_str(), vec[1].c_str());
	vec.push_back("gaoxiangnumber1");
	printf("vec[0] = %s\tvec[1] = %s\tvec[2] = %s\n", vec[0].c_str(), vec[1].c_str(), vec[2].c_str());

	printf("----------All Test Passed!----------\n");
}
