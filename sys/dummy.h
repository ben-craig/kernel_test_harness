#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int main();

extern volatile int *g_test_failures;
extern char *g_output_buffer;
extern int g_space_available;

#ifdef __cplusplus
}
#endif
