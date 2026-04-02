#ifndef __ERVP_IMAGE_H__
#define __ERVP_IMAGE_H__

#include <stdint.h>
#include "ervp_image_format_define.h"
#include "ervp_color_format.h"
#include "ervp_matrix.h"

#define IMAGE_FMT_YUV_420_PLANAR_YV12 GEN_IMAGE_FORMAT(0,COLOR_FORMAT_YUV,3,0)
#define IMAGE_FMT_YUV_420_PLANAR_NV12 GEN_IMAGE_FORMAT(1,COLOR_FORMAT_YUV,2,0)
#define IMAGE_FMT_RGB_888_PLANE GEN_IMAGE_FORMAT(2,COLOR_FORMAT_RGB,3,0)
#define IMAGE_FMT_XBGR_8888_PACKED GEN_IMAGE_FORMAT(3,COLOR_FORMAT_RGB,1,1)
#define IMAGE_FMT_RGB_888_PACKED GEN_IMAGE_FORMAT(4,COLOR_FORMAT_RGB,1,1)
#define IMAGE_FMT_RGB_565_PACKED GEN_IMAGE_FORMAT(5,COLOR_FORMAT_RGB,1,1)
#define IMAGE_FMT_YUV_422_PACKED GEN_IMAGE_FORMAT(6,COLOR_FORMAT_YUV,1,1)
#define IMAGE_FMT_Y_ONLY GEN_IMAGE_FORMAT(7,COLOR_FORMAT_SPECIAL,1,0)
#define IMAGE_FMT_JPEG GEN_IMAGE_FORMAT(8,COLOR_FORMAT_SPECIAL,0,1)
#define IMAGE_FMT_JPEGLS GEN_IMAGE_FORMAT(9,COLOR_FORMAT_SPECIAL,0,1)
#define IMAGE_FMT_BMP GEN_IMAGE_FORMAT(10,COLOR_FORMAT_SPECIAL,0,1)

typedef struct window_info ervp_image_window_info_t;

typedef struct {
	unsigned int format;
	int width;
	int height;

  int is_window;
  ervp_image_window_info_t* window_info;
	void *addr[3];
	unsigned int stride[3];
  int file_size; // jpeg and bmp
} ErvpImage;

typedef struct window_info{
  ErvpImage* original;
  int row_index;
  int col_index;
} ervp_image_window_info_t;

void image_init_structure(ErvpImage* image, int width, int height, unsigned int format);
void image_decide_stride(ErvpImage* image, int is_aligned);
void image_alloc_internals(ErvpImage* image, int width, int height, unsigned int format, int is_aligned, int is_permanent);
ErvpImage* image_alloc_wo_internals(int width, int height, unsigned int format);
ErvpImage* image_alloc_option(int width, int height, unsigned int format, int is_aligned, int is_permanent);
static inline ErvpImage* image_alloc(int width, int height, unsigned int format)
{
  return image_alloc_option(width, height, format, 0, 0);
}
static inline void image_init(ErvpImage* image, int width, int height, unsigned int format, int is_aligned)
{
	image_init_structure(image, width, height, format);
	image_decide_stride(image, is_aligned);
	image_alloc_internals(image, width, height, format, is_aligned, 0);
}

void image_free_internals(ErvpImage* image);
void image_free(ErvpImage* image);
//ErvpImage* resize(ErvpImage* original, ErvpImage* preallocated);
void image_print_info(const ErvpImage* const image);
void image_copy(const ErvpImage* const src, ErvpImage* dst, int row_index, int col_index);
void image_prepare_dump(const ErvpImage *image); // deprecated

int image_is_same_size(const ErvpImage* const left, const ErvpImage* const right);
int image_is_same_type(const ErvpImage* const left, const ErvpImage* const right);

static inline void* image_get_row_base_raw(const ErvpImage* const image, int plane_index, int row_index)
{
  return image->addr[plane_index] + row_index*image->stride[plane_index];
}

const char* image_format_get_name(unsigned int format);
void* image_get_row_base(const ErvpImage* const image, int plane_index, int row_index);
int image_get_macro_pixel_size(const ErvpImage* const image, int plane_index);
void* image_get_macro_pixel_addr(const ErvpImage* const image, int plane_index, int row_index, int col_index);
void* image_get_pixel_addr(const ErvpImage* const image, int plane_index, int row_index, int col_index);

int image_get_plane_height(const ErvpImage* image, int plane_index);

void image_clear(ErvpImage* image, unsigned int option_value);

ErvpImage* image_convert_format(const ErvpImage* const original, ErvpImage* preallocated, unsigned int format);

ErvpImage* image_generate_window(const ErvpImage* const image, int width, int height, int row_index, int col_index, ErvpImage* preallocated);
ErvpImage* image_generate_center_window(const ErvpImage* const image, int width, int height, ErvpImage* preallocated);

ErvpMatrixInfo* image_generate_matrix_info(const ErvpImage* const image, int plane_index, int row_index, int col_index);

#endif
