#ifndef NETLIB_SRC_EXCEPTION_H_
#define NETLIB_SRC_EXCEPTION_H_

#include <types.h>
#include <exception>

namespace netlib
{

class Exception: public std::exception
{
public:
	explicit Exception(const char *what);
	explicit Exception(const string &what);
	// A function that is designated by throw() promises not to throw any exceptions.
	// C++11: noexcept is equivalent to throw().
	virtual ~Exception() noexcept;
	virtual const char *what() const noexcept;
	const char *StackTrace() const noexcept;

private:
	void FillStackTrace();

	string message_;
	string stack_;
};

}

#endif // NETLIB_SRC_EXCEPTION_H_
