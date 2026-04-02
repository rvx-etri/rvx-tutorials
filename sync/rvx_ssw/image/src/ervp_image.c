#include <stdint.h>
#include "ervp_image.h"
#include "ervp_printf.h"
#include "ervp_malloc.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_assert.h"
#include "ervp_memory_util.h"
#include "ervp_memory_dump.h"
#include "ervp_round_int.h"
#include "ervp_image_util.h"

#define ALIGN1K(x)     ((((x) + 1023)>>10)<<10)

void image_init_structure(ErvpImage* image, int width, int height, unsigned int format)
{
	image->width = width;
	image->height = height;
	image->format = format;
  image->is_window = 0;
  image->window_info=NULL;
  image->file_size = 0;

  image->stride[0] = 0;
  image->stride[1] = 0;
  image->stride[2] = 0;
  image->addr[0] = 0;
  image->addr[1] = 0;
  image->addr[2] = 0;
}

int image_get_macro_pixel_size(const ErvpImage* const image, int plane_index)
{
  int size = 0;
  switch(image->format)
  {
    case IMAGE_FMT_YUV_420_PLANAR_YV12:
      if(plane_index==0)
        size = 1;
      else if(plane_index==1)
        size = 1;
      else if(plane_index==2)
        size = 1;
      break;
    case IMAGE_FMT_YUV_420_PLANAR_NV12:
      if(plane_index==0)
        size = 1;
      else if(plane_index==1)
        size = 2;
      else if(plane_index==2)
        ;
      break;
    case IMAGE_FMT_RGB_888_PLANE:
      if(plane_index==0)
        size = 1;
      else if(plane_index==1)
        size = 1;
      else if(plane_index==2)
        size = 1;
      break;
    case IMAGE_FMT_XBGR_8888_PACKED:
      if(plane_index==0)
        size = 4;
      else if(plane_index==1)
        ;
      else if(plane_index==2)
        ;
      break;
    case IMAGE_FMT_RGB_888_PACKED:
      if(plane_index==0)
        size = 3;
      else if(plane_index==1)
        ;
      else if(plane_index==2)
        ;
      break;
    case IMAGE_FMT_RGB_565_PACKED:
      if(plane_index==0)
        size = 2;
      else if(plane_index==1)
        ;
      else if(plane_index==2)
        ;
      break;
    case IMAGE_FMT_Y_ONLY:
      if(plane_index==0)
        size = 1;
      else if(plane_index==1)
        ;
      else if(plane_index==2)
        ;
      break;
  }
  assert_msg(size!=0, "\n%p %d %d", image, image->format, plane_index);
  return size;
}


static const char* image_format_name_dict[] = {
  "YUV_420_PLANAR_YV12",
	"YUV_420_PLANAR_NV12",
	"RGB_888_PLANE",
	"XBGR_8888_PACKED",
	"RGB_888_PACKED",
	"RGB_565_PACKED",
	"YUV_422_PACKED",
  "Y_ONLY",
  "JPEG",
	"JPEGLS",
	"BMP"
};

const char* image_format_get_name(unsigned int format)
{
  int id = image_get_id(format);
  assert_msg(id<=image_get_id(IMAGE_FMT_BMP), "%s", image_format_get_name(format));
  return image_format_name_dict[id];
}

