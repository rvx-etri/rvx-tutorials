#include "edge_video_system.h"

#include "platform_info.h"
#include "ervp_malloc.h"
#include "ervp_delay.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_mcom_input.h"
#include "ervp_assert.h"
#include "ervp_misc_util.h"
#include "ervp_real_clock.h"

#include "cis_mt9p111.h"
//#include "cis_ov2640.h"
//#include "cis_ov5642.h"
#include "cis_ov5640.h"
#include "hdmi_adv7619.h"
#include "hdmi_sil1136.h"

const int SDR_OFF_CONFIG_REG0 = 0;
const int SDR_OFF_CONFIG_REG1 = 4;
const int SDR_OFF_REF_REG = 8;

typedef struct {
	int hsize;
	int vsize;
	unsigned int stride;
	unsigned int addr;
	unsigned int offset0;
	unsigned int offset1;

	int type;
	int endian;

	// sync 
	// H
	unsigned int hss;
	unsigned int hfp;
	unsigned int hbp;
	// V
	unsigned int vss;
	unsigned int vfp;
	unsigned int vbp;

	unsigned int d_pol;
	unsigned int h_pol;
	unsigned int v_pol;
} sDISPLAY_CONFIG;

typedef struct {
	unsigned int hsize;
	unsigned int vsize;	
	unsigned int stride;
	
	unsigned int addr0;
	unsigned int addr1;
	unsigned int offset0;
	unsigned int offset1;

	int type;
	int endian;
} sCAMERA_CONFIG;

static sDISPLAY_CONFIG display_config;
static sCAMERA_CONFIG camera_config;

sVIDEO_SYSTEM_CONFIG edge_video_system_config;

int vframe_in_index_for_current_display = -1;

int run_by_default_config = 1;

static char *camera_name_dict[] = { "NONE", "CIS", "HDMI" };
static char *display_name_dict[] = { "NONE", "LCD", "HDMI" };

static char *edge_video_system_config_title = "--- EDGE VIDEO SYSTEM CONFIG ---";

#define NUM_DEFAULT_VFRAME_MAX 2

static ErvpImage* default_vframe_max[NUM_DEFAULT_VFRAME_MAX];

const int IMAGE_RESOLUTION_MAX = IMAGE_RESOLUTION_UHD4K;

static void __attribute__ ((constructor)) init_default_frame()
{
  for(int i=0; i<NUM_DEFAULT_VFRAME_MAX; i++)
  {
    default_vframe_max[i] = image_alloc_wo_internals(0,0,0);
    default_vframe_max[i]->width = get_resolution_width(IMAGE_RESOLUTION_MAX);
    default_vframe_max[i]->height = get_resolution_height(IMAGE_RESOLUTION_MAX);
    default_vframe_max[i]->addr[0] = palloc_largeram_backpart(VIDEO_FRAME_MAX_SIZE,VIDEO_FRAME_MAX_SIZE);
    default_vframe_max[i]->addr[1] = default_vframe_max[i]->addr[0] + VIDEO_SECTION_MAX_SIZE;
    default_vframe_max[i]->addr[2] = default_vframe_max[i]->addr[1] + VIDEO_SECTION_MAX_SIZE;
  }
}

static void default_vframe_max_update()
{
  //assert(edge_video_system_config.camera_resolution==edge_video_system_config.display_resolution);
  for(int i=0; i<NUM_DEFAULT_VFRAME_MAX; i++)
  {
    //default_vframe_max[i]->width = get_resolution_width(edge_video_system_config.display_resolution);
    //default_vframe_max[i]->height = get_resolution_height(edge_video_system_config.display_resolution);
    default_vframe_max[i]->format = edge_video_system_config.image_format;
    image_decide_stride(default_vframe_max[i], 1);
  }
}

static void __attribute__ ((destructor)) fini_default_frame()
{
  // DO NOT use free due to palloc_largeram_backpart
}

static inline int is_display_4k()
{
  assert(edge_video_system_config.vframe_out);
  return (edge_video_system_config.vframe_out->width > 1920);
}

