#include "lib.h"
#include "types.h"
#include <stdarg.h>
//#include <unistd.h>
//#include <sys/syscall.h>
//#include <string.h>

#define SYS_write 4
#define STDOUT_FILENO 1

/*
 * io lib here
 * 库函数写在这
 */
int32_t syscall(int num, uint32_t a1,uint32_t a2,
		uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret = 0;

	/* 内嵌汇编 保存 num, a1, a2, a3, a4, a5 至通用寄存器*/

	asm volatile("int $0x80" : "=a"(ret) : 
			"a"(num), "b"(a1), "c"(a2), "d"(a3), "D"(a4), "S"(a5) );
		
	return ret;
}

ssize_t write(int fd, const void *buf, size_t len) {
	ssize_t ret = 0;

	asm volatile("int $0x80" : "=a"(ret) : 
			"a"(SYS_write), "b"(fd), "c"(buf), "d"(len));

	return ret;
}


char* convert(unsigned int, int);       //Convert integer number into octal, hex, etc.

char *strcpy(char *dst, const char *src) {
	while ((*dst++ = *src++) != '\0');
	return dst;
}

size_t strlen(const char *s) {
	size_t len = 0;
	while (*s++)  ++len;
	return len;
}

int printf(const char *format,...){
	const char *traverse; 
	int x;
	char *s; 
	char buf[256];
	size_t len = 0;

	//Module 1: Initializing printf's arguments 
	va_list arg; 
	va_start(arg, format); 

	for(traverse = format; *traverse != '\0'; traverse++) 
	{ 
		if ( *traverse != '%' ) 
		{ 
			buf[len++] = *traverse;
		} 
		else
		{
			//Module 2: Fetching and executing arguments
			switch(*++traverse) 
			{ 
				case 'c' : 
					buf[len++] = va_arg(arg,int);     //Fetch char argument
					break; 

				case 'd' :
					x = va_arg(arg,int);         //Fetch Decimal/Integer argument
					if(x < 0) 
					{ 
						x = -x;
						buf[len++] = '-';
					} 
					s = convert(x, 10);
					strcpy(buf + len, s);
					len += strlen(s);
					break; 

				case 'x': //Fetch Hexadecimal representation
					s = convert(va_arg(arg,unsigned int),16);
					strcpy(buf + len, s);
					len += strlen(s);
					break; 

				case 's':
					s = va_arg(arg,char *);       //Fetch string
					strcpy(buf + len, s);
					len += strlen(s);
					break; 

			}   
		}
	} 

	//Module 3: Closing argument list to necessary clean-up
	va_end(arg); 

	buf[len] = '\0';

	// system call 'write'
	return write(STDOUT_FILENO, buf, len);
}

char *convert(unsigned int num, int base) 
{ 
	static char Representation[]= "0123456789abcdef";
	static char buffer[20]; 
	char *ptr; 

	ptr = &buffer[19]; 
	*ptr = '\0'; 

	do 
	{ 
		*--ptr = Representation[num % base]; 
		num /= base; 
	}while (num != 0);

	return ptr; 
}
