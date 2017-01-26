#ifndef NETLIB_NETLIB_NON_COPYABLE_H_
#define NETLIB_NETLIB_NON_COPYABLE_H_

namespace netlib
{

class NonCopyable // Class that disallow copy or assignment. Object semantics.
{
public:
	NonCopyable() = default; // Constructor
	~NonCopyable() = default; // Destructor

	NonCopyable(const NonCopyable&) = delete; // Copy constructor
	NonCopyable &operator=(const NonCopyable&) = delete; // Copy assignment operator
};

}

#endif // NETLIB_NETLIB_NON_COPYABLE_H_
