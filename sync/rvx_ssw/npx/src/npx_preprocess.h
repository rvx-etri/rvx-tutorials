#ifndef __NPX_PREPROCESS_H__
#define __NPX_PREPROCESS_H__

#include "npx_struct.h"
#include "npx_tensor.h"

#ifdef __cplusplus
extern "C" {
#endif

npx_layerio_tsseq_t *npx_preprocess(const char *pre_fname, const npx_network_t *net, const NpxTensorInfo *input, float input_scale);
// npx_layerio_tsseq_t *npx_dvs346_preprocess(const npx_network_t *net, const NpxTensorInfo *input, texpar_list_t *option_list);
npx_layerio_tsseq_t *npx_dvs346_preprocess(const npx_network_t *net, const NpxTensorInfo *input, int timesteps);

#ifdef __cplusplus
} // exter "C"
#endif

#endif // __NPX_PREPROCESS_H__
