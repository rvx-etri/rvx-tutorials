#ifndef __ERVP_MATRIX_H__
#define __ERVP_MATRIX_H__

#include <stdint.h>
#include "ervp_matrix_datatype_define.h"
#include "ervp_printf.h"
#include "platform_info.h"

// #define MATRIX_DATATYPE_SINT01 GEN_MATRIX_DATATYPE(0,1,0) // IMPOSSIBLE
#define MATRIX_DATATYPE_UINT01 GEN_MATRIX_DATATYPE(0, 0, -3, 1)
#define MATRIX_DATATYPE_SINT02 GEN_MATRIX_DATATYPE(0, 1, -2, 2)
#define MATRIX_DATATYPE_UINT02 GEN_MATRIX_DATATYPE(0, 0, -2, 2)
#define MATRIX_DATATYPE_SINT04 GEN_MATRIX_DATATYPE(0, 1, -1, 4)
#define MATRIX_DATATYPE_UINT04 GEN_MATRIX_DATATYPE(0, 0, -1, 4)
#define MATRIX_DATATYPE_SINT08 GEN_MATRIX_DATATYPE(0, 1, 0, 8)
#define MATRIX_DATATYPE_UINT08 GEN_MATRIX_DATATYPE(0, 0, 0, 8)
#define MATRIX_DATATYPE_SINT16 GEN_MATRIX_DATATYPE(0, 1, 1, 16)
#define MATRIX_DATATYPE_UINT16 GEN_MATRIX_DATATYPE(0, 0, 1, 16)
#define MATRIX_DATATYPE_SINT32 GEN_MATRIX_DATATYPE(0, 1, 2, 32)
// #define MATRIX_DATATYPE_UINT32 GEN_MATRIX_DATATYPE(0,0,2,32) // NOT SUPPORTED
#define MATRIX_DATATYPE_FLOAT32 GEN_MATRIX_DATATYPE(1, 1, 2, 32)

typedef struct
{
	void *addr;
	int stride_ls3;
	int num_row;
	int num_col;
	int datatype;
	uint8_t is_array_allocated;
	uint8_t is_binary;
	uint8_t bit_offset;
} ErvpMatrixInfo;

ErvpMatrixInfo *matrix_generate_info(int datatype, int num_row, int num_col, void *array_1d, ErvpMatrixInfo *prealloacted);
ErvpMatrixInfo *matrix_generate_submatrix_info(const ErvpMatrixInfo *original_matrix, ErvpMatrixInfo *prealloacted);

static inline void matrix_set_stride(ErvpMatrixInfo *info, int stride)
{
	info->stride_ls3 = stride << 3;
}
static inline int matrix_get_stride(const ErvpMatrixInfo *info)
{
	return (info->stride_ls3) >> 3;
}

static inline int matrix_num_elements(const ErvpMatrixInfo *info)
{
	return info->num_row * info->num_col;
}

int matrix_num_bytes(const ErvpMatrixInfo *info);

ErvpMatrixInfo *matrix_alloc(int datatype, int num_row, int num_col, ErvpMatrixInfo *prealloacted);
void matrix_free(ErvpMatrixInfo *ptr);
void matrix_list_free(ErvpMatrixInfo **ptr, int num);

static inline int matrix_is_same_size(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b)
{
	int all_are_equal = 1;
	if (a->num_row != b->num_row)
		all_are_equal = 0;
	else if (a->num_col != b->num_col)
		all_are_equal = 0;
	return all_are_equal;
}

int matrix_compare(const ErvpMatrixInfo *result, const ErvpMatrixInfo *ref, int prints);
int matrix_compare_one_by_one(const ErvpMatrixInfo *result, const ErvpMatrixInfo *ref, float error_rate, int prints);

const char *matrix_datatype_get_name(int datatype);
void matrix_print_brief(const ErvpMatrixInfo *mat);
void matrix_print(const ErvpMatrixInfo *mat);
void matrix_print_hex_bit(const ErvpMatrixInfo *mat, int num_bits);
void matrix_print_hex(const ErvpMatrixInfo *mat);

static inline void matrix_mult_size_print(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, const ErvpMatrixInfo *c)
{
	printf("\n(%d x %d) X (%d x %d) = (%d x %d)", a->num_row, a->num_col, b->num_row, b->num_col, c->num_row, c->num_col);
}

static inline void matrix_conv_size_print(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, const ErvpMatrixInfo *c)
{
	printf("\n(%d x %d) * (%d x %d) = (%d x %d)", a->num_row, a->num_col, b->num_row, b->num_col, c->num_row, c->num_col);
}

#endif
