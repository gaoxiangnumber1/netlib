#ifndef NETLIB_SRC_STRING_PIECE_H_
#define NETLIB_SRC_STRING_PIECE_H_

// A string like object that points into another piece of memory.
// Useful for providing an interface that allows clients to easily
// pass in either a "const char*" or a "string".

#include <types.h>
#ifndef NETLIB_STD_STRING
#include <string>
#endif

namespace netlib
{

// Used for passing c-style string argument to a function.
class StringArg: public Copyable
{
public:
	StringArg(const char *str): string_(str) {}
	StringArg(const string &str): string_(str.c_str()) {}
#ifndef NETLIB_STD_STING
	StringArg(const std::string &str): string_(str.c_str()) {}
#endif

	const char *GetCString() const
	{
		return string_;
	}

private:
	const char *string_;
};

class StringPiece//: public Copyable
{
public:
	// Provide non-explicit constructors to let users can pass a "const char*"
	// or a "string" wherever a "StringPiece" is expected.
	StringPiece(): data_(nullptr), length_(0) {}
	StringPiece(const char *str): data_(str), length_(static_cast<int>(strlen(str))) {}
	StringPiece(const string &str): data_(str.data()), length_(str.size()) {}
#ifndef NETLIB_STD_STRING
	StringPiece(const std::string &str): data_(str.data()), length_(str.size()) {}
#endif
	StringPiece(const char *ptr, int length): data_(ptr), length_(length) {}

	const char *get_data() const
	{
		return data_;
	}
	int get_size() const
	{
		return length_;
	}

private:
	const char *data_;
	int length_;
};

}

#endif // NETLIB_SRC_STRING_PIECE_H_
