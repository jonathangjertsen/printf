#include "printf.h"
#include <string.h>
#include <stdbool.h>

#ifndef FEATURE_64BIT
    #define FEATURE_64BIT 0
#endif

#ifndef FEATURE_32BIT
    #define FEATURE_32BIT 0
#endif

#ifndef FEATURE_SHORT
    #define FEATURE_SHORT 0
#endif

#ifndef FEATURE_CHAR
    #define FEATURE_CHAR 0
#endif

#define MAX_N_DIGITS_UINT16 4
#define MAX_DIVIDER_UINT16  10000U

#define MAX_N_DIGITS_UINT32 10
#define MAX_DIVIDER_UINT32  1000000000UL

#define MAX_N_DIGITS_UINT64 20
#define MAX_DIVIDER_UINT64  1000000000ULL

// Max printable number: negative int64_t (no null terminator needed)
#define AUX_BUFFER_SIZE (1 + MAX_N_DIGITS_UINT64)

#define FORMAT_CODE_DEC 0
#define FORMAT_CODE_BIN 1
#define FORMAT_CODE_HEX 2

#if FEATURE_64BIT
    #define LONGEST_INT long long int
    #define MAX_DIVIDER MAX_DIVIDER_UINT64
#elif FEATURE_32BIT
    #define LONGEST_INT long int
    #define MAX_DIVIDER MAX_DIVIDER_UINT32
#else
    #define LONGEST_INT int
    #define MAX_DIVIDER MAX_DIVIDER_UINT16
#endif

static const char g_hex_table[17] = "0123456789ABCDEF";

static int plain_length(const char *string);
static int place_int_dec(LONGEST_INT value, char *buffer);
static int place_int_hex(unsigned LONGEST_INT value, char *buffer);
static void printf_inner(const char *format, va_list args, emit_func_t emit);

int printf(const char *format, ...)
{
    va_list va;
    va_start(va, format);

    emit_func_t emit_func = printf_claim_stdio();
    if (emit_func)
    {
        printf_inner(format, va, emit_func);
    } 
    printf_release_stdio(emit_func);

    va_end(va);
    return 0;
}

