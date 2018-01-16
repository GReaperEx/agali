#include <agali/ktypes.h>

#include <string.h>

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

size_t strlen(const char* src)
{
    const char* origin = src;

    while (*src++) {
        continue;
    }

    return (size_t)(src - origin) - 1UL;
}

void strncpy(char* dst, const char* src, size_t num)
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

int strncmp(const char* src1, const char* src2, size_t num)
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

void strncat(char* dst, const char* src, size_t num)
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

size_t str2int(const char* src, unsigned long* numPtr, int base)
{
    int c, counter = 0;
    size_t result = 0;

    while ((c = toupper(*src++))) {
        if (isdigit(c) && (c - '0') < base) {
            result *= base;
            result += c - '0';
        } else if (isalpha(c) && (c - 'A' + 10) < base) {
            result *= base;
            result += c - 'A' + 10;
        } else {
            break;
        }
        ++counter;
    }

    if (numPtr) {
        *numPtr = result;
    }
    return counter;
}

char* int2str(size_t num, int base, char* buffer, int bufferSize)
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

void memset(void* dst, int byteValue, size_t size)
{
    size_t i = (16 - ((intptr)dst & 15)) & 15;
    uint8* _dst = dst;
    uint64 value;

    value = ((uint64)(byteValue & 0xFF) << 32) | (byteValue & 0xFF);
    value |= (value << 8);
    value |= (value << 16);

    while (i-- && size) {
        *_dst++ = value;
        --size;
    }
    while (size >= 16) {
        *((uint64*)_dst) = value;
        _dst += 16;
        size -= 16;
    }
    while (size--) {
        *_dst++ = value;
    }
}

void memcpy(void* dst, const void* src, size_t size)
{
    size_t i;
    uint8* _dst = dst;
    const uint8* _src = src;

    if (((intptr)_dst & 7) == ((intptr)_src & 7)) {
        i = (8 - ((intptr)_dst & 7)) & 7;
        while (i-- && size) {
            *_dst++ = *_src++;
            --size;
        }
        while (size >= 8) {
            *((size_t*)_dst) = *((size_t*)_src);
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

void memcpy_back(void* dst, const void* src, size_t size)
{
    size_t i;
    uint8* _dst = (uint8*)dst + size - 1;
    const uint8* _src = (const uint8*)src + size - 1;

    if (((intptr)_dst & 7) == ((intptr)_src & 7)) {
        i = ((intptr)_dst & 7);
        while (i-- && size) {
            *_dst-- = *_src--;
            --size;
        }
        while (size >= 8) {
            *((size_t*)_dst) = *((size_t*)_src);
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

void memmove(void* dst, const void* src, size_t size)
{
    if ((intptr)dst <= (intptr)src || (intptr)src + size <= (intptr)dst) {
        memcpy(dst, src, size);
    } else {
        memcpy_back(dst, src, size);
    }
}

int memcmp(const void* src1, const void* src2, size_t size)
{
    size_t i;
    const uint8* _src1 = src1;
    const uint8* _src2 = src2;

    for (i = 0; i < size; ++i) {
        if (_src1[i] != _src2[i]) {
            return _src1[i] - _src2[i];
        }
    }
    return 0;
}
