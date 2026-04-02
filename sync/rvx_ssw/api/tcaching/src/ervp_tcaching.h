#ifndef __ERVP_TCACHING_H__
#define __ERVP_TCACHING_H__

#include <stdio.h>

void tcaching_init();
void *tcaching_malloc(void *pt, size_t size);
void tcaching_free(void *tcaching_pt);
void tcaching_flush();

#endif