static inline int image_get_plane_size_rsa(const ErvpImage* const image, int plane_index)
{
 int rsa = -1;
  switch(image->format)
  {
    case IMAGE_FMT_YUV_420_PLANAR_YV12:
      if(plane_index==0)
        rsa = 0;
      else if(plane_index==1)
        rsa = 1;
      else if(plane_index==2)
        rsa = 1;
      break;
    case IMAGE_FMT_YUV_420_PLANAR_NV12:
      if(plane_index==0)
        rsa = 0;
      else if(plane_index==1)
        rsa = 1;
      else if(plane_index==2)
        ;
      break;
    case IMAGE_FMT_RGB_888_PLANE:
      if(plane_index==0)
        rsa = 0;
      else if(plane_index==1)
        rsa = 0;
      else if(plane_index==2)
        rsa = 0;
      break;
    case IMAGE_FMT_XBGR_8888_PACKED:
      if(plane_index==0)
        rsa = 0;
      else if(plane_index==1)
        ;
      else if(plane_index==2)
        ;
      break;
    case IMAGE_FMT_RGB_888_PACKED:
      if(plane_index==0)
        rsa = 0;
      else if(plane_index==1)
        ;
      else if(plane_index==2)
        ;
      break;
    case IMAGE_FMT_RGB_565_PACKED:
      if(plane_index==0)
        rsa = 0;
      else if(plane_index==1)
        ;
      else if(plane_index==2)
        ;
      break;
    case IMAGE_FMT_Y_ONLY:
      if(plane_index==0)
        rsa = 0;
      else if(plane_index==1)
        ;
      else if(plane_index==2)
        ;
      break;
  }
  assert_msg(rsa!=(-1), "\n%p %d %d", image, image->format, plane_index);
  return rsa;
}

int image_get_plane_height(const ErvpImage* image, int plane_index)
{
  int height = 0;
  assert(image);
  height = image->height >> image_get_plane_size_rsa(image, plane_index);
  assert_msg(height!=0, "\n%p %d %d", image, image->format, plane_index);
  return height;
}

static inline int image_get_plane_width(const ErvpImage* image, int plane_index)
{
  int width = 0;
  assert(image);
  width = image->width >> image_get_plane_size_rsa(image, plane_index);
  assert_msg(width!=0, "\n%p %d %d", image, image->format, plane_index);
  return width;
}

static inline int image_get_plane_width_size(const ErvpImage* image, int plane_index)
{
  return image_get_plane_width(image,plane_index)*image_get_macro_pixel_size(image, plane_index);
}

static inline int image_cal_stride(const ErvpImage* image, int plane_index, int is_aligned)
{
  int stride;
  assert(image);
  stride = image_get_plane_width_size(image, plane_index);
  assert_msg(stride!=0, "\n%p %d %d", image, image->format, plane_index);
  if(is_aligned)
    stride = round_up_power2(stride);
  return stride;
}

void* image_get_row_base(const ErvpImage* const image, int plane_index, int row_index)
{
  int row_index_cliped = CLIP(row_index, 0, image->height-1);
  int plane_row_index = row_index_cliped >> image_get_plane_size_rsa(image, plane_index);
  return image_get_row_base_raw(image, plane_index, plane_row_index);
}

ErvpImage* image_alloc_wo_internals(int width, int height, unsigned int format)
{
  ErvpImage* image;
  image = (ErvpImage*)malloc(sizeof(ErvpImage));
  assert(image);
  image_init_structure(image, width, height, format);
  return image;
}

void image_decide_stride(ErvpImage* image, int is_aligned)
{
  for(int i=0; i<image_get_num_plane(image->format); i++)
    image->stride[i] = image_cal_stride(image, i, is_aligned);
}

void image_alloc_internals(ErvpImage* image, int width, int height, unsigned int format, int is_aligned, int is_permanent)
{
  ervp_malloc_option_t option;
  option.value = 0;
  option.br.is_permanent = is_permanent;
  for(int i=0; i<image_get_num_plane(image->format); i++)
  {
    int align_size = 0;
    if(is_aligned)
      align_size = image->stride[i];
    image->addr[i] = malloc_ext(image->stride[i] * image_get_plane_height(image, i), align_size, option.value);
    assert(image->addr[i]);
  }
}

ErvpImage* image_alloc_option(int width, int height, unsigned int format, int is_aligned, int is_permanent)
{
  ErvpImage* image;
  image = image_alloc_wo_internals(width, height, format);
  switch(format)
  {
    case IMAGE_FMT_JPEG:
    case IMAGE_FMT_JPEGLS:
    case IMAGE_FMT_BMP:
      image->addr[0] = malloc(image->width* image->height);
      assert(image->addr[0]);
      break;
    default:
      image_decide_stride(image, is_aligned);
      image_alloc_internals(image, width, height, format, is_aligned, is_permanent);
      break;
  }
  return image;
}

