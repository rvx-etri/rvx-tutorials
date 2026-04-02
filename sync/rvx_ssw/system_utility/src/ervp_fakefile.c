#include <string.h>
#include <limits.h>

#include "ervp_fakefile.h"
#include "ervp_malloc.h"
#include "ervp_assert.h"
#include "ervp_printf.h"
#include "ervp_round_int.h"

static int ffputc_(int c, FAKEFILE *fp);
static int ffgetc_(FAKEFILE *fp);
static int ffeof_(FAKEFILE *fp);

int fakefile_exists(const char *name)
{
  FAKEFILE *fp = fakefile_dict_find(name);
  return (fp != NULL);
}

FAKEFILE *ffopen(const char *name, const char *mode)
{
#ifndef USE_FAKEFILE
  assert_must_msg(0, "USE_FAKEFILE is NOT enabled in \'app_name/rvx_each.mh\'");
#endif
  FAKEFILE *fp;
  fp = fakefile_dict_find(name);
  if (strcmp(mode, "r") == 0)
  {
    assert_msg(fp != NULL, "No file exist (%s)", name);
    fakefile_set_read_mode_(fp);
  }
  else if (strcmp(mode, "w") == 0)
  {
    if (fp != NULL)
    {
      fakefile_free(fp);
      fakefile_dict_del(fp);
    }
    fp = malloc(sizeof(FAKEFILE));
    assert(fp);
    fakefile_init_(fp);
    fp->name = malloc(strlen(name) + 1);
    strcpy(fp->name, name);
    fakefile_dict_add(fp);
    fp->mode = FILE_MODE_WRITE;
  }
  else if (strcmp(mode, "a") == 0)
  {
    if (fp == NULL)
    {
      fp = malloc(sizeof(FAKEFILE));
      assert(fp);
      fakefile_init_(fp);
      fp->name = name;
      fakefile_dict_add(fp);
    }
    else
    {
      fp->status = FILE_STATUS_MODIFIED;
      fp->current = fp->head;
      if (fp->current != NULL)
      {
        while (1)
        {
          if (fp->current->next == NULL)
            break;
          else
            fp->current = fp->current->next;
        }
      }
    }
    fp->mode = FILE_MODE_WRITE;
  }
  else
    assert(0);
  return fp;
}

int ffclose(FAKEFILE *fp)
{
  assert(fp->mode != FILE_MODE_IDLE);
  fp->mode = FILE_MODE_IDLE;
  fp->current = NULL;
  fp->read_index = 0;
  return 0;
}

static inline int ff_get_current_char(const FAKEFILE *fp)
{
  int result = EOF;
  assert(fp != NULL);
  if (fp->current != NULL)
    result = fp->current->data[fp->read_index];
  return result;
}

static int ffputc_(int c, FAKEFILE *fp)
{
  if (fp->current == NULL)
  {
    fp->head = fakefile_chunk_malloc_();
    fp->current = fp->head;
  }
  else if (fp->current->current_size == fp->current->size)
  {
    fakefile_chunk_t *chunk = fakefile_chunk_malloc_();
    fp->current->next = chunk;
    fp->current = chunk;
  }
  fp->current->data[fp->current->current_size++] = c;
  return 0;
}

int ffputc(int c, FAKEFILE *fp)
{
  fakefile_check_write_mode(fp);
  return ffputc_(c, fp);
}

static int ffgetc_(FAKEFILE *fp)
{
  int result = EOF;
  if (!ffeof_(fp))
  {
    result = fp->current->data[fp->read_index++];
    if (fp->read_index == fp->current->current_size)
    {
      fp->current = fp->current->next;
      fp->read_index = 0;
    }
  }
  return result;
}

int ffgetc(FAKEFILE *fp)
{
  fakefile_check_read_mode(fp);
  return ffgetc_(fp);
}

static int ffeof_(FAKEFILE *fp)
{
  int result;
  if (fp->current == NULL)
    result = 1;
  else
    result = 0;
  return result;
}

int ffeof(FAKEFILE *fp)
{
  fakefile_check_read_mode(fp);
  return ffeof_(fp);
}

char *ffgets(char *buf, int n, FAKEFILE *fp)
{
  size_t len = 0;
  char *s;
  unsigned char *p, *t;
  if (n <= 0) /* sanity check */
    return (NULL);
  if (ffeof(fp))
  {
    return (NULL);
  }

  s = buf;
  n--; /* leave space for NUL */
  while (n != 0)
  {
    char tmp = (char)ffgetc(fp);
    *s = tmp;
    s++;
    n--;

    if ((tmp == '\n') || (ffeof(fp) == 1))
    {
      break;
    }
  }
  *s = '\0';
  return (buf);
}

