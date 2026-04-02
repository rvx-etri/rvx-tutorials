#include <string.h>
#include "ervp_fakefile_csv.h"

#include "ervp_assert.h"
#include "ervp_malloc.h"
#include "ervp_printf.h"

void ervp_fakefile_csv_index_init(csv_index_t *index)
{
  assert(index);
  index->row_index = 0;
  index->col_index = 0;
}

csv_index_t *ervp_fakefile_csv_index_alloc()
{
  csv_index_t *result = malloc(sizeof(csv_index_t));
  ervp_fakefile_csv_index_init(result);
  return result;
}

void ervp_fakefile_csv_index_free(csv_index_t *index)
{
  free(index);
}

int is_csv_file(const char *filename)
{
  const char *ext = strrchr(filename, '.');
  return (ext != NULL && strcmp(ext, ".csv") == 0);
}

static void next_col(FAKEFILE *fp)
{
  ffputc(',', fp);
}

static void next_row(FAKEFILE *fp)
{
  ffputc('\n', fp);
}

void ervp_fakefile_csv_write_element_to_row(const char *element, FAKEFILE *fp, csv_index_t *index_to_write)
{
  if (index_to_write->col_index != 0)
  {
    index_to_write->row_index++;
    next_row(fp);
  }
  index_to_write->col_index = 1;
  ffwrite(element, sizeof(char), strlen(element), fp);
}

void ervp_fakefile_csv_write_element_to_col(const char *element, FAKEFILE *fp, csv_index_t *index_to_write)
{
  if (index_to_write->col_index != 0)
    next_col(fp);
  index_to_write->col_index++;
  ffwrite(element, sizeof(char), strlen(element), fp);
}

void ervp_fakefile_csv_write_integer_to_row(int value, FAKEFILE *fp, csv_index_t *index_to_write)
{
  char number_buffer[32];
  sprintf_rvx(number_buffer, "%d", value);
  ervp_fakefile_csv_write_element_to_row(number_buffer, fp, index_to_write);
}

void ervp_fakefile_csv_write_integer_to_col(int value, FAKEFILE *fp, csv_index_t *index_to_write)
{
  char number_buffer[32];
  sprintf_rvx(number_buffer, "%d", value);
  ervp_fakefile_csv_write_element_to_col(number_buffer, fp, index_to_write);
}

void ervp_fakefile_csv_write_float_to_row(float value, FAKEFILE *fp, csv_index_t *index_to_write)
{
  char number_buffer[32];
  sprintf_rvx(number_buffer, "%f", value);
  ervp_fakefile_csv_write_element_to_row(number_buffer, fp, index_to_write);
}

void ervp_fakefile_csv_write_float_to_col(float value, FAKEFILE *fp, csv_index_t *index_to_write)
{
  char number_buffer[32];
  sprintf_rvx(number_buffer, "%f", value);
  ervp_fakefile_csv_write_element_to_col(number_buffer, fp, index_to_write);
}