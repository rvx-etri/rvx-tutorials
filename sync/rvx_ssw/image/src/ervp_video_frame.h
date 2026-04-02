#ifndef __VIDEO_FRAME_H__
#define __VIDEO_FRAME_H__

#include "ervp_image.h"

typedef enum {
  IMAGE_RESOLUTION_VGA = 0, // 640x480
  IMAGE_RESOLUTION_1024x600 = 1, // LCD
  IMAGE_RESOLUTION_FHD = 2, // 1920x1080
  IMAGE_RESOLUTION_UHD4K = 3 // 3840x2160
} eIMAGE_RESOLUTION;

// 3840x2160x4byte
#define VIDEO_FRAME_MAX_SIZE 0x04000000
#define VIDEO_SECTION_MAX_SIZE (VIDEO_FRAME_MAX_SIZE>>2)

int get_resolution_width(int resolution);
int get_resolution_height(int resolution);

ErvpImage* vframe_alloc(int format, int resolution);

#endif
