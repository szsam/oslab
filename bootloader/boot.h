#ifndef BOOT_H
#define BOOT_H
#include <stddef.h>

typedef struct ELFHeader {
	unsigned int   magic;
	unsigned char  elf[12];
	unsigned short type;
	unsigned short machine;
	unsigned int   version;
	unsigned int   entry;
	unsigned int   phoff;
	unsigned int   shoff;
	unsigned int   flags;
	unsigned short ehsize;
	unsigned short phentsize;
	unsigned short phnum;
	unsigned short shentsize;
	unsigned short shnum;
	unsigned short shstrndx;
}ELFHeader;

/* ELF32 Program header */
typedef struct ProgramHeader {
	unsigned int type;
	unsigned int off;
	unsigned int vaddr;
	unsigned int paddr;
	unsigned int filesz;
	unsigned int memsz;
	unsigned int flags;
	unsigned int align;
}ProgramHeader;

void waitDisk(void);

void readSect(void *dst, int offset);

/* I/O functions */
static inline char inByte(short port) {
	char data;
	asm volatile("in %1,%0" : "=a" (data) : "d" (port));
	return data;
}

static inline int inLong(short port) {
	int data;
	asm volatile("in %1, %0" : "=a" (data) : "d" (port));
	return data;
}

static inline void outByte(short port, char data) {
	asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

inline void *memcpy(void *dest, const void *src, size_t n)
{
	char *cdest = (char *)dest;
	const char *csrc = (const char *)src;
	while (n-- > 0)
		*cdest++ = *csrc++;
	return dest;
}

inline void *memset(void *s, int c, size_t n)
{
	char *ps = (char *)s;
	while (n-- > 0)
		*ps = (char)c;
	return s;
}

#endif