static void evs_config_init()
{
  edge_video_system_config.camera_hw = CAMERA_HW_NONE;
  edge_video_system_config.display_hw = DISPLAY_HW_NONE;
  edge_video_system_config.image_format = IMAGE_FMT_XBGR_8888_PACKED;
  edge_video_system_config.camera_resolution = IMAGE_RESOLUTION_FHD;
  edge_video_system_config.display_resolution = IMAGE_RESOLUTION_FHD;
  edge_video_system_config.use_default_frame = 1;
  edge_video_system_config.vframe_out = 0;
  edge_video_system_config.vframe_in0 = 0;
  edge_video_system_config.vframe_in1 = 0;
}

void edge_video_system_setup_default_frame()
{
  default_vframe_max_update();
  if(edge_video_system_config.display_hw==DISPLAY_HW_NONE)
    edge_video_system_config.vframe_out = NULL;
  else
  {
    edge_video_system_config.vframe_out = image_generate_center_window(default_vframe_max[0], get_resolution_width(edge_video_system_config.display_resolution), get_resolution_height(edge_video_system_config.display_resolution), edge_video_system_config.vframe_out);
    image_clear(edge_video_system_config.vframe_out, 0);
  }
  if(edge_video_system_config.camera_hw==CAMERA_HW_NONE)
  {
    edge_video_system_config.vframe_in0 = NULL;
    edge_video_system_config.vframe_in1 = NULL;
  }
  else
  {
    edge_video_system_config.vframe_in0 = image_generate_center_window(default_vframe_max[0], get_resolution_width(edge_video_system_config.camera_resolution), get_resolution_height(edge_video_system_config.camera_resolution), edge_video_system_config.vframe_in0);
    edge_video_system_config.vframe_in1 = image_generate_center_window(default_vframe_max[1], get_resolution_width(edge_video_system_config.camera_resolution), get_resolution_height(edge_video_system_config.camera_resolution), edge_video_system_config.vframe_in1);
  }
  image_clear(default_vframe_max[0], 0);
  image_clear(default_vframe_max[1], 0);
  flush_cache();
}

void edge_video_system_init()
{
  evs_config_init();
  run_by_default_config = 1;
}

void edge_video_system_register_frame(ErvpImage* out, ErvpImage* in0, ErvpImage* in1)
{
  edge_video_system_config.use_default_frame = 0;
  edge_video_system_config.vframe_out = out;
  edge_video_system_config.vframe_in0 = in0;
  edge_video_system_config.vframe_in1 = in1;
}

static inline void edge_video_system_config_get_format()
{
  const char* key_list = "r 0123";
  const char* menu_list[] = {"Return","YUVPLANE","YUVINTRV","RGBPLANE","RGBINTRV"};
  char key = mcom_gui_template(edge_video_system_config_title, key_list, menu_list);
  switch(key)
  {
    case 'r':
      break;
    case '0':
      edge_video_system_config.image_format = IMAGE_FMT_YUV_420_PLANAR_YV12;
      break;
    case '1':
      edge_video_system_config.image_format = IMAGE_FMT_YUV_420_PLANAR_NV12;
      break;
    case '2':
      edge_video_system_config.image_format = IMAGE_FMT_RGB_888_PLANE;
      break;
    case '3':
      edge_video_system_config.image_format = IMAGE_FMT_XBGR_8888_PACKED;
      break;
    default:
      assert(0);
  }
}

static char* top_menu_list[] = {"Exit","","Image Format",
  "Input - None","Input - CIS","Input - HDMI (640x480)","Input - HDMI (1920x1080)","Input - HDMI (3840x2160)",
  "Output - None","Output - LCD","Output - HDMI (640x480)","Output - HDMI (1920x1080)","Output - HDMI (3840x2160)"
};

static inline char edge_video_system_config_get_top_menu()
{
  const char* key_list = "xrf 0123456789";
  
  if(run_by_default_config)
    top_menu_list[1] = "Run by Default Config";
  else
    top_menu_list[1] = "Run";
  return mcom_gui_template(edge_video_system_config_title, key_list, top_menu_list);
}

