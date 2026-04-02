#include <string.h>

#include "ervp_matrix_element.h"
#include "ervp_matrix_to_csv.h"
#include "ervp_fakefile_csv.h"
#include "ervp_stdlib.h"

#include "ervp_printf.h"

static int has_matrix_csv_info(const char *filename)
{
  FAKEFILE *fp = ffopen(filename, "r");
  assert(fp);

  int has_info = 0;

  while (1)
  {
    int c = ffgetc(fp);
    if (c == EOF)
      break;
    if (c == 'E')
    {
      has_info = 1;
      break;
    }
    else if (c == ' ')
      continue;
    else
      break;
  }
  ffclose(fp);

  return has_info;
}

static void pass_info(FAKEFILE *fp)
{
  while (1)
  {
    int c = ffgetc(fp);
    if (c == EOF)
      break;
    if (c == '\n')
      break;
  }
}

static int analyze_matrix_csv_col_count(const char *filename, int has_info)
{
  FAKEFILE *fp = ffopen(filename, "r");
  assert(fp);

  if (has_info)
    pass_info(fp);

  int cols = 0;
  while (1)
  {
    int c = ffgetc(fp);
    if (c == EOF)
      break;
    if (c == '\n')
      break;
    if (c == ',')
    {
      if (cols == 0)
        cols = 2;
      else
        cols++;
    }
  }
  ffclose(fp);

  return cols;
}

static int analyze_matrix_csv_row_count(const char *filename, int has_info)
{
  FAKEFILE *fp = ffopen(filename, "r");
  assert(fp);

  if (has_info)
    pass_info(fp);

  int valid_line = 0;
  int rows = 0;

  while (1)
  {
    int c = ffgetc(fp);
    if (c == EOF)
      break;
    if (c == '\r')
      continue;
    else if (c == '\n')
    {
      if (valid_line)
        rows++;
      valid_line = 0;
    }
    else
      valid_line = 1;
  }
  ffclose(fp);
  {
    if (valid_line)
      rows++;
    valid_line = 0;
  }
  return rows;
}

static int get_matrix_csv_cols(const char *filename)
{
  return -1;
}

static int get_matrix_csv_rows(const char *filename)
{
  return -1;
}

static int get_matrix_csv_datatype(const char *filename)
{
  return -1;
}

static float read_matrix_csv_single_float(FAKEFILE *fp)
{
  char buf[128]; // float 하나 읽기 버퍼 (정수보다 여유롭게)
  int idx = 0;
  int ch;

  // 이전 공백, 개행 문자 무시
  do
  {
    ch = ffgetc(fp);
    if (ch == EOF)
      assert(0 && "Unexpected end of file while reading float element");
  } while (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');

  // 숫자(소수점, 부호, e/E 포함) 읽기
  while (ch != EOF && ch != ',' && ch != '\n' && ch != '\r')
  {
    if (idx < (int)(sizeof(buf) - 1))
      buf[idx++] = (char)ch;
    ch = ffgetc(fp);
  }

  buf[idx] = '\0';

  // 엘리먼트가 비어 있으면 assert
  assert(idx > 0 && "Empty element found in CSV");

  return atof(buf);
}

static int read_matrix_csv_single_integer(FAKEFILE *fp)
{
  char buf[64]; // 정수 하나 읽기 버퍼
  int idx = 0;
  int ch;

  // 이전 공백, 개행 문자 무시
  do
  {
    ch = ffgetc(fp);
    if (ch == EOF)
      assert(0 && "Unexpected end of file while reading integer element");
  } while (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');

  // 숫자 (또는 부호 포함) 읽기
  while (ch != EOF && ch != ',' && ch != '\n' && ch != '\r')
  {
    if (idx < (int)(sizeof(buf) - 1))
      buf[idx++] = (char)ch;
    ch = ffgetc(fp);
  }

  buf[idx] = '\0';

  // 엘리먼트가 비어 있으면 assert
  assert(idx > 0 && "Empty element found in CSV");

  return atoi(buf);
}

static ErvpMatrixInfo *read_matrix_csv_elements(const char *filename, int has_info, int rows, int cols, int datatype, int check_value_range)
{
  ErvpMatrixInfo *result = matrix_alloc(datatype, rows, cols, NULL);
  FAKEFILE *fp = ffopen(filename, "r");
  assert(fp);
  if (has_info)
    pass_info(fp);
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++)
    {
      if (matrix_datatype_is_float(datatype))
      {
        float value = read_matrix_csv_single_float(fp);
        matrix_write_float_element(result, i, j, value);
      }
      else
      {
        int value = read_matrix_csv_single_integer(fp);
        matrix_write_fixed_element(result, i, j, value);
        if (check_value_range)
          assert(matrix_read_fixed_element(result, i, j) == value);
      }
    }
  ffclose(fp);
  return result;
}

ErvpMatrixInfo *matrix_read_csv_file(const char *filename, int datatype_from_user, int check_value_range)
{
  assert(is_csv_file(filename));

  int has_info = has_matrix_csv_info(filename);

  int cols = analyze_matrix_csv_col_count(filename, has_info);
  int rows = analyze_matrix_csv_row_count(filename, has_info);

  if (has_info)
  {
    assert(cols == get_matrix_csv_cols(filename));
    assert(rows == get_matrix_csv_rows(filename));
  }

  int datatype;
  if (datatype_from_user == DATATYPE_FROM_INFO)
  {
    assert(has_info);
    datatype = get_matrix_csv_datatype(filename);
  }
  else
  {
    if (has_info)
      assert(datatype_from_user == get_matrix_csv_datatype(filename));
    datatype = datatype_from_user;
  }

  ErvpMatrixInfo *result = read_matrix_csv_elements(filename, has_info, rows, cols, datatype, check_value_range);
  return result;
}

void matrix_write_csv_file(const ErvpMatrixInfo *matrix, const char *filename, int data_only)
{
  assert(is_csv_file(filename));
  FAKEFILE *fp = ffopen(filename, "w");

  csv_index_t csv_index;
  ervp_fakefile_csv_index_init(&csv_index);

  // info
  if (data_only == 0)
  {
    const char filetype[] = "ERVP_MATRIX";
    ervp_fakefile_csv_write_element_to_row(filetype, fp, &csv_index);

    const char *datatype_name = matrix_datatype_get_name(matrix->datatype);
    ervp_fakefile_csv_write_element_to_col(datatype_name, fp, &csv_index);

    ervp_fakefile_csv_write_integer_to_col(matrix->num_row, fp, &csv_index);
    ervp_fakefile_csv_write_integer_to_col(matrix->num_col, fp, &csv_index);
  }

  // values
  for (int i = 0; i < matrix->num_row; i++)
  {
    for (int j = 0; j < matrix->num_col; j++)
    {
      if (matrix_datatype_is_float(matrix->datatype))
      {
        float value = matrix_read_float_element(matrix, i, j);
        if (j == 0)
          ervp_fakefile_csv_write_float_to_row(value, fp, &csv_index);
        else
          ervp_fakefile_csv_write_float_to_col(value, fp, &csv_index);
      }
      else
      {
        int value = matrix_read_fixed_element(matrix, i, j);
        if (j == 0)
          ervp_fakefile_csv_write_integer_to_row(value, fp, &csv_index);
        else
          ervp_fakefile_csv_write_integer_to_col(value, fp, &csv_index);
      }
    }
  }

  //
  assert(fp);
  ffclose(fp);
}