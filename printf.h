#pragma once

#ifdef __cplusplus
    extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

typedef void (*emit_func_t)(const char *buffer, int size);

emit_func_t printf_claim_stdio();
void printf_release_stdio(emit_func_t stdio_func);

int printf(const char *format, ...);

#ifdef __cplusplus
    }
#endif
