#include <stdint.h>
#include "ervp_image_util.h"
#include "ervp_printf.h"
#include "ervp_malloc.h"
#include "ervp_memory_util.h"
#include "ervp_assert.h"
#include "ervp_round_int.h"
#include "ervp_matrix_op_sw.h"

#pragma pack(push)  // save the original data alignment
#pragma pack(1)     // Set data alignment to 1 byte boundary

typedef struct {
    uint16_t type;              // Magic identifier: 0x4d42
    uint32_t size;              // File size in bytes
    uint16_t reserved1;         // Not used
    uint16_t reserved2;         // Not used
    uint32_t offset;            // Offset to original data in bytes from beginning of file
    uint32_t dib_header_size;   // DIB Header size in bytes
    int32_t  width_px;          // Width of the original
    int32_t  height_px;         // Height of original
    uint16_t num_planes;        // Number of color planes
    uint16_t bits_per_pixel;    // Bits per pixel
    uint32_t compression;       // Compression type
    uint32_t image_size_bytes;  // Image size in bytes
    int32_t  x_resolution_ppm;  // Pixels per meter
    int32_t  y_resolution_ppm;  // Pixels per meter
    uint32_t num_colors;        // Number of colors
    uint32_t important_colors;  // Important colors
    uint32_t red_channel_bitmask;  //Red channel bitmask
    uint32_t green_channel_bitmask;  //Green channel bitmask
    uint32_t blue_channel_bitmask;  //blue channel bitmask
} BMPHeader;

#pragma pack(pop)  // restore the previous pack setting

const int BMP_HEADER_SIZE = sizeof(BMPHeader);

ErvpImage* image_convert_grayscale(const ErvpImage* const original, ErvpImage* preallocated)
{
  ErvpImage* result;
  assert(original);
  if(preallocated==NULL)
    result = image_alloc(original->width, original->height, IMAGE_FMT_Y_ONLY);
  else
  {
    assert(image_is_same_size(original, preallocated));
    assert(preallocated->format==IMAGE_FMT_Y_ONLY);
    result = preallocated;
  }
  switch(original->format)
  {
    case IMAGE_FMT_RGB_565_PACKED:
      {
        for(int i=0; i<original->height; i++)
        {
          uint16_t* src_row_base = (uint16_t*)image_get_row_base_raw(original, 0, i);
          uint8_t* dst_row_base = (uint8_t*)image_get_row_base_raw(result, 0, i);
          for(int j=0; j<original->width; j++)
            dst_row_base[j] = rgb565_to_luma(src_row_base[j]);
        }
      }
      break;
    case IMAGE_FMT_Y_ONLY:
      {
        for(int i=0; i<original->height; i++)
        {
          uint8_t* src_row_base = (uint8_t*)image_get_row_base_raw(original, 0, i);
          uint8_t* dst_row_base = (uint8_t*)image_get_row_base_raw(result, 0, i);
          for(int j=0; j<original->width; j++)
            dst_row_base[j] = src_row_base[j];
        }
      }
      break;
    case IMAGE_FMT_XBGR_8888_PACKED:
      {
        for(int i=0; i<original->height; i++)
        {
          uint32_t* src_row_base = (uint32_t*)image_get_row_base_raw(original, 0, i);
          uint8_t* dst_row_base = (uint8_t*)image_get_row_base_raw(result, 0, i);
          for(int j=0; j<original->width; j++)
            dst_row_base[j] = xbgr8888_to_luma(src_row_base[j]);
        }
      }
      break;
    default:
      assert(0);
  }
  return result;
}

static void make_bmp_header(const ErvpImage *original, ErvpImage *bmp)
{
  int width = original->width;
  int height = original->height;
  BMPHeader *bmp_header = (BMPHeader *)bmp->addr[0];
  uint32_t header_size = sizeof(BMPHeader);

  memset(bmp_header, 0, sizeof(BMPHeader));
  if(original->format == IMAGE_FMT_XBGR_8888_PACKED)
  {
    bmp_header->bits_per_pixel = 32;    // Bits per pixel
    bmp_header->red_channel_bitmask = 0xff000000;  //Red channel bitmask
    bmp_header->green_channel_bitmask = 0x00ff0000;  //Green channel bitmask
    bmp_header->blue_channel_bitmask = 0x0000ff00;  //blue channel bitmask
  }
  else if(original->format == IMAGE_FMT_RGB_565_PACKED)
  {
    bmp_header->bits_per_pixel = 16;    // Bits per pixel
    bmp_header->red_channel_bitmask = 0xF800;  //Red channel bitmask
    bmp_header->green_channel_bitmask = 0x07E0;  //Green channel bitmask
    bmp_header->blue_channel_bitmask = 0x001F;  //blue channel bitmask
  }
  bmp_header->type = 0x4d42;              // Magic identifier: 0x4d42
  bmp_header->size = bmp->file_size;              // File size in bytes
  bmp_header->offset = header_size;            // Offset to original data in bytes from beginning of file
  bmp_header->dib_header_size = 0x28;   // DIB Header size in bytes
  bmp_header->width_px = width;          // Width of the original
  bmp_header->height_px = height;         // Height of original
  bmp_header->num_planes = 1;        // Number of color planes
  bmp_header->compression = 3;       // Compression type
}