void image_free_internals(ErvpImage* image)
{
  assert(image);
  if(!(image->is_window))
    for(int i=0; i<3; i++)
    {
      void* addr = image->addr[i];
      if(addr!=NULL)
        free(addr);
    }
}

void image_free(ErvpImage* image)
{
  assert(image);
  image_free_internals(image);
  free(image);
}

ErvpImage* conv_yuv422_to_rgb565(const ErvpImage* const yuv422, ErvpImage* rgb565)
{
  int i, j;
  int width = yuv422->width;
  int height = yuv422->height;
  uint8_t y, u, v;
  uint8_t r, g, b;
  uint8_t *p_yuv422;
  uint16_t *p_rgb565;
  //printf("image size %d / %d\n", width, height);

  assert(yuv422!=NULL);
  assert(yuv422->format==IMAGE_FMT_YUV_422_PACKED);
  assert(!yuv422->is_window);

  if(rgb565==NULL)
    rgb565 = image_alloc(width, height, IMAGE_FMT_RGB_565_PACKED);
  assert(rgb565->format==IMAGE_FMT_RGB_565_PACKED);

  p_yuv422 = (uint8_t *)yuv422->addr[0];
  p_rgb565 = (uint16_t *)rgb565->addr[0];

  for(j=0; j<height; j++)
  {
    for(i=0; i<width; i++)
    {
      y = p_yuv422[(i<<1) + 1]; // Y
      u = p_yuv422[(i>>1)<<2]; // U
      v = p_yuv422[((i>>1)<<2) + 2]; // V
      r = extract_r_from_yuv(y, u, v);
      g = extract_g_from_yuv(y, u, v);
      b = extract_b_from_yuv(y, u, v);
      p_rgb565[i] = gen_rgb565(r, g, b);
    }
    p_yuv422 += width*2;
    p_rgb565 += width;
  }

  return rgb565;
}

ErvpImage* conv_yuv422_to_xbgr8888(const ErvpImage* const yuv422, ErvpImage* xbgr8888)
{
  int i, j;
  int width = yuv422->width;
  int height = yuv422->height;
  uint8_t y, u, v;
  uint8_t r, g, b;
  uint8_t *p_yuv422;
  uint8_t *p_xbgr8888;
  uint8_t *p_xbgr8888_base;

  assert(yuv422!=NULL);
  assert(yuv422->format==IMAGE_FMT_YUV_422_PACKED);
  assert(!yuv422->is_window);

  if(xbgr8888==NULL)
    xbgr8888 = image_alloc(width, height, IMAGE_FMT_XBGR_8888_PACKED);
  assert(xbgr8888->format==IMAGE_FMT_XBGR_8888_PACKED);

  p_yuv422 = (uint8_t *)yuv422->addr[0];
  p_xbgr8888_base = (uint8_t *)xbgr8888->addr[0];
  //printf("stride %d\n", xbgr8888->stride[0]);

  for(j=0; j<height; j++)
  {
    p_xbgr8888 = p_xbgr8888_base;
    for(i=0; i<width; i++)
    {
      y = p_yuv422[(i<<1) + 1]; // Y
      u = p_yuv422[(i>>1)<<2]; // U
      v = p_yuv422[((i>>1)<<2) + 2]; // V
      r = extract_r_from_yuv(y, u, v);
      g = extract_g_from_yuv(y, u, v);
      b = extract_b_from_yuv(y, u, v);
      p_xbgr8888[0] = 0;
      p_xbgr8888[1] = b;
      p_xbgr8888[2] = g;
      p_xbgr8888[3] = r;
      p_xbgr8888 += 4;
    }
    p_yuv422 += width*2;
    p_xbgr8888_base += xbgr8888->stride[0];
  }


  return xbgr8888;
}

