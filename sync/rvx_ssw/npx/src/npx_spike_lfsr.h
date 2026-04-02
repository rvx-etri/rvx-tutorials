#ifndef __NPX_SPIKE_LFSR_H__
#define __NPX_SPIKE_LFSR_H__

#include "npx_tensor.h"

#ifdef __cplusplus
extern "C" {
#endif

NpxTensorInfo *npx_covert_lfsr_type(const NpxTensorInfo *input_tensor);
NpxTensorInfo *npx_gen_spike_lfsr(const NpxTensorInfo *input_tensor, uint8_t *plfsr);

#ifdef __cplusplus
} // exter "C"
#endif
#endif // __NPX_SPIKE_H__
