#ifndef __ERVP_FAKEFILE_SYSTEM_H__
#define __ERVP_FAKEFILE_SYSTEM_H__

#include <stdint.h>
#include "uthash.h"

#include "ervp_assert.h"

typedef struct fakefile_chunk {
  uint8_t* data;
  int size;
  int current_size;
  struct fakefile_chunk *next;
} fakefile_chunk_t;

typedef struct {
  const char* name;
  fakefile_chunk_t* head;
  int status;
  int mode;
  fakefile_chunk_t* current;
  int read_index;
  UT_hash_handle hh;
} FAKEFILE;

static const int FILE_STATUS_MODIFIED = 0;
static const int FILE_STATUS_READ_ONLY = 1;

static const int FILE_MODE_IDLE = 0;
static const int FILE_MODE_READ = 1;
static const int FILE_MODE_WRITE = 2;

void fakefile_chunk_init_(fakefile_chunk_t* chunk);
fakefile_chunk_t *fakefile_chunk_malloc_();
void fakefile_chunk_free_(fakefile_chunk_t* chunk);

void fakefile_init_(FAKEFILE* fp);
void fakefile_free(FAKEFILE* fp);
int fakefile_get_size(FAKEFILE* fp);

static inline void fakefile_check_read_mode(FAKEFILE* fp)
{
  assert(fp!=NULL);
  assert_msg(fp->mode==FILE_MODE_READ, "%s", fp->name);
}

static inline void fakefile_check_write_mode(FAKEFILE* fp)
{
  assert(fp!=NULL);
  assert_msg(fp->mode==FILE_MODE_WRITE, "%s", fp->name);
}

static inline void fakefile_set_read_mode_(FAKEFILE* fp)
{
  assert(fp!=NULL);
  assert_msg(fp->mode==FILE_MODE_IDLE, "%s", fp->name);
  fp->mode = FILE_MODE_READ;
  fp->current = fp->head;
  fp->read_index = 0;
}

void fakefile_dict_add(FAKEFILE* fp);
void fakefile_dict_del(FAKEFILE* fp);

FAKEFILE* fakefile_dict_find(const char* name);
void fakefile_print_dict();
void fakefile_prepare_dump();

#endif