/*
ErvpImage* conv_xbgr8888_to_rgb565(const ErvpImage* const xbgr8888, ErvpImage* rgb565)
{
  int i, j;
  uint8_t r, g, b;
  int width = xbgr8888->width;
  int height = xbgr8888->height;
  uint8_t *p_xbgr8888;
  uint8_t *p_xbgr8888_base;
  uint16_t *p_rgb565;

  assert(xbgr8888!=NULL);
  assert(xbgr8888->format==IMAGE_FMT_XBGR_8888_PACKED);

  if(rgb565==NULL)
    rgb565 = image_alloc(width, height, IMAGE_FMT_RGB_565_PACKED);
  assert(rgb565->format==IMAGE_FMT_RGB_565_PACKED);

  p_xbgr8888_base = (uint8_t *)xbgr8888->addr[0];
  p_rgb565 = (uint16_t *)rgb565->addr[0];
  for(j=0; j<height; j++)
  {
    p_xbgr8888 = p_xbgr8888_base;
    for(i=0; i<width; i++)
    {
      b = p_xbgr8888[1];
      g = p_xbgr8888[2];
      r = p_xbgr8888[3];
      p_xbgr8888 += 4;
      *p_rgb565 = gen_rgb565(r, g, b);
      p_rgb565++;
    }
    p_xbgr8888_base += xbgr8888->stride[0];
  }

  return rgb565;
}
*/

ErvpImage* conv_xbgr8888_to_rgb565(const ErvpImage* const xbgr8888, ErvpImage* rgb565)
{
  int i, j;
  int width = xbgr8888->width;
  int height = xbgr8888->height;

  assert(xbgr8888!=NULL);
  assert(xbgr8888->format==IMAGE_FMT_XBGR_8888_PACKED);

  if(rgb565==NULL)
    rgb565 = image_alloc(width, height, IMAGE_FMT_RGB_565_PACKED);
  assert(rgb565->format==IMAGE_FMT_RGB_565_PACKED);

  for(int i=0; i<height; i++)
  {
    uint32_t* xbgr8888_row_base = image_get_row_base(xbgr8888, 0, i);
    uint16_t* rgb565_row_base = image_get_row_base(rgb565, 0, i);
    for(int j=0; j<width; j++)
      rgb565_row_base[j] = gen_rgb565_from_xbgr8888(xbgr8888_row_base[j]);
  }
  return rgb565;
}

ErvpImage* conv_rgb565_packed_to_xbgr8888(const ErvpImage* const rgb565, ErvpImage* xbgr8888)
{
  int i, j;
  int width = rgb565->width;
  int height = rgb565->height;

  assert(rgb565!=NULL);
  assert(rgb565->format==IMAGE_FMT_RGB_565_PACKED);

  if(xbgr8888==NULL)
    xbgr8888 = image_alloc(width, height, IMAGE_FMT_XBGR_8888_PACKED);
  assert(xbgr8888->format==IMAGE_FMT_XBGR_8888_PACKED);

  for(int i=0; i<height; i++)
  {
    uint16_t* rgb565_row_base = image_get_row_base(rgb565, 0, i);
    uint32_t* xbgr8888_row_base = image_get_row_base(xbgr8888, 0, i);
    for(int j=0; j<width; j++)
      xbgr8888_row_base[j] = gen_xbgr8888_from_rgb565(rgb565_row_base[j]);
  }
  return xbgr8888;
}

ErvpImage* conv_y8_to_xbgr8888(const ErvpImage* const y8, ErvpImage* xbgr8888)
{
  int i, j;
  int width = y8->width;
  int height = y8->height;

  assert(y8!=NULL);
  assert(y8->format==IMAGE_FMT_Y_ONLY);

  if(xbgr8888==NULL)
    xbgr8888 = image_alloc(width, height, IMAGE_FMT_XBGR_8888_PACKED);
  assert(xbgr8888->format==IMAGE_FMT_XBGR_8888_PACKED);

  for(int i=0; i<height; i++)
  {
    uint8_t* y8_row_base = image_get_row_base(y8, 0, i);
    uint32_t* xbgr8888_row_base = image_get_row_base(xbgr8888, 0, i);
    for(int j=0; j<width; j++)
      xbgr8888_row_base[j] = gen_xbgr8888_from_y8(y8_row_base[j]);
  }
  return xbgr8888;
}

