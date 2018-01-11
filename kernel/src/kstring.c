#include "kstring.h"
#include "sse.h"

void strcpy(char* dst, const char* src)
{
    while ((*dst++ = *src++)) {
        continue;
    }
}

int strcmp(const char* src1, const char* src2)
{
    int c1, c2;

    c1 = *src1++;
    c2 = *src2++;
    while (c1 == c2 && c1 != '\0') {
        c1 = *src1++;
        c2 = *src2++;
    }

    return c1 - c2;
}

void strcat(char* dst, const char* src)
{
    while (*dst++) {
        continue;
    }
    dst -= 1;
    while ((*dst++ = *src++)) {
        continue;
    }
}

uint64 strlen(const char* src)
{
    const char* origin = src;

    while (*src++) {
        continue;
    }

    return (uint64)(src - origin) - 1UL;
}

void strncpy(char* dst, const char* src, uint64 num)
{
    int c;

    while ((c = *src++) && num > 1) {
        *dst++ = c;
        --num;
    }
    while (num != 0) {
        *dst++ = '\0';
        --num;
    }
}

int strncmp(const char* src1, const char* src2, uint64 num)
{
    int c1, c2;

    if (num == 0) {
        return 0;
    }

    c1 = *src1++;
    c2 = *src2++;
    --num;
    while (num != 0 && c1 == c2 && c1 != '\0') {
        c1 = *src1++;
        c2 = *src2++;
        --num;
    }

    return c1 - c2;
}

void strncat(char* dst, const char* src, uint64 num)
{
    if (num != 0) {
        while (*dst++) {
            continue;
        }
        dst -= 1;
        while ((*dst++ = *src++)) {
            if (--num == 0) {
                *dst = '\0';
                break;
            }
        }
    }
}

uint64 str2int(const char* src, int base)
{
    int c;
    uint64 result = 0;

    while ((c = toupper(*src++))) {
        result *= base;
        if (isdigit(c) && (c - '0') < base) {
            result += c - '0';
        } else if (isalpha(c) && (c - 'A' + 10) < base) {
            result += c - 'A' + 10;
        } else {
            break;
        }
    }

    return result;
}

char* int2str(uint64 num, int base, char* buffer, int bufferSize)
{
    static const char digits[] = "0123456789"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrrtuvwxyz";
    int i, strSize = 0;

    if (base <= 0 || base > (int)sizeof(digits) - 1 || buffer == NULL || bufferSize <= 0) {
        return NULL;
    }
    if (bufferSize == 1) {
        *buffer = '\0';
        return buffer;
    }

    do {
        buffer[strSize++] = digits[num % base];
        num /= base;
    } while (strSize < bufferSize - 1 && num != 0);
    buffer[strSize] = '\0';

    for (i = 0; i < strSize/2; ++i) {
        char temp = buffer[i];
        buffer[i] = buffer[strSize - i - 1];
        buffer[strSize - i - 1] = temp;
    }

    return buffer;
}

void memset(void* dst, int byteValue, uint64 size)
{
    uint64 i = (16 - ((intptr)dst & 15)) & 15;
    uint8* _dst = dst;
    sse_vec64 value;

    value[0] = value[1] = (uint8)byteValue;
    value |= (value << 8);
    value |= (value << 16);
    value |= (value << 32);

    while (i-- && size) {
        *_dst++ = value[0];
        --size;
    }
    while (size >= 16) {
        *((sse_vec64*)_dst) = value;
        _dst += 16;
        size -= 16;
    }
    while (size--) {
        *_dst++ = value[0];
    }
}

void memcpy(void* dst, const void* src, uint64 size)
{
    uint64 i;
    uint8* _dst = dst;
    const uint8* _src = src;

    if (((intptr)_dst & 15) == ((intptr)_src & 15)) {
        i = (16 - ((intptr)_dst & 15)) & 15;
        while (i-- && size) {
            *_dst++ = *_src++;
            --size;
        }
        while (size >= 16) {
            *((sse_vec64*)_dst) = *((sse_vec64*)_src);
            _dst += 16;
            _src += 16;
            size -= 16;
        }
    } else if (((intptr)_dst & 7) == ((intptr)_src & 7)) {
        i = (8 - ((intptr)_dst & 7)) & 7;
        while (i-- && size) {
            *_dst++ = *_src++;
            --size;
        }
        while (size >= 8) {
            *((uint64*)_dst) = *((uint64*)_src);
            _dst += 8;
            _src += 8;
            size -= 8;
        }
    } else if (((intptr)_dst & 3) == ((intptr)_src & 3)) {
        i = (4 - ((intptr)_dst & 3)) & 3;
        while (i-- && size) {
            *_dst++ = *_src++;
            --size;
        }
        while (size >= 4) {
            *((uint32*)_dst) = *((uint32*)_src);
            _dst += 4;
            _src += 4;
            size -= 4;
        }
    } else if (((intptr)_dst & 1) == ((intptr)_src & 1)) {
        i = (2 - ((intptr)_dst & 1)) & 1;
        if (i-- && size) {
            *_dst++ = *_src++;
            --size;
        }
        while (size >= 2) {
            *((uint16*)_dst) = *((uint16*)_src);
            _dst += 2;
            _src += 2;
            size -= 2;
        }
    }

    while (size--) {
        *_dst++ = *_src++;
    }
}

void memcpy_back(void* dst, const void* src, uint64 size)
{
    uint64 i;
    uint8* _dst = (uint8*)dst + size - 1;
    const uint8* _src = (const uint8*)src + size - 1;

    if (((intptr)_dst & 15) == ((intptr)_src & 15)) {
        i = ((intptr)_dst & 15);
        while (i-- && size) {
            *_dst-- = *_src--;
            --size;
        }
        while (size >= 16) {
            *((sse_vec64*)_dst) = *((sse_vec64*)_src);
            _dst -= 16;
            _src -= 16;
            size -= 16;
        }
    } else if (((intptr)_dst & 7) == ((intptr)_src & 7)) {
        i = ((intptr)_dst & 7);
        while (i-- && size) {
            *_dst-- = *_src--;
            --size;
        }
        while (size >= 8) {
            *((uint64*)_dst) = *((uint64*)_src);
            _dst -= 8;
            _src -= 8;
            size -= 8;
        }
    } else if (((intptr)_dst & 3) == ((intptr)_src & 3)) {
        i = ((intptr)_dst & 3);
        while (i-- && size) {
            *_dst-- = *_src--;
            --size;
        }
        while (size >= 4) {
            *((uint32*)_dst) = *((uint32*)_src);
            _dst -= 4;
            _src -= 4;
            size -= 4;
        }
    } else if (((intptr)_dst & 1) == ((intptr)_src & 1)) {
        i = ((intptr)_dst & 1);
        if (i-- && size) {
            *_dst-- = *_src--;
            --size;
        }
        while (size >= 2) {
            *((uint16*)_dst) = *((uint16*)_src);
            _dst -= 2;
            _src -= 2;
            size -= 2;
        }
    }

    while (size--) {
        *_dst-- = *_src--;
    }
}

void memmove(void* dst, const void* src, uint64 size)
{
    if ((intptr)dst <= (intptr)src || (intptr)src + size <= (intptr)dst) {
        memcpy(dst, src, size);
    } else {
        memcpy_back(dst, src, size);
    }
}
