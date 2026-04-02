#ifndef __ERVP_IMAGE_FILE_H__
#define __ERVP_IMAGE_FILE_H__

#include "ervp_image.h"

ErvpImage* image_load_from_memory(const void *data, const int size, ErvpImage* rgb888);
ErvpImage* image_file_read(const char *filename, ErvpImage* rgb888);

typedef union
{
  unsigned int value;
  unsigned int jpeg_quality;
} ervp_image_write_option_t;

int image_file_write(const ErvpImage* image, const char *filename, unsigned int option_value);

#endif // __ERVP_IMAGE_FILE_H__
