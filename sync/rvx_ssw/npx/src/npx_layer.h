#ifndef __NPX_LAYER_H__
#define __NPX_LAYER_H__

#include "platform_info.h"

#include "ervp_printf.h"
#include "npx_struct.h"

#include <stdint.h>

char npx_layer_type_to_char(npx_layer_type_t type);
const char *npx_layer_type_to_str(npx_layer_type_t type);
npx_layerio_tsseq_t *npx_layerio_tsseq_alloc(int timesteps);
void npx_layerio_tsseq_free(npx_layerio_tsseq_t *p);

npx_layerio_tsseq_t *npx_output_tsseq_alloc(int timesteps, const int *is_boundary, float scaled, int datatype, int num_dim, int *size_array);
int npx_layer_output_size(npx_layer_type_t layer_type, void *layer);
int npx_layer_testvector_size(npx_layer_type_t layer_type, void *layer);

// weak functions
void npx_forward_conv2d_layer_default(npx_conv2d_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state);
void npx_forward_maxpool2d_layer_default(npx_maxpool2d_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state);
void npx_forward_avgpool2d_layer_default(npx_avgpool2d_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state);
void npx_forward_linear_layer_default(npx_linear_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state);
void npx_forward_flatten_layer_default(npx_flatten_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state);
void npx_forward_leaky_layer_default(npx_leaky_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state);
void npx_forward_layer_block_default(npx_layer_block_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state);
void npx_forward_conv2d_layer_reuse(npx_conv2d_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state);
// weak functions

#endif // __NPX_LAYER_H__