ErvpImage* conv_rgb888_packed_to_xbgr8888(const ErvpImage* const rgb888_packed, ErvpImage* xbgr8888)
{
  int i, j;
  int width = rgb888_packed->width;
  int height = rgb888_packed->height;

  assert(rgb888_packed!=NULL);
  assert(rgb888_packed->format==IMAGE_FMT_RGB_888_PACKED);

  if(xbgr8888==NULL)
    xbgr8888 = image_alloc(width, height, IMAGE_FMT_XBGR_8888_PACKED);
  assert(xbgr8888->format==IMAGE_FMT_XBGR_8888_PACKED);

  for(int i=0; i<height; i++)
  {
    uint8_t* p_rgb888_packed = image_get_row_base(rgb888_packed, 0, i);
    uint8_t* p_xbgr8888 = image_get_row_base(xbgr8888, 0, i);
    for(int j=0; j<width; j++)
    {
      p_xbgr8888[1] = p_rgb888_packed[2];
      p_xbgr8888[2] = p_rgb888_packed[1];
      p_xbgr8888[3] = p_rgb888_packed[0];

      p_rgb888_packed += 3;
      p_xbgr8888 += 4;
    }
  }

  return xbgr8888;
}

ErvpImage* conv_rgb888_packed_to_rgb888_plane(const ErvpImage* const rgb888_packed, ErvpImage* rgb888_plane)
{
  int i, j;
  int width = rgb888_packed->width;
  int height = rgb888_packed->height;

  assert(rgb888_packed!=NULL);
  assert(rgb888_packed->format==IMAGE_FMT_RGB_888_PACKED);

  if(rgb888_plane==NULL)
    rgb888_plane = image_alloc(width, height, IMAGE_FMT_RGB_888_PLANE);
  assert(rgb888_plane->format==IMAGE_FMT_RGB_888_PLANE);

  for(int i=0; i<height; i++)
  {
    uint8_t* p_rgb888_packed = image_get_row_base(rgb888_packed, 0, i);
    uint8_t* p_rgb888_r = image_get_row_base(rgb888_plane, 0, i);
    uint8_t* p_rgb888_g = image_get_row_base(rgb888_plane, 1, i);
    uint8_t* p_rgb888_b = image_get_row_base(rgb888_plane, 2, i);

    for(int j=0; j<width; j++)
    {
      uint8_t r, g, b;
      r = p_rgb888_packed[0];
      g = p_rgb888_packed[1];
      b = p_rgb888_packed[2];      

      p_rgb888_packed += 3;
      p_rgb888_r[j] = r;
      p_rgb888_g[j] = g;
      p_rgb888_b[j] = b;
    }
  }

  return rgb888_plane;
}

ErvpImage* conv_xbgr8888_to_rgb888_packed(const ErvpImage* const xbgr8888, ErvpImage* rgb888_packed)
{
  int width = xbgr8888->width;
  int height = xbgr8888->height;
  int i, j;

  uint8_t *p_rgb888_packed;
  uint8_t *p_xbgr8888;

  assert(xbgr8888!=NULL);
  assert(xbgr8888->format==IMAGE_FMT_XBGR_8888_PACKED);
  assert(!xbgr8888->is_window);

  if(rgb888_packed==NULL)
    rgb888_packed = image_alloc(width, height, IMAGE_FMT_RGB_888_PACKED);
  assert(rgb888_packed->format==IMAGE_FMT_RGB_888_PACKED);

  p_rgb888_packed = (uint8_t *)rgb888_packed->addr[0];
  p_xbgr8888 = (uint8_t *)xbgr8888->addr[0];

  for(j = 0; j < height; ++j){
    for(i = 0; i < width; ++i){
      p_rgb888_packed[0] = p_xbgr8888[3];
      p_rgb888_packed[1] = p_xbgr8888[2];
      p_rgb888_packed[2] = p_xbgr8888[1];

      p_rgb888_packed += 3;
      p_xbgr8888 += 4;
    }
  }
  return rgb888_packed;
}

