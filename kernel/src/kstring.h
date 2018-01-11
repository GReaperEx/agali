#ifndef KSTRING_H
#define KSTRING_H

#include "ktypes.h"

void strcpy(char* dst, const char* src);
int strcmp(const char* src1, const char* src2);
void strcat(char* dst, const char* src);

uint64 strlen(const char* src);

void strncpy(char* dst, const char* src, uint64 num);
int strncmp(const char* src1, const char* src2, uint64 num);
void strncat(char* dst, const char* src, uint64 num);

uint64 str2int(const char* src, int base);
char* int2str(uint64 num, int base, char* buffer, int bufferSize);

void memset(void* dst, int byteValue, uint64 size);
void memcpy(void* dst, const void* src, uint64 size);
void memcpy_back(void* dst, const void* src, uint64 size); // Copies from last to first
void memmove(void* dst, const void* src, uint64 size);

#endif // KSTRING_H
