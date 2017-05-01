#include <stdarg.h>
#include "types.h"
#include "lib.h"

#define STDOUT_FILENO 1

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
