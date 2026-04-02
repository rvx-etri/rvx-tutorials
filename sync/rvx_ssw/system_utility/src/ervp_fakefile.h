#ifndef __ERVP_FAKEFILE_H__
#define __ERVP_FAKEFILE_H__

#include "ervp_fakefile_system.h"

#define EOF ((int)-1)

#define SEEK_SET (0)
#define SEEK_CUR (1)
#define SEEK_END (2)

FAKEFILE* ffopen(const char* name, const char* mode);
int ffputc(int c, FAKEFILE* fp);
int ffgetc(FAKEFILE* fp); // NOT char
int ffeof(FAKEFILE* fp);
int ffclose(FAKEFILE* fp);
char *ffgets(char *buf, int n, FAKEFILE *fp);
char *ffgetline(FAKEFILE *fp);
size_t ffread(void *buffer, size_t size, size_t count, FAKEFILE *fp);
size_t ffwrite(const void *buffer, size_t size, size_t count, FAKEFILE *fp);

int fftell(FAKEFILE *fp);
int ffseek(FAKEFILE *fp, long offset, int whence);

size_t fakefile_generate(void* raw_data, size_t size, const char* name);
void fakefile_print(FAKEFILE* fp, int num_char);
int fakefile_diff(FAKEFILE* a, FAKEFILE* b);
void fakefile_copy(FAKEFILE* src, FAKEFILE* dst);
int fakefile_exists(const char* name);

// fakefile_get_size in ervp_fakefile_system.h
int fakefile_get_size_by_name(const char* name);

#endif
