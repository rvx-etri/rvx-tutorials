#ifndef __ERVP_MATRIX_TO_CSV_H__
#define __ERVP_MATRIX_TO_CSV_H__

#include "ervp_matrix.h"

static const int DATATYPE_FROM_INFO = -1;

ErvpMatrixInfo *matrix_read_csv_file(const char *filename, int datatype, int check_value_range);
void matrix_write_csv_file(const ErvpMatrixInfo *matrix, const char *filename, int data_only);

#endif