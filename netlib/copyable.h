#ifndef NETLIB_NETLIB_COPYABLE_H_
#define NETLIB_NETLIB_COPYABLE_H_

namespace netlib
{
// A tag class emphasizes the objects are copyable.
// The empty base class optimization applies.
// Any derived class of copyable should be a value type.
class Copyable {}; // Value semantics
}

#endif // NETLIB_NETLIB_COPYABLE_H_
