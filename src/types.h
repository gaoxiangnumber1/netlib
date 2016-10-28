#ifndef NETLIB_SRC_BASE_TYPES_H_
#define NETLIB_SRC_BASE_TYPES_H_

#include <stdint.h> // Header for `int64_t`

// By default, we don't use std::string, but user can explicitly
// use std::string by define NETLIB_STD_STRING
#ifdef NETLIB_STD_STRING
#include <string>
#else
#include <ext/vstring.h>
#include <ext/vstring_fwd.h>
#endif // NETLIB_STD_STRING

#ifndef NDEBUG
// If the macro NDEBUG was defined at the moment <assert.h> was last included, assert()
// does nothing. Otherwise, assert() prints an error message to stderr and terminates the
// program by calling abort(3) if expression is false(i.e., compares equal to zero).
#include <assert.h>
#endif // NDEBUG

namespace netlib
{
#ifdef NETLIB_STD_STRING
using std::string;
#else
typedef __gnu_cxx::__sso_string string;
#endif // NETLIB_STD_STRING

// up_cast is a safe version of static_cast or const_cast for up-casting
// in the type hierarchy(i.e. casting `Test*` to `SuperClassOfTest*` or
// casting `Test*` to `const Test*`).
// When using up_cast, the compiler checks that the up-cast is safe.
// up_casts are needed in situations where C++ demands an exact type match.
// syntax: `up_cast<ToType>(FromTypeExpression)`
template<typename To, typename From>
inline To up_cast(From const &from)
{
	return from;
}

// When we up-cast(i.e., cast `Test*` to `SuperClassOfTest*`), we can use up_cast
// since up-casts always succeed.  When we downcast(i.e., cast `Test*` to
// `SubClassOfTest*`), static_cast isn't safe because the pointer to base class
// may a bare Test, or of type DifferentSubClassOfTest.
// Thus, we should use this macro when we downcast.
// In debug mode, we use dynamic_cast to double-check the downcast is legal
// (program aborts if it's not).  In normal mode, we do the efficient static_cast instead.
// So, we should test in debug mode to make sure the cast is legal. This is the only place
// in the code we should use dynamic_cast. In particular, we shouldn't use dynamic_cast
// in order to do RTTI, for example:
//    `if (dynamic_cast<SubClass1>(Test)) HandleSubClass1Object(Test);`
//    `if (dynamic_cast<SubClass2>(Test)) HandleSubClass2Object(Test);`
// syntax: `down_cast<ToType*>(FromTypeExpression)`
template<typename To, typename From>
inline To down_cast(From* from) // Only accept pointers since To is always a pointer type.
{
	// Ensure that To is a subtype of From*.  This test is only for compile-time
	// type checking, and has no overhead in an optimized build at run-time.
	if(false)
	{
		up_cast<From*, To>(0);
	}

#ifndef NDEBUG // RTTI in debug mode only.
	assert(from == NULL || dynamic_cast<To>(from) != NULL);
#endif
	return static_cast<To>(from);
}
}

#endif // NETLIB_SRC_BASE_TYPES_H_
