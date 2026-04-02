#include "ervp_printf.h"
#include "ervp_malloc.h"
#include "ervp_memory_util.h"
#include "ervp_assert.h"

#include <string.h>
#include <stdint.h>

#include "npx_spike_lfsr.h"
#include "npx_struct.h"

static uint8_t next_lfsr(uint8_t *plfsr);

NpxTensorInfo *npx_gen_spike_lfsr(const NpxTensorInfo *input_tensor, uint8_t *plfsr)
{
  int c, h, w;
  static NpxTensorInfo *spike_tensor = 0;

  assert(input_tensor->datatype == MATRIX_DATATYPE_UINT08);

  if(spike_tensor == 0)
  {
    spike_tensor = npx_tensor_alloc_wo_data(3);
    spike_tensor->size[0] = input_tensor->size[0];
    spike_tensor->size[1] = input_tensor->size[1];
    spike_tensor->size[2] = input_tensor->size[2];
    npx_tensor_set_datatype(spike_tensor, MATRIX_DATATYPE_SINT08);
    npx_tensor_alloc_data(spike_tensor);
  }

  // 1. pseudo ransdom number generation (lfsr data)
  uint8_t lfsr_data = next_lfsr(plfsr);

  uint8_t *input_data = input_tensor->addr;
  npx_spike_t *spike_data = spike_tensor->addr;

  // 2. spike generation (compare input data and lfsr data)
  for (c = 0; c < input_tensor->size[2]; c++)
  {
    int c_offset = c * input_tensor->size[1] * input_tensor->size[0];
    for (h = 0; h < input_tensor->size[1]; h++)
    {
      int h_offset = h * input_tensor->size[0];
      for (w = 0; w < input_tensor->size[0]; w++)
      {
        int index = c_offset + h_offset + w;
        
        spike_data[index] = (input_data[index] > lfsr_data)? 1: 0;
      }
    }
  }

#if 0
  int i, j;
  printf("## Spike value \n");
  for (j = 0; j < input_tensor->size[1]; j++)
  {
    for (i = 0; i < input_tensor->size[0]; i++)
    {
      printf("%3d ", ((npx_spike_t *)spike_tensor->addr)[num_col * j + i]);
      // printf("%3d ", spike_data[num_col * j + i]);
    }
    printf("\n");
  }
#endif

  return spike_tensor;
}

uint8_t next_lfsr(uint8_t *plfsr)
{
  const uint8_t poly = 0xB8;
  // static uint8_t plfsr = 0xFF;
  uint8_t lsb = *plfsr & 1;
  uint8_t _poly;
  *plfsr = *plfsr >> 1;
  _poly = poly * lsb;
  *plfsr = *plfsr ^ _poly;

  return *plfsr;
}
