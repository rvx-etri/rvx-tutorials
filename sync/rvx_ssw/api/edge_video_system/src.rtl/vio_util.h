#ifndef _H_VIO_UTIL_H_
#define _H_VIO_UTIL_H_

#include "ervp_mmio_util.h"
#include "ervp_video_frame.h"
#include "vi.h"
#include "vo.h"
#include "vio_define.h"

#define VOM_FRAME0 (I_SYSTEM_DDR_LASTADDR-VIDEO_FRAME_MAX_SIZE+1)
#define VIM_FRAME0 (VOM_FRAME0)
#define VIM_FRAME1 (VIM_FRAME0-VIDEO_FRAME_MAX_SIZE)

void vio_clear_buffer(unsigned int addr, unsigned int offset0, unsigned int offset1, unsigned int stride, unsigned int height, int type);

#endif
