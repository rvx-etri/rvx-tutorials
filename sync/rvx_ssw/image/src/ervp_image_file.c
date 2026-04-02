#include "platform_info.h"
#include "ervp_assert.h"
#include "ervp_printf.h"
#include "ervp_image.h"
#include "ervp_fakefile.h"

#include "ervp_image_file.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#endif

typedef enum {
  IMAGE_FILE_FMT_PNG = 0,
  IMAGE_FILE_FMT_BMP = 1,
  IMAGE_FILE_FMT_TGA = 2,
  IMAGE_FILE_FMT_HDR = 3,
  IMAGE_FILE_FMT_JPG = 4,
} ImageFileFormat;

ErvpImage* image_load_from_memory(const void *data, const int size, ErvpImage* rgb888)
{
  int w, h, c;
  int i, j, k;

  uint8_t *p_rgb888;

  assert(data!=NULL);

  p_rgb888 = stbi_load_from_memory(data, size, &w, &h, &c, 3);
  if (!p_rgb888)
  {
    printf("Cannot load image from memory\n");
    assert(0);
  }
  else
  {
    printf("w: %d / h: %d / c: %d\n", w, h, c);
  }

  if(rgb888==NULL)
    rgb888 = image_alloc_wo_internals(w, h, IMAGE_FMT_RGB_888_PACKED);
  assert(rgb888->format==IMAGE_FMT_RGB_888_PACKED);

  image_free(rgb888);
  image_decide_stride(rgb888, 0);

  rgb888->addr[0] = (void *)p_rgb888;

  return rgb888;
}

ErvpImage* image_file_read(const char *filename, ErvpImage* rgb888)
{
  int w, h, c;
  uint8_t *p_rgb888;
  ErvpImage* result = NULL;

  p_rgb888 = stbi_load(filename, &w, &h, &c, 3);
  assert_msg(p_rgb888!=0, "Failed to read image file %s\n", filename);

  if(rgb888==NULL)
  {
    result = image_alloc_wo_internals(w, h, IMAGE_FMT_RGB_888_PACKED);
    image_decide_stride(result, 0);
  }
  else
    assert_not_implemented();
  result->addr[0] = (void *)p_rgb888;
  return result;
}

static int _rgb888_write_file(const ErvpImage* rgb888, const char *filename, unsigned int option_value)
{
  int w = rgb888->width;
  int h = rgb888->height;
  int c = 3;
  ervp_image_write_option_t option;
  option.value = option_value;

  int len = strlen(filename);
  const char *filename_suffix = &filename[len-4];
  
  unsigned char *data = (unsigned char *)rgb888->addr[0];
  int success = 0;
  if(strcmp(filename_suffix,".bmp")==0)
    success = stbi_write_bmp(filename, w, h, c, data);
  else if(strcmp(filename_suffix,".jpg")==0)
    success = stbi_write_jpg(filename, w, h, c, data, option.jpeg_quality);
  else if(strcmp(filename_suffix,".png")==0)
    success = stbi_write_png(filename, w, h, c, data, w*c);
  else if(strcmp(filename_suffix,".tga")==0)
    success = stbi_write_tga(filename, w, h, c, data);
  else if(strcmp(filename_suffix,".hdr")==0)
    success = stbi_write_hdr(filename, w, h, c, data);
  return success;
}

static int _jpeg_or_bmp_write_file(const ErvpImage* image, const char *filename)
{
  int success;
  int written_size;
  FAKEFILE* f = ffopen(filename, "w");
  assert(image->addr[0]);
  written_size = ffwrite(image->addr[0], 1, image->file_size, f);
  ffclose(f);
  success = (written_size==image->file_size);
  return success;
}

int image_file_write(const ErvpImage* image, const char *filename, unsigned int option_value)
{
  assert(image!=NULL);
  int success = 0;
  if(image->format==IMAGE_FMT_RGB_888_PACKED)
    success = _rgb888_write_file(image, filename, option_value);
  else if((image->format==IMAGE_FMT_JPEG) || (image->format==IMAGE_FMT_BMP))
    success = _jpeg_or_bmp_write_file(image, filename);
  assert_msg(success!=0, "\nFailed to write image file \"%s\"\n", filename);
  return success;
}