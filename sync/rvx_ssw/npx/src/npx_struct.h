#ifndef __NPX_STRUCT_H__
#define __NPX_STRUCT_H__

#include "ervp_printf.h"
#include "ervp_matrix_op.h"
#include "npx_tensor.h"

#include <stdint.h>

typedef enum
{
  NPXL_CONV2D,
  NPXL_LINEAR,
  NPXL_MAXPOOL2D,
  NPXL_AVGPOOL2D,
  NPXL_LEAKY,
  NPXL_FLATTEN,
  NPXL_BLOCK
} npx_layer_type_t;

typedef struct
{
  int timesteps;
  NpxTensorInfo **sequence;
  float scaled;
  const int *is_boundary;
} npx_layerio_tsseq_t;

typedef int8_t npx_spike_t;
typedef int32_t npx_threshold_t;
typedef float npx_beta_t;

typedef struct
{
  int in_channels;
  int in_size[2];
  int in_is_quantized;
  int in_is_binary;
  int out_channels;
  int out_size[2];
  int out_is_quantized;
  int out_is_binary;
} npx_layer2d_iodata_t;

typedef struct
{
  npx_layer2d_iodata_t iodata;
  int kernel_size;
  int stride;
  ervp_mpad_option_t pad_options;
  NpxTensorInfo *weight_tensor;
  ErvpMatrixInfo **weight_matrix_info_list_for_output_reuse;
  ErvpMatrixInfo **weight_matrix_info_list_for_input_reuse;
} npx_conv2d_layer_t;

typedef struct
{
  npx_layer2d_iodata_t iodata;
  int kernel_size;
  int stride;
  ervp_mpad_option_t pad_options;
} npx_pool2d_layer_t;

typedef npx_pool2d_layer_t npx_maxpool2d_layer_t;
typedef npx_pool2d_layer_t npx_avgpool2d_layer_t;

typedef struct
{
  npx_layer2d_iodata_t iodata;
  npx_beta_t beta;
  int beta_denominator_rsa;
  int beta_denominator;
  int beta_numerator;
  const char *reset_mechanism;
  const char *reset_delay;
  npx_threshold_t threshold;
  ErvpMatrixInfo *membrane_potential_total;
  ErvpMatrixInfo **membrane_potential;
  int membrane_potential_scaled;
  //
  int is_hard_reset : 1;
  int is_soft_reset : 1;
  int has_reset_delay : 1;
  int does_decay : 1;
} npx_leaky_layer_t;

typedef struct
{
  npx_layer2d_iodata_t iodata;
} npx_flatten_layer_t;

typedef struct
{
  npx_layer2d_iodata_t iodata;
  int in_features;
  int out_features;
  NpxTensorInfo *weight_tensor;
} npx_linear_layer_t;

typedef struct
{
  const npx_layerio_tsseq_t *input_tsseq;
  npx_layerio_tsseq_t *output_tsseq;
} npx_layerio_state_t;

typedef struct
{
  npx_layer_type_t layer_type;
  void *layer;
  ervp_mop_mapping_t *mop_mapping;
  const char *operator;
  void (*forward)(void *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state);
} npx_layer_compute_t;

typedef struct
{
  int num_layer;
  npx_layer_compute_t **layer_compute_seq;
} npx_layer_block_t;

typedef struct
{
  int num_layer;
  npx_layer_compute_t **layer_compute_seq;
  //
  int h;
  int w;
  int c;
  int classes;
  char *neuron_type;
} npx_network_t;

typedef enum
{
  MATRIX4D,
  MATRIX3D,
  DVS,
  WAVEFORM,
  UNKNOWN
} npx_rawinput_type_t;

typedef struct
{
  NpxTensorInfo *tensor;
  npx_rawinput_type_t type;
  float scaled;
  int32_t label;
} npx_rawinput_t;

typedef struct
{
  int tv_size;
  int8_t *tv_data;
  int num_layer;
  int *layer_output_size;
  int *testvector_acc_size;
} npx_testvector_t;

// typedef struct npx_pre_info
//{
//   int c;
//   int h;
//   int w;
//   int timesteps;
//   char *step_generation;
// } npx_pre_info_t;

#endif // __NPX_STRUCT_H__
