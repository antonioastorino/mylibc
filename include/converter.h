#ifndef CONVERTER_H
#define CONVERTER_H
#include "common.h"

Error str_to_int(const char* str_p, int*);
Error str_to_size_t(const char* str_p, size_t*);
Error str_to_float(const char* str_p, float*);
float rounder(float to_be_rounded, float step, size_t num_of_decimals);

#if TEST == 1
void test_converter(void);
#endif
#endif /* CONVERTER_H */
