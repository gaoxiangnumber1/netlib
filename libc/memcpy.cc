#include <stdio.h>

// void *memcpy(void *dest, const void *src, size_t n);
// copy n bytes from src to dest. Memory areas must not overlap.
// Use memmove(3) if the memory areas overlap. Return a pointer to dest.
void *MemCopy(void *dest, const void *src, size_t n)
{
	if(dest == nullptr || src == nullptr || dest == src)
	{
		return dest;
	}
	char *dest_char = static_cast<char*>(dest);
	const char *src_char = static_cast<const char*>(src);
	if(dest_char < src_char || dest_char >= src_char + n)
	{
		for(size_t index = 0; index < n; ++index)
		{
			dest_char[index] = src_char[index];
		}
	}
	else
	{
		for(size_t index = 0; index < n; ++index)
		{
			dest_char[n - 1 - index] = src_char[n - 1 - index];
		}
	}
	return dest;
}

int main()
{
	char str[4][32] = {"  abcdefg          ", "  abcdefg          ", "  abcdefg          ", "  abcdefg          "};
	size_t n = 7;
	MemCopy(str[0], str[0] + 2, n);
	MemCopy(str[1] + 2, str[1] + 2, n);
	MemCopy(str[2] + 4, str[2] + 2, n);
	MemCopy(str[3] + 10, str[3] + 2, n);
	printf("%s\n%s\n%s\n%s\n", str[0], str[1], str[2], str[3]);
	//"abcdefgfg          "
	//"  abcdefg          "
	//"  ababcdefg        "
	//"  abcdefg abcdefg  "
}