static int edge_video_system_check_exit()
{
  const char* key_list = "x";
  const char* menu_list[] = {"Exit"};
  char key = mcom_gui_template(edge_video_system_config_title, key_list, menu_list);
  assert(key=='x');
  return 1;
}

int edge_video_system_config_gui()
{
  int run = 0;
  vim_disable();
  vom_disable();
  while(1)
  {
    char key = edge_video_system_config_get_top_menu();
    if(key=='r')
    {
      printf("\n\n > RUN");
      edge_video_system_config.use_default_frame = 1;
      if(run_by_default_config)
      {
        edge_video_system_config.camera_hw = CAMERA_HW_HDMI;
        edge_video_system_config.display_hw = DISPLAY_HW_HDMI;
        edge_video_system_config.image_format = IMAGE_FMT_XBGR_8888_PACKED;
        run_by_default_config = 0;
      }
      run = 1;
      break;
    }
    run_by_default_config = 0;

    if(key=='x')
    {
      run = 0;
      break;
    }

    if(key=='f')
    {
      edge_video_system_config_get_format();
      continue;
    }

    if((key>='0')&&(key<='9'))
    {
      switch(key)
      {
        case '0':
          edge_video_system_config.camera_hw = CAMERA_HW_NONE;
          break;
        case '1':
          edge_video_system_config.camera_hw = CAMERA_HW_CIS;
          edge_video_system_config.camera_resolution = IMAGE_RESOLUTION_VGA;
          break;
        case '2':
          edge_video_system_config.camera_hw = CAMERA_HW_HDMI; 
          edge_video_system_config.camera_resolution = IMAGE_RESOLUTION_VGA;
          break;
        case '3':
          edge_video_system_config.camera_hw = CAMERA_HW_HDMI; 
          edge_video_system_config.camera_resolution = IMAGE_RESOLUTION_FHD;
          break;
        case '4':
          edge_video_system_config.camera_hw = CAMERA_HW_HDMI; 
          edge_video_system_config.camera_resolution = IMAGE_RESOLUTION_UHD4K;
          break;
        case '5':
          edge_video_system_config.display_hw = DISPLAY_HW_NONE;
          break;
        case '6':
          edge_video_system_config.display_hw = DISPLAY_HW_LCD;
          edge_video_system_config.display_resolution = IMAGE_RESOLUTION_1024x600;
          break;
        case '7':
          edge_video_system_config.display_hw = DISPLAY_HW_HDMI;
          edge_video_system_config.display_resolution = IMAGE_RESOLUTION_VGA;
          break;
        case '8':
          edge_video_system_config.display_hw = DISPLAY_HW_HDMI;
          edge_video_system_config.display_resolution = IMAGE_RESOLUTION_FHD;
          break;
        case '9':
          edge_video_system_config.display_hw = DISPLAY_HW_HDMI;
          edge_video_system_config.display_resolution = IMAGE_RESOLUTION_UHD4K;
          break;
        default:
          break;
      }
      continue;
    }

    assert(0);
  }
  return run;
}

void edge_video_system_start()
{
  if(edge_video_system_config.use_default_frame)
    edge_video_system_setup_default_frame();
  printf("\n\n%s", edge_video_system_config_title);
  int camera_hw = edge_video_system_config.camera_hw;
  printf("\n > Input: %s", camera_name_dict[camera_hw]);
  if(camera_hw)
  {
    if(camera_hw==CAMERA_HW_CIS)
      assert(edge_video_system_config.camera_resolution==IMAGE_RESOLUTION_VGA);
    printf(" (%dx%d)", edge_video_system_config.vframe_in0->width, edge_video_system_config.vframe_in0->height);
  }
  int display_hw = edge_video_system_config.display_hw;
  printf("\n > Output: %s", display_name_dict[display_hw]);
  if(display_hw)
  {
    if(display_hw==DISPLAY_HW_LCD)
      assert(edge_video_system_config.display_resolution==IMAGE_RESOLUTION_1024x600);
    printf(" (%dx%d)", edge_video_system_config.vframe_out->width, edge_video_system_config.vframe_out->height);
  }
  if(edge_video_system_config.camera_hw!=CAMERA_HW_NONE)
  {
    vframe_config_vim(edge_video_system_config.vframe_in0, edge_video_system_config.vframe_in1);
    edge_video_system_start_camera();
  }
  if(edge_video_system_config.display_hw!=DISPLAY_HW_NONE)
  {
    vframe_config_vom(edge_video_system_config.vframe_out);
    edge_video_system_start_display();
  }
  //image_print_info(edge_video_system_config.vframe_out);
  //image_print_info(edge_video_system_config.vframe_in0);
  //image_print_info(edge_video_system_config.vframe_in1);
}

