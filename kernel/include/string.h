#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void strcpy(char* dst, const char* src);
int strcmp(const char* src1, const char* src2);
void strcat(char* dst, const char* src);

size_t strlen(const char* src);

void strncpy(char* dst, const char* src, size_t num);
int strncmp(const char* src1, const char* src2, size_t num);
void strncat(char* dst, const char* src, size_t num);

size_t str2int(const char* src, unsigned long* numPtr, int base);
char* int2str(size_t num, int base, char* buffer, int bufferSize);

void memset(void* dst, int byteValue, size_t num);
void memcpy(void* dst, const void* src, size_t num);
void memcpy_back(void* dst, const void* src, size_t num); // Copies from last to first
void memmove(void* dst, const void* src, size_t num);
int memcmp(const void* src1, const void* src2, size_t num);

#endif // STRING_H
