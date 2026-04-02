#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_malloc.h"
#include "ervp_memory_util.h"

#include "npx_dvs.h"

static int dvs_time_searchsorted(npx_dvs_t *npx_dvs, int stat_index, int end_index, int timestamp_to_find)
{
  int index = -1;
  for (int i = stat_index; i < end_index; i++)
  {
    if (npx_dvs[i].timestamp >= timestamp_to_find)
    {
      index = i;
      break;
    }
  }
  assert(index != (-1));
  return index;
}

static inline uint8_t *get_pixel_addr(NpxTensorInfo *frame, int step_index, int polarity, int y, int x)
{
  void *addr = frame->addr;
  addr += (x * frame->stride[0]);
  addr += (y * frame->stride[1]);
  addr += (polarity * frame->stride[2]);
  addr += (step_index * frame->stride[3]);
  return addr;
}

static void merge_spikes_to_frame(const NpxTensorInfo *dvs, NpxTensorInfo *frame, float overlap)
{
  int num_events = dvs->size[1];
  int timesteps = frame->size[3];
  npx_dvs_t *dvs_data_array = (npx_dvs_t *)dvs->addr;
  int window_size = (dvs_data_array[num_events - 1].timestamp - dvs_data_array[0].timestamp) / timesteps;
  window_size = (int)((float)window_size * (1.0 + overlap));
  int window_stride = (int)((float)window_size * (1.0 - overlap));

  int *start_timestamp = (int *)malloc(sizeof(int) * timesteps);
  int *end_timestamp = (int *)malloc(sizeof(int) * timesteps);
  for (int t = 0; t < timesteps; t++)
  {
    start_timestamp[t] = t * window_stride + dvs_data_array[0].timestamp;
    end_timestamp[t] = start_timestamp[t] + window_size;
  }

  int *indices_start = (int *)malloc(sizeof(int) * timesteps);
  int *indices_end = (int *)malloc(sizeof(int) * timesteps);

  for (int t = 0; t < timesteps; t++)
  {
    if (t == 0)
    {
      indices_start[t] = dvs_time_searchsorted(dvs_data_array, 0, num_events, start_timestamp[t]);
      indices_end[t] = dvs_time_searchsorted(dvs_data_array, 0, num_events, end_timestamp[t]);
    }
    else
    {
      indices_start[t] = dvs_time_searchsorted(dvs_data_array, indices_start[t - 1], num_events, start_timestamp[t]);
      indices_end[t] = dvs_time_searchsorted(dvs_data_array, indices_end[t - 1], num_events, end_timestamp[t]);
    }
    // printf("index [%d] %d - %d\n", t, indices_start[t], indices_end[t]);
  }

  // reset to zero
  memset(frame->addr, 0, npx_tensor_sizes(frame));

  for (int t = 0; t < timesteps; t++)
  {
    for (int i = indices_start[t]; i < indices_end[t]; i++)
    {
      if((dvs_data_array[i].y >= frame->size[1]) || (dvs_data_array[i].x >= frame->size[0])) continue;
      (*get_pixel_addr(frame, t, dvs_data_array[i].polarity, dvs_data_array[i].y, dvs_data_array[i].x)) += 1;
    }
  }
}

NpxTensorInfo *npx_dvs_to_frame(NpxTensorInfo *dvs, int sensor_size_h, int sensor_size_w, int timesteps, NpxTensorInfo *frame)
{
  NpxTensorInfo *result;

  if (frame == NULL)
  {
    result = npx_tensor_alloc_wo_data(4);
    result->size[0] = sensor_size_w;
    result->size[1] = sensor_size_h;
    result->size[2] = 2;
    result->size[3] = timesteps;
    npx_tensor_set_datatype(result, MATRIX_DATATYPE_UINT08);
    npx_tensor_alloc_data(result);
  }
  else
  {
    result = frame;
  }

  merge_spikes_to_frame(dvs, result, 0);

  return result;
}