static void make_bmp_data(const ErvpImage *original, ErvpImage *bmp)
{
  int i;
  int width = original->width;
  int height = original->height;
  uint8_t *p_image_data;	
  uint8_t *p_bmp_data;	
  BMPHeader *bmp_header = (BMPHeader *)bmp->addr[0];
  uint32_t line_size = 0;

  if(original->format == IMAGE_FMT_XBGR_8888_PACKED)
  {
    line_size = width*4;
  }
  else if(original->format == IMAGE_FMT_RGB_565_PACKED)
  {
    line_size = width*2;
  }

  p_bmp_data = (uint8_t *)bmp_header + bmp_header->offset;
  p_image_data = original->addr[0] + original->stride[0]*(height-1);
  //line size memcopy
  for(i = 0; i<height; i++)
  {
    memcpy(p_bmp_data, p_image_data, line_size);
    p_bmp_data += line_size;
    p_image_data -= original->stride[0];
  }
}

ErvpImage *image_convert_bmp(const ErvpImage* original, ErvpImage *preallocated)
{
  ErvpImage* result;
  assert((original->format==IMAGE_FMT_RGB_565_PACKED)||(original->format==IMAGE_FMT_XBGR_8888_PACKED));
  if(preallocated == NULL)
  {
    int file_size;
    result = image_alloc_wo_internals(original->width, original->height, IMAGE_FMT_BMP);
    if(original->format == IMAGE_FMT_XBGR_8888_PACKED)
      file_size = BMP_HEADER_SIZE + original->width*4*original->height + 2;
    else if(original->format == IMAGE_FMT_RGB_565_PACKED)
      file_size = BMP_HEADER_SIZE + original->width*2*original->height + 2;
    else
      file_size = 0;
    file_size = round_up_int(file_size, 4);
    result->addr[0] = (uint8_t*)malloc(file_size);
    assert(result->addr[0]);
    result->file_size = file_size;
  }
  else
    result = preallocated;
  make_bmp_header(original, result);
  make_bmp_data(original, result);
  return result;
}

ErvpImage* image_resize(const ErvpImage* const original, int width, int height, ErvpImage* preallocated)
{
  int width_hop, height_hop;
  ErvpImage* result;
  assert(original);
  if(preallocated==NULL)
    result = image_alloc(width, height, original->format);
  else
  {
    result = preallocated;
    assert(preallocated->width==width);
    assert(preallocated->height==height);
    assert(preallocated->format==original->format);
  }
  width_hop = original->width/width;
  height_hop = original->height/height;
  for(int i=0; i<image_get_num_plane(original->format); i++)
  {
    for(int j=0; j<height; j++)
      for(int k=0; k<width; k++)
      {
        uint8_t* src_addr = image_get_macro_pixel_addr(original, i, j*height_hop, k*width_hop);
        uint8_t* dst_addr = image_get_macro_pixel_addr(result, i, j, k);
        for(int l=0; l<image_get_macro_pixel_size(original,i); l++)
          dst_addr[l] = src_addr[l];
      }
  }
  return result;
}

ErvpImage* image_filter(ervp_mop_mapping_t* mop_mapping, const ErvpImage* const original, const ErvpMatrixInfo* const filter, ErvpImage* preallocated)
{
  ErvpImage* result;
  assert(original);
  assert(!image_is_packed(original->format));
  ervp_mconv_option_t conv_option;
  conv_option.value = 0;
  conv_option.br.performs_cliping = 1;
  int width = matrix_conv_output_cols(original->width, filter->num_col, conv_option.value);
  int height = matrix_conv_output_rows(original->height, filter->num_row, conv_option.value);
  if(preallocated==NULL)
    result = image_alloc(width, height, original->format);
  else
  {
    assert(original->width==width);
    assert(original->height==height);
    result = preallocated;
  }

  if(matrix_datatype_is_float(filter->datatype))
  {
    ErvpMatrixInfo* filter_converted;    
    const int shift_amount = 16;
    
    filter_converted = matrix_alloc(MATRIX_DATATYPE_SINT32,filter->num_row,filter->num_col,NULL);
    for(int i=0; i<filter->num_row; i++)
      for(int j=0; j<filter->num_col; j++)
        matrix_write_fixed_element(filter_converted,i,j,matrix_read_float_element(filter,i,j)*(1<<shift_amount));
    
    conv_option.br.rshift = shift_amount;
    for(int i=0; i<image_get_num_plane(original->format); i++)
    {
      ErvpMatrixInfo* a = image_generate_matrix_info(original, i, 0, 0);
      ErvpMatrixInfo* c = image_generate_matrix_info(result, i, 0, 0);
      mop_mapping->matrix_conv(mop_mapping, a,filter_converted,c,conv_option.value);
    }    
    matrix_free(filter_converted);
  }
  else
  {
    for(int i=0; i<image_get_num_plane(original->format); i++)
    {
      ErvpMatrixInfo* a = image_generate_matrix_info(original, i, 0, 0);
      ErvpMatrixInfo* c = image_generate_matrix_info(result, i, 0, 0);
      mop_mapping->matrix_conv(mop_mapping, a,filter,c, conv_option.value);
    }
  }
  
  return result;
}