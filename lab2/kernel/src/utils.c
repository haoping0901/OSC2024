#include "../include/utils.h"

void memset(char *s, int ch, int size)
{
    if (!size) return;
    
    ch &= 0xFF;
    while (size--) *s++ = ch;
    
    return;
}

int strcmp(const char *s1, const char *s2)
{
    unsigned char c1, c2;

    while (1) {
        c1 = *s1++;
        c2 = *s2++;
        if (c1 != c2)
            return c1 - c2;
        
        if (!c1) break;
    }
    
    return 0;
}

int strncmp(const char *s1, const char *s2, unsigned long long count)
{
    unsigned char c1, c2;

    while (count) {
        c1 = *s1++;
        c2 = *s2++;

        if (c1 != c2)
            return c1 < c2 ? -1 : 1;
        
        if (!c1)
            break;

        --count;
    }
    
    return 0;
}

int strlen(const char *str)
{
    const char* cur = str;

    while (*cur != '\0') {
        ++cur;
    }
    
    return cur - str;
}
