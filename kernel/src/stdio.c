#include <stdio.h>

#include <stdarg.h>
#include <string.h>
#include <ctype.h>

int puts(const char* str)
{
    textui_puts(str);
    putchar('\n');
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
        unsigned long minWidth = 0;
        unsigned long precision = 0xFFFFFFFF;
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
            format += str2int(format, &minWidth, 10);
        } else if (*format == '*') {
            ++format;
            minWidth = va_arg(args, int);
        }

        if (*format == '.') {
            if (isdigit(*++format)) {
                format += str2int(format, &precision, 10);
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
                int2str(-num, 10, buffer, sizeof(buffer));
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
            len = ((unsigned)len > precision ? precision : len);
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
            len = ((unsigned)len > precision ? precision : len);
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
