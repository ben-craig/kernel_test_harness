#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int main();

extern volatile long *g_test_failures;
extern char *g_output_buffer;
extern size_t g_space_available;

#ifdef __cplusplus
}
#endif
