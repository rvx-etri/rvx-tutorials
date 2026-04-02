#ifndef __ERVP_MEMORY_DUMP_H__
#define __ERVP_MEMORY_DUMP_H__

static const int MEMORY_DUMP_NONE = 0;
static const int MEMORY_DUMP_FAKEFILE = 1;
static const int MEMORY_DUMP_IMAGE = 2;

void memory_dump_init();
void memory_dump_prepare(int dump_type, void* addr, int size);

#endif