void edge_video_system_start_display()
{
  assert(edge_video_system_config.display_hw!=DISPLAY_HW_NONE);
  vom_disable();
  switch(edge_video_system_config.display_hw)
  {
    case DISPLAY_HW_LCD:
      {
        vom_set_lcd_reset(0); delay_ms(10);
        vom_set_lcd_reset(3); delay_ms(100); // bit[1]:lcd pwm on
        vom_set_clkpol(1);
        vom_enable_lcd();
      }
      break;
    case DISPLAY_HW_HDMI:
      {
        vom_set_hdmi_reset(0); delay_ms(10);
        vom_set_hdmi_reset(1); delay_ms(100);
        if(is_display_4k())
        {
          vom_set_clkpol(1);
          vom_set_4kmode(1); // CLK:300MHz
          sil1136_init_sg(display_config.hsize, display_config.vsize, display_config.vss, display_config.vfp, display_config.vbp, display_config.hss, display_config.hfp, display_config.hbp, 300000000);
        }
        else
        {
          vom_set_clkpol(0);
          vom_set_4kmode(0); // CLK:150MHz
          sil1136_init_sg(display_config.hsize, display_config.vsize, display_config.vss, display_config.vfp, display_config.vbp, display_config.hss, display_config.hfp, display_config.hbp, 150000000);        
        }
        vom_enable_hdmi();
      }
      break;
    default:
      assert(0);
  }
  delay_ms(100);
}

void edge_video_system_start_camera()
{
  assert(edge_video_system_config.camera_hw!=CAMERA_HW_NONE);
  vim_disable();
  switch(edge_video_system_config.camera_hw)
  {
    case CAMERA_HW_CIS:
      {
        vim_set_cispin(0, 0); delay_ms(5);
        vim_set_cispin(1, 0); delay_ms(100);
        vim_set_clkpol(0); // CLK falling or rising

#if 0
        cis_ov2640_check_test_reg();
        cis_ov2640_write_sensor_vals(OV2640_SVGA_YUV422); // by Sukho Lee
        //cis_ov2640_write_sensor_vals(OV2640_VGA_JPEG); // by Sukho Lee
#endif

#if 0
        cis_ov5642_check_test_reg();
        //cis_ov5642_write_sensor_vals(OV5642_MP5_QVGA_YUV422); // by Sukho Lee
        cis_ov5642_write_sensor_vals(OV5642_MP5_VGA_YUV); // by Sukho Lee
#endif

#if 1
        cis_ov5640_check_test_reg();
        cis_ov5640_write_sensor_vals(OV5640YUV_Sensor_Dvp_Init); // by Sukho Lee
        cis_ov5640_write_sensor_vals(OV5640_VGA_Preview); // by Sukho Lee
#endif
        //mt9p111_init_sg();
        vim_enable_cis();
      }
      break;
    case CAMERA_HW_HDMI:
      {
        vim_set_hdmi_reset(0); delay_ms(5);
        vim_set_hdmi_reset(1); delay_ms(100);
        vim_set_clkpol(0); // CLK falling or rising  // xckup15 : 0, ultra :1 by S.Lee
        adv7619_init_ycbcr();
        vim_enable_hdmi();
      }
      break;
    default:
      assert(0);
  }
  delay_ms(100);
}

