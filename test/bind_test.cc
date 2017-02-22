#include <stdio.h>
#include <functional>

using std::function;
using std::bind;

using Functor = function<void(int)>;
int Fun()
{
	return printf("Test function bind passed.\n");
}

int main()
{
	// The function signature can be different.
	Functor functor = bind(Fun);
	// We can choose not use the original signature's args, if so,
	// the parameter(s) supplied were ignored.
	functor(0);
}
