#ifndef HAD_MEM_C
#define HAD_MEM_C

#include "mem.h"

int absval(int i)
{
    if(i < 0) return (0 - i);
    return i;
}

char* strcpyexceptchar(char* dest, char* src, char ch)
{
    size_t i = 0, j = 0;
    while(src[j] != '\0') {
        while(src[j] == ch)
            j++;
        dest[i] = src[j];
        i++;
        j++;
    };
    dest[i] = '\0';
    return dest;
}

char* delwhitespace(char* src)
{
    size_t i = 0, j = 0;
    while(src[j] != '\0') {
        while((src[j] == ' ') || (src[j] == '\t'))
            j++;
        src[i] = src[j];
        i++;
        j++;
    };
    src[i] = '\0';
    return src;
}

char* endofnum(char* src)
{
    if(src == NULL) return NULL;

    int i = 0;
    while(src[i] != '\0' && src[i] >= '0' && src[i] <= '9')
        i++;
    return src + i;
}

int str2num(char* src)
{
    if(src == NULL) return 0;
    size_t i = 0;
    int res;
    while(src[i] != '\0' && src[i] >= '0' && src[i] <= '9')
        i++;
    char* numstr = yamalloc((i + 1) * sizeof(char));
    strncpy(numstr, src, i);
    numstr[i] = '\0';
    res = atoi(numstr);
    yafree(numstr);
    return res;
}

char* strcpy2chr(char* tarstr, char* srcstr, char tarchr)
{
    unsigned int i = 0;
    while((*(srcstr + i) != tarchr) && (*(srcstr + i) != '\0')) {
        *(tarstr + i) = *(srcstr + i);
        i++;
    }

    *(tarstr + i) = '\0';
    return tarstr;
}

char* ui2str(const unsigned int i)
{
    static char result[11];
    memset(result, '\0', 11);
    sprintf(result, "%u", i);
    return result;
}

void* yamalloc(const size_t i)
{
    static int imalloc = 0;
    void* addr = NULL;

    if(i == 0) {
#ifndef NDEBUG
        fprintf(stderr, "Allocated:%d\n", imalloc);
#endif
        return NULL;
    }
    imalloc ++;

    addr = malloc(i);
#ifndef NDEBUG
    fprintf(stderr, "Allocated:0x%lx\n", (unsigned long) addr);
#endif
    memset(addr, '\0', i);
    return addr;
}

void yafree(void* mem)
{
    static int ifree = 0;

    if(mem == NULL) {
#ifndef NDEBUG
        fprintf(stderr, "Recovered:%d\n", ifree);
#endif
        return;
    }
    ifree ++;

    free(mem);
#ifndef NDEBUG
    fprintf(stderr, "Recovered:0x%lx\n", (unsigned long) mem);
#endif
    return;
}

char* newstrcat(const int num, char* newstart, ...)
{
    int len = 0;
    int i;
    va_list ap;
    char* newstr = NULL;
    char* argstr = NULL;

    if(newstart) len = strlen(newstart);

    va_start(ap, newstart);

    for(i = 1; i < num; i++) {
        argstr = (char*) va_arg(ap, char*);
        if(argstr != NULL) len += strlen(argstr);
    }

    va_end(ap);

    newstr = malloc((len + 1) * sizeof(char));
    memset(newstr, '\0', (len + 1) * sizeof(char));
    if(newstart) {
        strcpy(newstr, newstart);
        free(newstart);
    }

    va_start(ap, newstart);

    for(i = 1; i < num; i++) {
        argstr = (char*) va_arg(ap, char*);
        if(argstr != NULL) strcat(newstr, argstr);
    }

    va_end(ap);

    return newstr;
}

#endif
