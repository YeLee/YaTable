#ifndef MEM_H
#define MEM_H
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

int absval(int i);
char* strcpyexceptchar(char* dest, char* src, char ch);
char* delwhitespace(char* src);
char* endofnum(char* src);
int str2num(char* src);
char* strcpy2chr(char* tarstr, char* srcstr, char tarchr);
char* ui2str(const unsigned int i);

void* yamalloc(const size_t i);
void yafree(void* mem);

char* newstrcat(const int num, char* newstart, ...);

#endif

