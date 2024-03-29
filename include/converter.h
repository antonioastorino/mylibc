#ifndef CONVERTER_H
#define CONVERTER_H
#include "common.h"
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    Error str_to_int(const char* str_p, int*);
    Error str_to_size_t(const char* str_p, size_t*);
    Error str_to_uint8_t(const char* str_p, uint8_t*);
    Error str_to_float(const char* str_p, float*);
    float rounder(float to_be_rounded, float step, size_t num_of_decimals);

#if TEST == 1
    void test_converter(void);
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CONVERTER_H */