ErvpImage* conv_xbgr8888_to_rgb888_plane(const ErvpImage* const xbgr8888, ErvpImage* rgb888_plane)
{
  int i, j;
  int width = xbgr8888->width;
  int height = xbgr8888->height;

  assert(xbgr8888!=NULL);
  assert(xbgr8888->format==IMAGE_FMT_XBGR_8888_PACKED);

  if(rgb888_plane==NULL)
    rgb888_plane = image_alloc(width, height, IMAGE_FMT_RGB_888_PLANE);
  assert(rgb888_plane->format==IMAGE_FMT_RGB_888_PLANE);

  for(int i=0; i<height; i++)
  {
    uint32_t* p_xbgr8888 = image_get_row_base(xbgr8888, 0, i);
    uint8_t* p_rgb888_r = image_get_row_base(rgb888_plane, 0, i);
    uint8_t* p_rgb888_g = image_get_row_base(rgb888_plane, 1, i);
    uint8_t* p_rgb888_b = image_get_row_base(rgb888_plane, 2, i);
    
    for(int j=0; j<width; j++)
    {
      uint32_t xbgr8888 = p_xbgr8888[j];
      uint8_t r, g, b;
      r = extract_r_from_xbgr8888(xbgr8888);
      g = extract_g_from_xbgr8888(xbgr8888);
      b = extract_b_from_xbgr8888(xbgr8888);
      p_rgb888_r[j] = r;
      p_rgb888_g[j] = g;
      p_rgb888_b[j] = b;
    }
  }
  return rgb888_plane;
}

ErvpImage* conv_rgb888_plane_to_xbgr8888(const ErvpImage* const rgb888_plane, ErvpImage* xbgr8888)
{
  int i, j;
  int width = rgb888_plane->width;
  int height = rgb888_plane->height;

  assert(rgb888_plane!=NULL);
  assert(rgb888_plane->format==IMAGE_FMT_RGB_888_PLANE);

  if(xbgr8888==NULL)
    xbgr8888 = image_alloc(width, height, IMAGE_FMT_XBGR_8888_PACKED);
  assert(xbgr8888->format==IMAGE_FMT_XBGR_8888_PACKED);

  for(int i=0; i<height; i++)
  {
    uint8_t* p_rgb888_r = image_get_row_base(rgb888_plane, 0, i);
    uint8_t* p_rgb888_g = image_get_row_base(rgb888_plane, 1, i);
    uint8_t* p_rgb888_b = image_get_row_base(rgb888_plane, 2, i);
    uint32_t* p_xbgr8888 = image_get_row_base(xbgr8888, 0, i);
    for(int j=0; j<width; j++)
      p_xbgr8888[j] = gen_xbgr8888(0,p_rgb888_r[j],p_rgb888_g[j],p_rgb888_b[j]);
  }

  return xbgr8888;
}

// deprecated
void image_prepare_dump(const ErvpImage *image)
{
  assert(image->file_size>0);
  switch(image->format)
  {
    case IMAGE_FMT_JPEG:
    case IMAGE_FMT_BMP:
      break;
    default:
      assert_not_implemented();
  }
  memory_dump_prepare(MEMORY_DUMP_IMAGE, (void*)(image->addr[0]), round_up_int(image->file_size,4));
}