char *ffgetline(FAKEFILE *fp)
{
  if (ffeof(fp))
    return 0;
  size_t size = 512;
  char *line = (char *)malloc(size * sizeof(char));
  assert(line);
  if (!ffgets(line, size, fp))
  {
    free(line);
    return 0;
  }

  size_t curr = strlen(line);

  while ((line[curr - 1] != '\n') && !ffeof(fp))
  {
    if (curr == size - 1)
    {
      size *= 2;
      line = (char *)realloc(line, size * sizeof(char));
    }
    size_t readsize = size - curr;
    if (readsize > INT_MAX)
      readsize = INT_MAX - 1;
    ffgets(&line[curr], readsize, fp);
    curr = strlen(line);
  }
  if (curr >= 2)
    if (line[curr - 2] == 0x0d)
      line[curr - 2] = 0x00;

  if (curr >= 1)
    if (line[curr - 1] == 0x0a)
      line[curr - 1] = 0x00;

  return line;
}

int fakefile_diff(FAKEFILE *a, FAKEFILE *b)
{
  fakefile_check_read_mode(a);
  fakefile_check_read_mode(b);
  int diff = 0;
  while ((!ffeof_(a)) && (!ffeof_(b)))
  {
    if (ffgetc_(a) != ffgetc_(b))
    {
      diff = 1;
      break;
    }
  }
  if (diff == 0)
  {
    if (ffeof_(a) && ffeof_(b))
      ;
    else
    {
      diff = 1;
    }
  }
  return diff;
}

void fakefile_print(FAKEFILE *fp, int num_char)
{
  fakefile_check_read_mode(fp);
  printf("\n\"%s\"", fp->name);
  for (int i = 0; i != num_char; i++)
  {
    if (!ffeof_(fp))
    {
      if ((i & 0x7) == 0)
        printf("\n");
      printf(" 0x%02x", ffgetc_(fp));
    }
  }
}

void fakefile_copy(FAKEFILE *src, FAKEFILE *dst)
{
  fakefile_check_read_mode(src);
  fakefile_check_write_mode(dst);
  while (1)
  {
    int ch = ffgetc_(src);
    if (ch == EOF)
      break;
    ffputc_(ch, dst);
  }
}

size_t ffread(void *buffer, size_t size, size_t count, FAKEFILE *fp)
{
  assert(buffer);
  fakefile_check_read_mode(fp);
  size_t num_bytes = size * count;
  uint8_t *ptr = (uint8_t *)buffer;
  for (int i = 0; i < num_bytes; i++)
  {
    int ch = ffgetc_(fp);
    if (ch == EOF)
      break;
    *ptr++ = ch;
  }
  size_t num_read_bytes = ((int)ptr) - ((int)buffer);
  size_t num_read = round_down_int(num_read_bytes, size);
  return num_read;
}

size_t ffwrite(const void *buffer, size_t size, size_t count, FAKEFILE *fp)
{
  assert(buffer);
  fakefile_check_write_mode(fp);
  uint8_t *ptr = (uint8_t *)buffer;
  size_t num_bytes = size * count;
  assert(fp != NULL);
  for (int i = 0; i < num_bytes; i++)
    ffputc_(ptr[i], fp);
  return count;
}

int fftell(FAKEFILE *fp)
{
  int offset;
  fakefile_chunk_t *chunk;
  assert(fp != NULL);

  offset = 0;
  chunk = fp->head;
  while (chunk != fp->current)
  {
    offset += chunk->current_size;
    chunk = chunk->next;
  }

  offset += fp->read_index;

  return offset;
}

static inline int fakefile_set_current_offset(FAKEFILE *fp, long current_offset)
{
  assert(fp != NULL);
  assert(current_offset >= 0);

  fp->current = fp->head;
  while (fp->current != NULL)
  {
    if (current_offset < fp->current->current_size)
    {
      break;
    }
    current_offset -= fp->current->current_size;
    fp->current = fp->current->next;
  }

  fp->read_index = current_offset;

  return 0;
}

int ffseek(FAKEFILE *fp, long offset, int whence)
{
  long curoff;

  int max_offset = fakefile_get_size(fp);

  switch (whence)
  {
  case SEEK_CUR:
    curoff = fftell(fp);
    curoff += offset;
    break;
  case SEEK_SET:
    curoff = offset;
    break;
  case SEEK_END:
    curoff = max_offset + offset;
    break;
  default:
    return -1;
  }

  if (curoff < 0)
  {
    return -1;
  }
  else if (curoff >= max_offset)
  {
    fp->read_index = curoff - max_offset;
    fp->current = NULL;
  }
  else
  {
    fakefile_set_current_offset(fp, curoff);
  }

  return 0;
}

size_t fakefile_generate(void *raw_data, size_t size, const char *name)
{
  FAKEFILE *result;
  fakefile_chunk_t *fakefile_chunk = malloc(sizeof(fakefile_chunk_t));
  assert(fakefile_chunk);
  fakefile_chunk_init_(fakefile_chunk);
  fakefile_chunk->data = (uint8_t *)raw_data;
  fakefile_chunk->size = size;
  fakefile_chunk->current_size = size;
  result = (FAKEFILE *)malloc(sizeof(FAKEFILE));
  assert(result);
  fakefile_init_(result);
  result->status = FILE_STATUS_MODIFIED;
  result->name = name;
  result->head = fakefile_chunk;
  fakefile_dict_add(result);
  return size;
};

int fakefile_get_size_by_name(const char *name)
{
  FAKEFILE *fp = ffopen(name, "r");
  int size = fakefile_get_size(fp);
  ffclose(fp);
  return size;
}