static inline int conevert_image_format_to_vom_format(int format)
{
  int result;
  switch(format)
  {
    case IMAGE_FMT_YUV_420_PLANAR_YV12:
      result = 0;
      break;
    case IMAGE_FMT_YUV_420_PLANAR_NV12:
      result = 1;
      break;
    case IMAGE_FMT_RGB_888_PLANE:
      result = 2;
      break;
    case IMAGE_FMT_XBGR_8888_PACKED:
      result = 3;
      break;
    default:
      assert(0);
  }
  return format;
}

void vframe_config_vom(const ErvpImage* vframe_out)
{
  int hsync_mask;
  assert(vframe_out);
  assert(edge_video_system_config.display_hw!=DISPLAY_HW_NONE);
  display_config.hsize = vframe_out->width;
  display_config.vsize = vframe_out->height;
  display_config.stride = vframe_out->stride[0];
  display_config.addr = vframe_out->addr[0];
  display_config.offset0 = vframe_out->addr[1] - vframe_out->addr[0];
  display_config.offset1 = vframe_out->addr[2] - vframe_out->addr[0];
  display_config.type = conevert_image_format_to_vom_format(vframe_out->format);
  display_config.endian = 0;

  switch(edge_video_system_config.display_hw)
  {
    case DISPLAY_HW_LCD:
      {
        hsync_mask = 1;
        // 1024 x 600 LCD. DataSheet ������.
        display_config.hss   = 320; display_config.hfp   = 0; display_config.hbp   = 0;
        display_config.vss   =  35; display_config.vfp   = 0; display_config.vbp   = 0;
        display_config.d_pol =   0; display_config.h_pol = 1;	display_config.v_pol = 1;
      }
      break;
    case DISPLAY_HW_HDMI:
      {
        hsync_mask = 0;
        assert(edge_video_system_config.display_resolution>=IMAGE_RESOLUTION_FHD);
        switch(edge_video_system_config.display_resolution)
        {
          case IMAGE_RESOLUTION_UHD4K:
            {
              // 4K : 300MHz. V:30, H:67.5kHz
              display_config.hss = 200; display_config.hfp = 200; display_config.hbp = 200;
              display_config.vss =  32; display_config.vfp =  30; display_config.vbp =  30;
              break;
            }
          case IMAGE_RESOLUTION_FHD:
            {
              // FHD : 150MHz. V:59.7, H:67.5kHz
              display_config.hss = 100; display_config.hfp = 100; display_config.hbp = 100;
              display_config.vss =  17; display_config.vfp =  15; display_config.vbp =  15;
              break;
            }
          default:
            assert(0);
        }
        display_config.d_pol = 0;	 display_config.h_pol = 0;		display_config.v_pol = 0;
      }
      break;
    default:
      hsync_mask = 0;
      assert(0);
  }
  vom_disable();
  vom_set_base(display_config.addr);
  vom_set_offset(display_config.offset0, display_config.offset1);
  vom_set_size(display_config.hsize, display_config.vsize);
  vom_set_stride(display_config.stride);
  vom_set_type(display_config.type);
  vom_set_syncsize(display_config.hss, display_config.vss);
  vom_set_hporch(display_config.hfp, display_config.hbp);
  vom_set_vporch(display_config.vfp, display_config.vbp);
  vom_set_sync_pol(display_config.d_pol, display_config.h_pol, display_config.v_pol, hsync_mask);
  vom_set_endian(display_config.endian);
  mmio_write_data(VOM_LCD_PATTERN, (0<<24) | (255<<16) | (0<<8) | (0)); // R G B
}