void image_print_info(const ErvpImage* const image)
{
  assert(image);
  printf("\n\naddr: 0x%x", image);
  printf("\nformat: %s", image_format_get_name(image->format));
  printf("\nwidth: %d", image->width);
  printf("\nheight: %d", image->height);
  for(int i=0; i<image_get_num_plane(image->format); i++)
  {
    printf("\nplane %d addr: 0x%x", i, (unsigned int)(image->addr[i]));
    printf("\nplane %d stride: 0x%x", i, (unsigned int)(image->stride[i]));
  }
  uint32_t* plane0 = (uint32_t*)image->addr[0];
  for(int i=0; i<4; i++)
    printf("\nplane0 value: 0x%x", (unsigned int)(plane0[i]));
}

void image_copy(const ErvpImage* const src, ErvpImage* dst, int row_index, int col_index)
{
  assert(src!=NULL);
  assert(dst!=NULL);
  assert(src->format==dst->format);
  assert((src->width+col_index)<=dst->width);
  assert((src->height+row_index)<=dst->height);

  for(int i=0; i<image_get_num_plane(src->format); i++)
    for(int j=0; j<image_get_plane_height(src, i); j++)
    {
      void* src_addr = image_get_row_base(src,i,j);
      void* dst_addr = image_get_row_base(dst,i,row_index+j);
      dst_addr += (image_get_macro_pixel_size(dst,i)*col_index);
      int size = image_get_plane_width_size(src,i);
      memcpy(dst_addr,src_addr,size);
    }
}

int image_is_same_size(const ErvpImage* const left, const ErvpImage* const right)
{
  int result = 0;
  if(left==NULL)
    ;
  else if(right==NULL)
    ;
  else if(left->width!=right->width)
    ;
  else if(left->height!=right->height)
    ;
  else
    result = 1;
  return result;
}

int image_is_same_type(const ErvpImage* const left, const ErvpImage* const right)
{
  int result = 0;
  if(!image_is_same_size(left,right))
    ;
  else if(left->format!=right->format)
    ;
  else
    result = 1;
  return result;
}

void* image_get_macro_pixel_addr(const ErvpImage* const image, int plane_index, int row_index, int col_index)
{
  void* row_base = image_get_row_base(image, plane_index, row_index);
  int col_index_cliped = CLIP(col_index, 0, image->width-1);
  int addr_offset;
  void* result;
  addr_offset = col_index_cliped * image_get_macro_pixel_size(image, plane_index);
  result = row_base + addr_offset;
  assert_invalid_data_pointer(result);
  return result;
}

void* image_get_pixel_addr(const ErvpImage* const image, int plane_index, int row_index, int col_index)
{
  return image_get_macro_pixel_addr(image, plane_index, row_index, col_index);
}

ErvpImage* image_convert_format(const ErvpImage* const original, ErvpImage* preallocated, unsigned int format)
{
  ErvpImage* result;
  assert(original!=NULL);

  if(format == original->format)
  {
    if (preallocated==NULL)
      result = image_alloc(original->width,original->height,format);
    else
      result = preallocated;
    image_copy(original,result,0,0);
  }
  else if(format == IMAGE_FMT_BMP)
  {
    result = image_convert_bmp(original, preallocated);
  }
  else if((original->format == IMAGE_FMT_YUV_422_PACKED) && (format == IMAGE_FMT_RGB_565_PACKED))
  {
    result = conv_yuv422_to_rgb565(original, preallocated);
  }
  else if((original->format == IMAGE_FMT_XBGR_8888_PACKED) && (format == IMAGE_FMT_RGB_565_PACKED))
  {
    result = conv_xbgr8888_to_rgb565(original, preallocated);
  }
  else if((original->format == IMAGE_FMT_RGB_565_PACKED) && (format == IMAGE_FMT_XBGR_8888_PACKED))
  {
    result = conv_rgb565_packed_to_xbgr8888(original, preallocated);
  }
  else if((original->format == IMAGE_FMT_XBGR_8888_PACKED) && (format == IMAGE_FMT_RGB_888_PACKED))
  {
    result = conv_xbgr8888_to_rgb888_packed(original, preallocated);
  }
  else if((original->format == IMAGE_FMT_XBGR_8888_PACKED) && (format == IMAGE_FMT_RGB_888_PLANE))
  {
    result = conv_xbgr8888_to_rgb888_plane(original, preallocated);
  }
  else if((original->format == IMAGE_FMT_RGB_888_PACKED) && (format == IMAGE_FMT_XBGR_8888_PACKED))
  {
    result = conv_rgb888_packed_to_xbgr8888(original, preallocated);
  }
  else if((original->format == IMAGE_FMT_RGB_888_PACKED) && (format == IMAGE_FMT_RGB_888_PLANE))
  {
    result = conv_rgb888_packed_to_rgb888_plane(original, preallocated);
  }
  else if((original->format == IMAGE_FMT_RGB_888_PLANE) && (format == IMAGE_FMT_XBGR_8888_PACKED))
  {
    result = conv_rgb888_plane_to_xbgr8888(original, preallocated);
  }
  else if((original->format == IMAGE_FMT_Y_ONLY) && (format == IMAGE_FMT_XBGR_8888_PACKED))
  {
    result = conv_y8_to_xbgr8888(original, preallocated);
  }
  else
  {
    result = NULL;
    printf("\n%s %s\n", image_format_get_name(original->format), image_format_get_name(format));
    assert_not_implemented();
  }
  assert(result);
  return result;
}

