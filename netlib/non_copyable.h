#ifndef NETLIB_NETLIB_NON_COPYABLE_H_
#define NETLIB_NETLIB_NON_COPYABLE_H_

namespace netlib
{

class NonCopyable // Class that disallow copy or assignment. Object semantics.
{
public:
	NonCopyable() = default;
	~NonCopyable() = default;

	NonCopyable(const NonCopyable&) = delete;
	NonCopyable &operator=(const NonCopyable&) = delete;
};

}

#endif // NETLIB_NETLIB_NON_COPYABLE_H_
