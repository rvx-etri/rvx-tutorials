#ifndef __NPX_DVS_H__
#define __NPX_DVS_H__

#include <stdint.h>

#include "npx_tensor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint32_t x;
  uint32_t y;
  uint32_t polarity;
  uint32_t timestamp;
} npx_dvs_t;

NpxTensorInfo *npx_dvs_to_frame(NpxTensorInfo *dvs, int sensor_size_h, int sensor_size_w, int timesteps, NpxTensorInfo *frame);
NpxTensorInfo *npx_dvs_downsample(NpxTensorInfo *dvs, int sensor_size_h, int sensor_size_w, int target_size_h, int target_size_w);
NpxTensorInfo *npx_dvs_denoise(NpxTensorInfo *dvs, int sensor_size_h, int sensor_size_w, int filter_time);

#ifdef __cplusplus
} // exter "C"
#endif

#endif // __NPX_DVS_H__
