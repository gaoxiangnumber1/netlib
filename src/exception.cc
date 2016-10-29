#include <exception.h>

#include <execinfo.h>
#include <stdlib.h>

using netlib::Exception;

Exception::Exception(const char *message): message_(message)
{
	FillStackTrace();
}

Exception::Exception(const string &message): message_(message)
{
	FillStackTrace();
}

Exception::~Exception() noexcept {}

const char *Exception::what() const noexcept
{
	return message_.c_str();
}

const char *Exception::StackTrace() const noexcept
{
	return stack_.c_str();
}

void Exception::FillStackTrace()
{
	int length = 200;
	void *buffer[length];
	// int backtrace(void **buffer, int size);
	// Return the number of addresses returned in buffer.
	int address_number = ::backtrace(buffer, length);
	// char **backtrace_symbols(void *const *buffer, int size);
	// Return a pointer to the array malloc(3)ed by the call.
	char **strings = ::backtrace_symbols(buffer, address_number);
	if(strings)
	{
		for(int index = 0; index < address_number; ++index)
		{
			stack_.append(strings[index]);
			stack_.push_back('\n');
		}
		free(strings);
	}
}
