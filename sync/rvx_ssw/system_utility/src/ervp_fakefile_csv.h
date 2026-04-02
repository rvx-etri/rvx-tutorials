#ifndef __ERVP_FAKEFILE_CSV_H__
#define __ERVP_FAKEFILE_CSV_H__

#include "ervp_fakefile.h"

typedef struct csv_index
{
  int row_index;
  int col_index;
} csv_index_t;

void ervp_fakefile_csv_index_init(csv_index_t *index);
csv_index_t *ervp_fakefile_csv_index_alloc();
void ervp_fakefile_csv_index_free(csv_index_t *index);

int is_csv_file(const char *filename);

void ervp_fakefile_csv_write_element_to_row(const char *element, FAKEFILE *fp, csv_index_t *index_to_write);
void ervp_fakefile_csv_write_element_to_col(const char *element, FAKEFILE *fp, csv_index_t *index_to_write);

void ervp_fakefile_csv_write_integer_to_row(int value, FAKEFILE *fp, csv_index_t *index_to_write);
void ervp_fakefile_csv_write_integer_to_col(int value, FAKEFILE *fp, csv_index_t *index_to_write);

void ervp_fakefile_csv_write_float_to_row(float value, FAKEFILE *fp, csv_index_t *index_to_write);
void ervp_fakefile_csv_write_float_to_col(float value, FAKEFILE *fp, csv_index_t *index_to_write);

#endif