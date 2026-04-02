#include <string.h>
#include "ervp_fakefile_system.h"
#include "ervp_round_int.h"
#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_malloc.h"
#include "ervp_memory_dump.h"
#include "ervp_variable_allocation.h"

static const int DEFAULT_CHUNK_SIZE = 1024;

static FAKEFILE* dict NOTCACHED_DATA = NULL;

void fakefile_print(FAKEFILE* fp, int num_char);

void fakefile_chunk_init_(fakefile_chunk_t* chunk)
{
  chunk->data = NULL;
  chunk->size = 0;
  chunk->current_size = 0;
  chunk->next = NULL;
}

fakefile_chunk_t *fakefile_chunk_malloc_()
{
  fakefile_chunk_t *chunk;
  chunk = malloc(sizeof(fakefile_chunk_t));
  assert(chunk);
  fakefile_chunk_init_(chunk);
  chunk->size = DEFAULT_CHUNK_SIZE;
  chunk->data = malloc(chunk->size * sizeof(uint8_t));
  assert(chunk->data);
  return chunk;
}

void fakefile_chunk_free_(fakefile_chunk_t* chunk)
{
  free(chunk->data);
  free(chunk);
}

void fakefile_init_(FAKEFILE* fp)
{
  fp->name = NULL;
  fp->head = NULL;
  fp->status = FILE_STATUS_MODIFIED;
  fp->mode = FILE_MODE_IDLE;
  fp->current = NULL;
  fp->read_index = 0;
}

void fakefile_free(FAKEFILE* fp)
{
  if(fp->status!=FILE_STATUS_READ_ONLY)
  {
    free(fp->name);
    fakefile_chunk_t* current = fp->head;
    while(1)
    {
      if(current==NULL)
        break;
      fakefile_chunk_t* next = current->next;
      fakefile_chunk_free_(current);
      current = next;
    }
  }
}

int fakefile_get_size(FAKEFILE* fp)
{
  int size;
  fakefile_chunk_t* chunk;
  assert(fp!=NULL);

  size = 0;
  chunk = fp->head;  
  while(chunk!=NULL)
  {
    size += chunk->current_size;
    chunk = chunk->next;
  }
  return size;
}

void fakefile_dict_add(FAKEFILE* fp)
{
  HASH_ADD_KEYPTR(hh, dict, fp->name, strlen(fp->name), fp);
}

void fakefile_dict_del(FAKEFILE* fp)
{
  HASH_DELETE(hh, dict, fp);
}

FAKEFILE* fakefile_dict_find(const char* name)
{
  FAKEFILE* fp;
  HASH_FIND_STR(dict, name, fp);
  return fp;
}

void fakefile_print_dict()
{
  FAKEFILE *fp, *temp;
  HASH_ITER(hh, dict, fp, temp)
  {
    fakefile_print(fp, -1);
  }
}

int ffgetc(FAKEFILE* fp);
int ffclose(FAKEFILE* fp);

void fakefile_prepare_dump()
{
  FAKEFILE *fp, *temp;
  int has_new_file = 0;
  int total_size = 0;
  HASH_ITER(hh, dict, fp, temp)
  {
    if(fp->status==FILE_STATUS_MODIFIED)
    {
      has_new_file = 1;
      total_size += 4; // file_size value
      total_size += (strlen(fp->name) + 1); // name
      total_size += fakefile_get_size(fp);
      total_size = round_up_int(total_size, 4);
    }
  }
  //printf("\ntotal_size: %d", total_size);
  if(has_new_file)
  {
    void* addr = malloc(total_size);
    assert(addr);
    void* pt = addr;
    HASH_ITER(hh, dict, fp, temp)
    {
      if(fp->status==FILE_STATUS_MODIFIED)
      {
        int file_size = fakefile_get_size(fp);
        *((uint32_t*)pt) = file_size;
        pt += 4;
        strcpy((char*)pt, fp->name);
        pt += (strlen(fp->name) + 1);
        fakefile_set_read_mode_(fp);
        for(int i=0; i<file_size; i++)
          *((uint8_t*)pt++) = ffgetc(fp);
        ffclose(fp);
        pt = (void*)round_up_int((int)pt, 4);
      }
    }
    assert((pt-addr)==total_size);
    memory_dump_prepare(MEMORY_DUMP_FAKEFILE, addr, total_size);
    //printf("\nfakefiles are ready to be dumped");
  }
}
