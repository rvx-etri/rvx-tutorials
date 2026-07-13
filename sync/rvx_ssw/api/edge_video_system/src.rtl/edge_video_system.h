#ifndef __EDGE_VIDEO_SYSTEM_H__
#define __EDGE_VIDEO_SYSTEM_H__

#include "ervp_image.h"
#include "ervp_video_frame.h"

#include "vio_define.h"
#include "vi.h"
#include "vo.h"

typedef enum {
  CAMERA_HW_NONE = 0,
  CAMERA_HW_CIS = 1, // VIM_EN_CIS
  CAMERA_HW_HDMI = 2 // VIM_EN_HDMI
} eCAMERA_HW;

typedef enum {
  DISPLAY_HW_NONE = 0,
  DISPLAY_HW_LCD = 1, // VOM_EN_LCD
  DISPLAY_HW_HDMI = 2 // VOM_EN_HDMI
} eDISPLAY_HW;

typedef struct {
  int camera_hw;
  int display_hw;
  int use_default_frame;
  ErvpImage *vframe_out, *vframe_in0, *vframe_in1;
  // default frame config
  int image_format;
  int camera_resolution;
  int display_resolution;
} sVIDEO_SYSTEM_CONFIG;

extern sVIDEO_SYSTEM_CONFIG edge_video_system_config;

void edge_video_system_init();
void edge_video_system_register_frame(ErvpImage* out, ErvpImage* in0, ErvpImage* in1);
void edge_video_system_register_default_frame();
int edge_video_system_config_gui(); // return 0 means the exit
void edge_video_system_display_direct(int with_key_exit);

void edge_video_system_start();
void edge_video_system_start_display();
void edge_video_system_start_camera();
void edge_video_system_display_direct(int with_key_exit); // type x for exit
void vframe_config_vom(const ErvpImage* vframe_out);
void vframe_config_vim(const ErvpImage* vframe_in0, const ErvpImage* vframe_in1);

extern int vframe_in_index_for_current_display;

static inline ErvpImage* edge_video_system_get_displayed_vframe_in()
{
  return vframe_in_index_for_current_display? edge_video_system_config.vframe_in1 : edge_video_system_config.vframe_in0;
}

static inline ErvpImage* edge_video_system_get_undisplayed_vframe_in()
{
  return vframe_in_index_for_current_display? edge_video_system_config.vframe_in0 : edge_video_system_config.vframe_in1;
}

static inline ErvpImage* edge_video_system_get_vframe_output()
{
  return edge_video_system_config.vframe_out;
}

ErvpImage* edge_video_system_capture();
ErvpImage* edge_video_system_capture_ms(int ms);

#endif
