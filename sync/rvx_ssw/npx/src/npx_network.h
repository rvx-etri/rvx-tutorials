#ifndef __NPX_NETWORK_H__
#define __NPX_NETWORK_H__

#include "ervp_printf.h"
#include "ervp_malloc.h"
#include "npx_struct.h"

#include <stdint.h>

static inline npx_layer_compute_t *npx_layer_compute_alloc()
{
  return (npx_layer_compute_t *)calloc(1, sizeof(npx_layer_compute_t));
}

static inline npx_layer_compute_t **npx_layer_compute_seq_alloc(int num_layer)
{
  return (npx_layer_compute_t **)calloc(num_layer, sizeof(npx_layer_compute_t *));
}

static inline void npx_layer_compute_seq_free(npx_layer_compute_t **npx_layer_compute_seq)
{
  free(npx_layer_compute_seq);
}

void npx_layerio_tsseq_print(npx_layerio_tsseq_t *tsseq);

void npx_network_print(const npx_network_t *net);
void npx_network_load_parameters(const npx_network_t *net, const char *filename);
npx_testvector_t *npx_load_testvector(const char *tv_fname, npx_network_t *net);

void npx_verify_with_testvector(npx_layerio_tsseq_t *output_tsseq, npx_testvector_t *tv, int layer_index);
int npx_classify(npx_network_t *net, const npx_layerio_tsseq_t *output_tsseq, int min_acc_value, const npx_rawinput_t *sample);

void npx_network_reset(npx_network_t *net);
void npx_network_map_matrix_operator(npx_network_t *net, int layer_index, ervp_mop_mapping_t *mop_mapping);

// weak functions
npx_layerio_tsseq_t *npx_foward_layers(npx_layer_compute_t **layer_compute_seq, const npx_layerio_tsseq_t *input_tsseq, int layer_start_index, int layer_end_index);
npx_layerio_tsseq_t *npx_inference(npx_network_t *net, const npx_layerio_tsseq_t *input_tsseq, int layer_start_index, int layer_end_index);
void npx_network_optimize(npx_network_t *net, const char *pattern, int num_change, const char *operator, void (*forward)(void *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state));
// weak functions

#endif // __NPX_NETWORK_H__
