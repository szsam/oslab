#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n)
{
	char *cdest = (char *)dest;
	const char *csrc = (const char *)src;
	while (n-- > 0)
		*cdest++ = *csrc++;
	return dest;
}

void *memset(void *s, int c, size_t n)
{
	char *ps = (char *)s;
	while (n-- > 0)
		*ps = (char)c;
	return s;
}
