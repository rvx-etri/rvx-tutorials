#ifndef __ERVP_TENSOR_H__
#define __ERVP_TENSOR_H__

#include <stdint.h>
#include "platform_info.h"

typedef struct {
	void *addr;
	int *size;
	int *stride; // stride[0] is data size of value.
	int num_dim;
	//int datatype;
} ErvpTensorInfo;


ErvpTensorInfo *tensor_make(int num_dim);
void tensor_alloc(ErvpTensorInfo *a);
int tensor_num_value(const ErvpTensorInfo *a);

void tensor_reshape(const ErvpTensorInfo *a, ErvpTensorInfo *b);
ErvpTensorInfo *tensor_permute(const ErvpTensorInfo *a, ErvpTensorInfo *b, int *dims);

#endif