NpxTensorInfo *npx_dvs_downsample(NpxTensorInfo *dvs, int sensor_size_h, int sensor_size_w, 
                                        int target_size_h, int target_size_w)
{
  NpxTensorInfo *result;
  float spatial_factor_h = (float)target_size_h / sensor_size_h;
  float spatial_factor_w = (float)target_size_w / sensor_size_w;

#if 0
  result = dvs;
#else
  result = npx_tensor_alloc_wo_data(2);
  result->size[0] = dvs->size[0];
  result->size[1] = dvs->size[1];
  npx_tensor_set_datatype(result, MATRIX_DATATYPE_SINT32);
  npx_tensor_alloc_data(result);
#endif

  npx_dvs_t *dvs_data_array = (npx_dvs_t *)dvs->addr;
  npx_dvs_t *result_data_array = (npx_dvs_t *)result->addr;

  for(int i = 0; i < result->size[1]; i++)
  {
    //if(i<30) printf("%d %d -> ", dvs_data_array[i].y, dvs_data_array[i].y);
    result_data_array[i].y = (uint32_t)((float)dvs_data_array[i].y * spatial_factor_h);
    result_data_array[i].x = (uint32_t)((float)dvs_data_array[i].x * spatial_factor_w);
    result_data_array[i].polarity = dvs_data_array[i].polarity;
    result_data_array[i].timestamp = dvs_data_array[i].timestamp;
    //if(i<30) printf("%d %d\n", result_data_array[i].y, result_data_array[i].y);
  }

  return result;
}

NpxTensorInfo *npx_dvs_denoise(NpxTensorInfo *dvs, int sensor_size_h, int sensor_size_w, int filter_time)
{
  int i, j;
  NpxTensorInfo *result;
  uint32_t x, y, t;

#if 0
  result = dvs;
#else
  result = npx_tensor_alloc_wo_data(2);
  result->size[0] = dvs->size[0];
  result->size[1] = dvs->size[1];
  npx_tensor_set_datatype(result, MATRIX_DATATYPE_SINT32);
  npx_tensor_alloc_data(result);
#endif

  uint32_t (*timestamp_memory)[sensor_size_w] = (uint32_t (*)[sensor_size_w])malloc(sensor_size_h*sensor_size_w*sizeof(uint32_t));
  npx_dvs_t *dvs_data_array = (npx_dvs_t *)dvs->addr;
  npx_dvs_t *result_data_array = (npx_dvs_t *)result->addr;
  
  uint32_t base_timestamp = dvs_data_array[0].timestamp;
  for (j = 0; j < sensor_size_h; j++)
  {
    for (i = 0; i < sensor_size_w; i++)
    {
      timestamp_memory[j][i] = base_timestamp + filter_time;
    }
  }

  int copy_index = 0;
  for(i = 0; i < dvs->size[1]; i++)
  {
    x = dvs_data_array[i].x;
    y = dvs_data_array[i].y;
    t = dvs_data_array[i].timestamp;
    // if (x > 259) printf("--x:%d \n", x);
    if( (x<sensor_size_w) && (y<sensor_size_h) )
    {
      // if(x>259) printf("x:%d \n", x);
      timestamp_memory[y][x] = t + filter_time;

      if(
        ((x > 0) && (timestamp_memory[y][x - 1] > t))
        || ((x < sensor_size_w - 1) && (timestamp_memory[y][x + 1] > t))
        || ((y > 0) && (timestamp_memory[y - 1][x] > t))
        || ((y < sensor_size_h - 1) && (timestamp_memory[y + 1][x] > t))
      )
      {
        result_data_array[copy_index].x = x;
        result_data_array[copy_index].y = y;
        result_data_array[copy_index].timestamp = t;
        result_data_array[copy_index].polarity = dvs_data_array[i].polarity;
        copy_index++;
      }
    }
  }
  result->size[1] = copy_index;

  free(timestamp_memory);

  return result;
}