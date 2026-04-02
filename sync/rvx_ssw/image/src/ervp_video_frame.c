#include "ervp_video_frame.h"
#include "ervp_malloc.h"
#include "ervp_assert.h"

int get_resolution_width(int resolution)
{
  int result = 0;
  switch(resolution)
  {
    case IMAGE_RESOLUTION_VGA:
      result = 640;
      break;
    case IMAGE_RESOLUTION_1024x600:
      result = 1024;
      break;
    case IMAGE_RESOLUTION_FHD:
      result = 1920;
      break;
    case IMAGE_RESOLUTION_UHD4K:
      result = 3840;
      break;
  }
  assert(result!=0);
  return result;
}

int get_resolution_height(int resolution)
{
  int result = 0;
  switch(resolution)
  {
    case IMAGE_RESOLUTION_VGA:
      result = 480;
      break;
    case IMAGE_RESOLUTION_1024x600:
      result = 600;
      break;
    case IMAGE_RESOLUTION_FHD:
      result = 1080;
      break;
    case IMAGE_RESOLUTION_UHD4K:
      result = 2160;
      break;
  }
  assert(result!=0);
  return result;
}

ErvpImage* vframe_alloc(int format, int resolution)
{
  ErvpImage* vframe;
  vframe = image_alloc_option(get_resolution_width(resolution), get_resolution_height(resolution), format, 1, 1);
  image_clear(vframe, 0);
  return vframe;
}
