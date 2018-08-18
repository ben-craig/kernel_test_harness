#include <string.h>
#include <wdm.h>
#include "dummy.h"

volatile long *g_test_failures;
char *g_output_buffer;
size_t g_space_available;

static const char ASSERTION_FAILED[] = "assertion failed ";
static const size_t ASSERTION_FAILED_LEN = sizeof(ASSERTION_FAILED) - 1;

static void global_append(const char *buf, size_t buflen)
{
    if(buflen >= g_space_available)
        return;
    strncpy_s(g_output_buffer, g_space_available, buf, buflen);
    g_output_buffer[buflen] = 0;
    g_output_buffer += buflen;
    g_space_available -= buflen;
}

static int num_digits(int val)
{
    if(val < 10) return 1;
    if(val < 100) return 2;
    if(val < 1000) return 3;
    if(val < 10000) return 4;
    if(val < 100000) return 5;
    if(val < 1000000) return 6;
    if(val < 10000000) return 7;
    if(val < 100000000) return 8;
    if(val < 1000000000) return 9;
    return 10;
}

static void global_append_int(int val)
{
    int buflen = num_digits(val);
    if(buflen >= g_space_available)
        return;
    for(int i = 0; i < buflen; ++i)
    {
        int digit = val % 10;
        val /= 10;
        g_output_buffer[buflen - i - 1] = (char)('0' + digit);
    }
    g_output_buffer[buflen] = 0;
    g_output_buffer += buflen;
    g_space_available -= buflen;
}

extern "C" void doAssert(const char *file, int line, const char *expr)
{
    InterlockedIncrement(g_test_failures);
    global_append(ASSERTION_FAILED, ASSERTION_FAILED_LEN);
    global_append(file, strlen(file));
    global_append("(", 1);
    global_append_int(line);
    global_append("): ", 3);
    global_append(expr, strlen(expr));
    global_append("\n", 1);
}
