#ifndef UTILS_H
#define UTILS_H

#define NULL ((void*) 0)
typedef unsigned long long size_t;

void memset(char *s, int ch, int size);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t count);
int strlen(const char *str);
void* memcpy(void* dest, const void* src, size_t len);

#endif