void image_clear(ErvpImage* image, unsigned int option_value)
{
  assert(image!=NULL);
  //printf("\n%d %d %d\n", image_get_num_plane(image->format), image_get_plane_height(image, 0), image->stride[0]);
  for(int i=0; i<image_get_num_plane(image->format); i++)
  {
    uint32_t pixel_value = 0x00000000;
    switch(image->format)
    {
      case IMAGE_FMT_YUV_420_PLANAR_YV12:
      case IMAGE_FMT_YUV_420_PLANAR_NV12:
        if(i>0)
          pixel_value = 0x80808080;
        break;
    }
    for(int j=0; j<image_get_plane_height(image, i); j++)
    {
      uint32_t* row_base = image_get_row_base(image, i, j);
      for(int k=0; k<(image->stride[i])>>2; k++)
        row_base[k] = pixel_value;
    }
  }
}

ErvpImage* image_generate_window(const ErvpImage* const image, int width, int height, int row_index, int col_index, ErvpImage* preallocated)
{
  ErvpImage* result;
  if(preallocated==NULL)
    result = image_alloc_wo_internals(width, height, image->format);
  else
  {
    result = preallocated;
    image_init_structure(result, width, height, image->format);
  }
  result->is_window = 1;
  result->window_info = malloc(sizeof(ervp_image_window_info_t));
  assert(result->window_info);
  result->window_info->original = image;
  result->window_info->row_index = row_index;
  result->window_info->col_index = col_index;
  for(int i=0; i<3; i++)
  {
    result->addr[i] = image->addr[i];
    result->stride[i] = image->stride[i];
  }
  for(int i=0; i<image_get_num_plane(image->format); i++)
    result->addr[i] = image_get_macro_pixel_addr(image, i, row_index, col_index);
  assert(result->file_size==0);
  return result;
}

ErvpImage* image_generate_center_window(const ErvpImage* const image, int width, int height, ErvpImage* preallocated)
{
  int row_dinex = (image->height - height) >>1;
  int col_index = (image->width - width) >>1;
  return image_generate_window(image, width, height, row_dinex, col_index, preallocated);
}

ErvpMatrixInfo* image_generate_matrix_info(const ErvpImage* const image, int plane_index, int row_index, int col_index)
{
  ErvpMatrixInfo* minfo;
  assert(image);
  assert(plane_index < image_get_num_plane(image->format));
  minfo = matrix_generate_info(MATRIX_DATATYPE_UINT08,image_get_plane_height(image,plane_index), image_get_plane_width(image,plane_index),NULL,NULL);
  minfo->addr = image_get_macro_pixel_addr(image,plane_index,row_index,col_index);
  matrix_set_stride(minfo, image->stride[plane_index]);
  return minfo;
}