static void printf_inner(const char *format, va_list args, emit_func_t emit)
{
    char aux_buffer[AUX_BUFFER_SIZE] = { 0 };
    const char *cursor = format;

    while (true)
    {
        // Emit the next stretch of plain text
        int len = plain_length(cursor);
        if (len > 0)
        {
            emit(format, len);
            cursor += len;
        }

        // Whatever is next must be '%' (in which case we parse), or '\0' (in which case we stop)
        if (*cursor != '%')
        {
            return;
        }
        cursor++;

        // Look for #
        int prefix_offset = 0;
        if (*cursor == '#')
        {
            prefix_offset = 2;
            cursor++;
        }

        // Now parsing a datatype specifier.
        // First, map the 'l', 'll', 'h', 'hh' prefixes to an int 'longness'
        // long long => longness = 2
        // long      => longness = 1
        // int       => longness = 0
        // short     => longness = -1
        // char      => longness = -2
#if FEATURE_32BIT
        int_fast8_t longness = 0;
        if (*cursor == 'l')
        {
            longness++;
            cursor++;
        }
#if FEATURE_64BIT
        if (*cursor == 'l')
        {
            longness++;
            cursor++;
        }
#endif // FEATURE_64BIT
#endif // FEATURE_32BIT

#if FEATURE_SHORT
        if (*cursor == 'h')
        {
            longness--;
            cursor++;
        }
#if FEATURE_CHAR
        if (*cursor == 'h')
        {
            longness--;
            cursor++;
        }
#endif // FEATURE_CHAR
#endif // FEATURE_SHORT

        // This will be set in the switch statement using fall-through
        // hex     => format_code = 2
        // bin     => format_code = 1
        // dec     => format_code = 0
        int format_code = 0;

        switch (*cursor)
        {
            // Some signed integer
            case 'd':
            case 'i':
            {
                // Extract number
                LONGEST_INT number;
                switch (longness)
                {
#if FEATURE_SHORT
#if FEATURE_CHAR
                    case -2:
                        number = va_arg(args, int) % 0x100;
                    break;
#endif // FEATURE_CHAR
                    case -1:
                        number = va_arg(args, int) % 0x10000;
                    break;
#endif // FEATURE_SHORT
                    case 0:
                        number = va_arg(args, int);
                    break;
#if FEATURE_32BIT
                    case 1:
                        number = va_arg(args, long int);
                    break;
#if FEATURE_64BIT
                    case 2:
                        number = va_arg(args, long long int);
                    break;
#endif // FEATURE_64BIT
#endif // FEATURE_32BIT
                }

                // Format and emit number
                const int size = place_int_dec(number, aux_buffer);
                emit(aux_buffer, size);
            }
            break;


            // Some signed integer
            // The code is quite similar for hex and decimal
            case 'x':
            case 'X':
            format_code++;
            case 'b':
            format_code++;
            case 'u':
            {
                // Generate prefix
                if (prefix_offset)
                {
                    aux_buffer[0] = '0';
                    aux_buffer[1] = (format_code == FORMAT_CODE_BIN) ? 'b' : 'x';
                }

                // Extract number
                LONGEST_INT number;
                switch (longness)
                {
#if FEATURE_SHORT
#if FEATURE_CHAR
                    case -2:
                        number = va_arg(args, unsigned int) & 0xff;
                    break;
#endif // FEATURE_CHAR

                    case -1:
                        number = va_arg(args, unsigned int) & 0xffff;
                    break;
#endif // FEATURE_SHORT
                    case 0:
                        number = va_arg(args, unsigned int);
                    break;
#if FEATURE_32BIT
                    case 1:
                        number = va_arg(args, long int);
                    break;
#if FEATURE_64BIT
                    case 2:
                        number = va_arg(args, long long int);
                    break;
#endif // FEATURE_64BI
#endif // FEATURE_32BIT
                }

                // Format and emit number
                int size = 0;
                if (format_code == FORMAT_CODE_HEX)
                {
                    size = place_int_hex(number, &aux_buffer[prefix_offset]);
                }
                else if (format_code == FORMAT_CODE_DEC)
                {
                    size = place_int_dec(number, &aux_buffer[prefix_offset]);
                }
                emit(aux_buffer, prefix_offset + size);
                if (format_code == FORMAT_CODE_BIN)
                {
                    // Shuttle a bit up to the msb of number, then shuttle it back down while printing
                    // This prints out the bits from most significant to least
                    LONGEST_INT mask = 1;
                    while (mask < number)
                    {
                        mask <<= 1;
                    }
                    while (mask > 1)
                    {
                        mask >>= 1;
                        emit(number & mask ? "1" : "0", 1);
                    }
                }
                break;
            }

            // String
            case 's':
            {
                const char *string = va_arg(args, const char *);
                emit(string, strlen(string));
            }
            break;

#if FEATURE_CHAR
            // Character
            case 'c':
            {
                const char ch = va_arg(args, int);
                emit(&ch, 1);
            }
            break;
#endif // FEATURE_CHAR

            // Percent or other character
            case '%':
            cursor++;
            default:
            {
                emit(cursor-1, 2);
            }
            break;
        }
        cursor++;
    }
}

static int plain_length(const char *string)
{
    const char *ch = string;
    while (true)
    {
        if ((*ch == '\0') || (*ch == '%'))
        {
            return ch - string;
        }
        ch++;
    }
}

static int place_int_dec(LONGEST_INT value, char *buffer)
{
    if (value < 0)
    {
        buffer[0] = '-';
        return place_int_dec(-value, &buffer[1]) + 1;
    }

    if (value == 0)
    {
        buffer[0] = '0';
        return 1;
    }

    int_fast16_t printed = 0;
    LONGEST_INT divider = MAX_DIVIDER;
    bool first_zero_encountered = false;
    while (divider > 0)
    {
        uint_fast8_t digit = ((value / divider) % 10);
        first_zero_encountered |= digit != 0;
        if (first_zero_encountered)
        {
            buffer[printed] = g_hex_table[digit];
            printed++;
        }
        else
        {
            // This is a leading zero, don't print it
        }
        divider /= 10;
    }
    return printed;
}

static int place_int_hex(unsigned LONGEST_INT value, char *buffer)
{
    int hexlen = 0;
    unsigned LONGEST_INT probe = value;
    while (probe > 0)
    {
        probe >>= 8;
        hexlen += 2;
    }
    int counter = hexlen;
    while (counter > 0)
    {
        counter--;
        buffer[counter] = g_hex_table[value & 0xf];
        value >>= 4;
        counter--;
        buffer[counter] = g_hex_table[value & 0xf];
        value >>= 4;
    }
    return hexlen;
}
