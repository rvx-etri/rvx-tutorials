#ifndef __ERVP_MEMORY_UTIL_H__
#define __ERVP_MEMORY_UTIL_H__

#include <stddef.h>
#include <stdint.h>

void print_memory_byte(uint8_t* array, int num);
void print_memory_4byte(uint32_t* array, int num);
int memory_compare(const void* result, const void* ref, size_t num, int prints);

#define memcpy memcpy_rvx
void* memcpy_rvx(void* dst, const void* src, size_t num);

#define memset memset_rvx
void* memset_rvx(void* ptr, int value, size_t num);

#endif
