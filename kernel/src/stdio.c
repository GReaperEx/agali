#include <stdio.h>

#include <stdarg.h>
#include <string.h>
#include <ctype.h>

int puts(const char* str)
{
    textui_puts(str);
    putchar('\n');
    return 0;
}

int printf(const char* format, ...)
{
    va_list args;
    int c, i = 0;
    char buffer[32];
    int count = 0;
    int len;

    va_start(args, format);
    while ((c = *format++) != '\0') {
        int left = 0;
        int sign = 0;
        char padding = ' ';
        int altForm = 0;
        int minWidth = 0;
        int precision = 0x7FFFFFFF;
        int typeMod = 0;
        const char* altBuffer = NULL;

        if (c != '%') {
            putchar(c);
            ++count;
            continue;
        }

        switch (*format++)
        {
        case '-':
            left = 1;
        break;
        case '+':
            sign = 1;
        break;
        case '0':
            padding = '0';
        break;
        case '#':
            altForm = 1;
        break;
        default:
            --format;
        }

        if (isdigit(*format)) {
            uint64 temp;
            format += str2int(format, &temp, 10);
            minWidth = temp;
        } else if (*format == '*') {
            ++format;
            minWidth = va_arg(args, int);
        }

        if (*format == '.') {
            if (isdigit(*++format)) {
                uint64 temp;
                format += str2int(format, &temp, 10);
                precision = temp;
            } else if (*format == '*') {
                ++format;
                precision = va_arg(args, int);
            }else {
                precision = 0;
            }
        }

        switch (*format++)
        {
        case 'h':
            typeMod = 1;
        break;
        case 'l':
            typeMod = 2;
        break;
        //case 'L':
        //    typeMod = 3;
        //break;
        default:
            --format;
        }

        switch ((c = *format++))
        {
        case '%':
            buffer[0] = '%';
            buffer[1] = '\0';
        break;
        case 'd':
        case 'i':
        {
            long num;

            switch (typeMod)
            {
            case 0:
                num = va_arg(args, int);
            break;
            case 1:
                num = (short)(va_arg(args, int));
            break;
            case 2:
                num = va_arg(args, long);
            break;
            }

            if (num < 0) {
                buffer[0] = '-';
                int2str(-num, 10, &buffer[1], sizeof(buffer) - 1);
            } else {
                int2str(num, 10, buffer, sizeof(buffer));
            }
        }
        break;
        case 'x':
        case 'X':
        case 'p':
        {
            unsigned long num;

            if (c == 'p') {
                num = (unsigned long)va_arg(args, void*);
            } else {
                switch (typeMod)
                {
                case 0:
                    num = va_arg(args, unsigned int);
                break;
                case 1:
                    num = (unsigned short)(va_arg(args, unsigned int));
                break;
                case 2:
                    num = va_arg(args, unsigned long);
                break;
                }
            }

            int2str(num, 16, buffer, sizeof(buffer));
        }
        break;
        case 'o':
        {
            unsigned long num;

            switch (typeMod)
            {
            case 0:
                num = va_arg(args, unsigned int);
            break;
            case 1:
                num = (unsigned short)(va_arg(args, unsigned int));
            break;
            case 2:
                num = va_arg(args, unsigned long);
            break;
            }

            int2str(num, 8, buffer, sizeof(buffer));
        }
        break;
        case 'u':
        {
            unsigned long num;

            switch (typeMod)
            {
            case 0:
                num = va_arg(args, unsigned int);
            break;
            case 1:
                num = (unsigned short)(va_arg(args, unsigned int));
            break;
            case 2:
                num = va_arg(args, unsigned long);
            break;
            }

            int2str(num, 10, buffer, sizeof(buffer));
        }
        break;
        case 'c':
            buffer[0] = (char)(va_arg(args, int));
            buffer[1] = '\0';
        break;
        case 's':
            altBuffer = va_arg(args, const char*);
        break;
        case 'n':
            *(va_arg(args, int*)) = count;
            buffer[0] = '\0';
        break;
        default:
            buffer[0] = '\0';
        }

        if (altBuffer != NULL) {
            len = strlen(altBuffer);
            len = (len > precision ? precision : len);
            if (!left && len < minWidth) {
                for (i = 0; i < minWidth - len; ++i) {
                    putchar(padding);
                    ++count;
                }
            }
            for (i = 0; i < len; ++i) {
                putchar(altBuffer[i]);
                ++count;
            }
            if (left && len < minWidth) {
                for (i = 0; i < minWidth - len; ++i) {
                    putchar(padding);
                    ++count;
                }
            }
        } else {
            len = strlen(buffer);
            len = (len > precision ? precision : len);
            if (!left && len < minWidth) {
                for (i = 0; i < minWidth - len; ++i) {
                    putchar(padding);
                    ++count;
                }
            }
            for (i = 0; i < len; ++i) {
                putchar(buffer[i]);
                ++count;
            }
            if (left && len < minWidth) {
                for (i = 0; i < minWidth - len; ++i) {
                    putchar(padding);
                    ++count;
                }
            }
        }
    }
    va_end(args);

    return count;
}

int getchar(void)
{
    int c;

    textui_showCursor(TRUE);
    c = kbrd_getchar();
    textui_showCursor(FALSE);

    textui_putchar(c);
    return c;
}

char* gets(char* dst, int n)
{
    int c, i = 0;

    textui_showCursor(TRUE);
    while (i < n - 1) {
        dst[i++] = (c = kbrd_getchar());
        textui_putchar(c);
        if (c == '\n') {
            break;
        }
    }
    dst[i] = '\0';
    textui_showCursor(FALSE);

    return dst;
}
