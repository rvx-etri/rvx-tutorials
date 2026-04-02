#ifndef __ERVP_IMAGE_UTIL_H__
#define __ERVP_IMAGE_UTIL_H__

#include "ervp_matrix_op.h"
#include "ervp_image.h"
#include "ervp_matrix.h"

ErvpImage* image_convert_grayscale(const ErvpImage* const original, ErvpImage* preallocated);
ErvpImage* image_convert_bmp(const ErvpImage* const original, ErvpImage* preallocated);
ErvpImage* image_resize(const ErvpImage* const original, int width, int height, ErvpImage* preallocated);
ErvpImage* image_filter(ervp_mop_mapping_t* mop_mapping, const ErvpImage* const original, const ErvpMatrixInfo* const filter, ErvpImage* preallocated);

#endif