void vframe_config_vim(const ErvpImage* vframe_in0, const ErvpImage* vframe_in1)
{
  assert(vframe_in0);
  assert(vframe_in1);
  assert(image_is_same_type(vframe_in0, vframe_in1));

  camera_config.hsize = vframe_in0->width;
  camera_config.vsize = vframe_in0->height;
  camera_config.stride = vframe_in0->stride[0];
  camera_config.addr0 = vframe_in0->addr[0];
  camera_config.addr1 = vframe_in1->addr[0];
  camera_config.offset0 = vframe_in0->addr[1] - vframe_in0->addr[0];
  camera_config.offset1 = vframe_in0->addr[2] - vframe_in0->addr[0];
  camera_config.type = conevert_image_format_to_vom_format(vframe_in0->format);
  camera_config.endian = 0;

  vim_disable();
  vim_set_base0(camera_config.addr0);
	vim_set_base1(camera_config.addr1);
	vim_set_offset(camera_config.offset0, camera_config.offset1);
	vim_set_size(camera_config.hsize, camera_config.vsize);
	vim_set_stride(camera_config.stride);
	vim_set_type(camera_config.type);
	vim_set_endian(camera_config.endian);
}

static inline int vim_get_stable_index()
{
  return vim_get_idx();
}

static inline void change_vframe_out(const ErvpImage* vframe_in)
{
  const ErvpImage* original_vframe = vframe_in;
  while(1)
  {
    if(edge_video_system_config.vframe_out->width > original_vframe->width)
    {
      assert(original_vframe->is_window);
      original_vframe = original_vframe->window_info->original;
      continue;
    }
    edge_video_system_config.vframe_out = image_generate_center_window(original_vframe, edge_video_system_config.vframe_out->width, edge_video_system_config.vframe_out->height, edge_video_system_config.vframe_out);
    vom_set_base(edge_video_system_config.vframe_out->addr[0]);
    break;
  }
}

void edge_video_system_display_direct(int with_key_exit)
{
  int prev_idx = -1;
  while(1) {
    delay_ms(10); // to reduce polling overhead
    int idx = vim_get_stable_index();
    if(prev_idx != idx) {
      if(idx == 1) change_vframe_out(edge_video_system_config.vframe_in1);
      else         change_vframe_out(edge_video_system_config.vframe_in0);

      prev_idx = idx;
    }
    if(with_key_exit && edge_video_system_check_exit())
      break;
  }
  vom_disable();
  vim_disable();
}

ErvpImage* edge_video_system_capture()
{
  int prev_idx = -1;
  vframe_in_index_for_current_display = -1;
  while(1) {
    delay_ms(10); // to reduce polling overhead
    int idx = vim_get_stable_index();
    if(prev_idx != idx) {
      if(idx == 1) change_vframe_out(edge_video_system_config.vframe_in1);
      else         change_vframe_out(edge_video_system_config.vframe_in0);

      prev_idx = idx;
    }
    printf("\n\n%s", edge_video_system_config_title);
    printf("\nc. Capture ");
    if(getc_from_mcom()=='c')
      break;
  }
  vim_disable();
  vframe_in_index_for_current_display = prev_idx;
  return edge_video_system_get_displayed_vframe_in();
}

ErvpImage* edge_video_system_capture_ms(int ms)
{
  uint64_t current_clock_us, target_clock_us;
  int prev_idx = -1;
  vframe_in_index_for_current_display = -1;
  printf("\n\n%s", edge_video_system_config_title);
  printf("\nCaptured after %.1f secs", ((float)ms)/1000);
  current_clock_us = get_real_clock_tick();
  target_clock_us = current_clock_us + (((uint64_t)ms * 1000));
  while(1) {
    delay_ms(10); // to reduce polling overhead
    int idx = vim_get_stable_index();
    if(prev_idx != idx) {
      if(idx == 1) change_vframe_out(edge_video_system_config.vframe_in1);
      else         change_vframe_out(edge_video_system_config.vframe_in0);
      prev_idx = idx;
    }
    current_clock_us = get_real_clock_tick();
    if(current_clock_us >= target_clock_us)
      break;
  }
  vim_disable();
  vframe_in_index_for_current_display = prev_idx;
  return edge_video_system_get_displayed_vframe_in();
}

void edge_video_system_change_vframe_out(const ErvpImage* vframe_in)
{
  assert(edge_video_system_config.use_default_frame==1);
  change_vframe_out(vframe_in);
}
