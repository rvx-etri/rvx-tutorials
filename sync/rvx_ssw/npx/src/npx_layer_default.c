#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_malloc.h"
#include "ervp_assert.h"
#include "ervp_math.h"
#include "ervp_matrix_op_sw.h"
#include "ervp_special_matrix_op.h"

#include <string.h>
#include <stdint.h>
#include <limits.h>

#include "npx_tensor.h"
#include "npx_layer.h"
#include "npx_network.h"
#include "npx_profiling.h"

char npx_layer_type_to_char(npx_layer_type_t type)
{
  char result = 0;
  switch (type)
  {
  case NPXL_CONV2D:
    result = 'c';
    break;
  case NPXL_LINEAR:
    result = 'f';
    break;
  case NPXL_MAXPOOL2D:
    result = 'm';
    break;
  case NPXL_AVGPOOL2D:
    result = 'a';
    break;
  case NPXL_LEAKY:
    result = 'n';
    break;
  case NPXL_FLATTEN:
    result = '-';
    break;
  case NPXL_BLOCK:
    result = 'b';
    break;
  default:
    assert(0);
  }
  return result;
}

const char *npx_layer_type_to_str(npx_layer_type_t type)
{
  const char *result = NULL;
  switch (type)
  {
  case NPXL_CONV2D:
    result = "NPXL_CONV2D";
    break;
  case NPXL_LINEAR:
    result = "NPXL_LINEAR";
    break;
  case NPXL_MAXPOOL2D:
    result = "NPXL_MAXPOOL2D";
    break;
  case NPXL_AVGPOOL2D:
    result = "NPXL_AVGPOOL2D";
    break;
  case NPXL_LEAKY:
    result = "NPXL_LEAKY";
    break;
  case NPXL_FLATTEN:
    result = "NPXL_FLATTEN";
    break;
  case NPXL_BLOCK:
    result = "NPXL_BLOCK";
    break;
  default:
    assert(0);
  }
  return result;
}

int npx_layer_output_size(npx_layer_type_t layer_type, void *layer)
{
  assert(layer);
  int outputs = 0;
  if (layer_type == NPXL_BLOCK)
  {
    npx_layer_block_t *p = (npx_layer_block_t *)layer;
    assert(p->layer_compute_seq);
    npx_layer_compute_t *layer_compute = p->layer_compute_seq[p->num_layer - 1];
    assert(layer_compute);
    assert(layer_compute->layer_type != NPXL_BLOCK);
    outputs = npx_layer_output_size(layer_compute->layer_type, layer_compute->layer);
  }
  else
  {
    npx_layer2d_iodata_t *p = &(((npx_conv2d_layer_t *)layer)->iodata);
    outputs = p->out_size[0] * p->out_size[1] * p->out_channels;
  }
  return outputs;
}

int npx_layer_testvector_size(npx_layer_type_t layer_type, void *layer)
{
  assert(layer);
  int outputs = 0;
  if (layer_type == NPXL_BLOCK)
  {
    npx_layer_block_t *p = (npx_layer_block_t *)layer;
    assert(p->layer_compute_seq);
    for (int i = 0; i < p->num_layer; i++)
    {
      npx_layer_compute_t *layer_compute = p->layer_compute_seq[i];
      assert(layer_compute);
      assert(layer_compute->layer_type != NPXL_BLOCK);
      outputs += npx_layer_output_size(layer_compute->layer_type, layer_compute->layer);
    }
  }
  else
    outputs = npx_layer_output_size(layer_type, layer);
  return outputs;
}

npx_layerio_tsseq_t *npx_layerio_tsseq_alloc(int timesteps)
{
  npx_layerio_tsseq_t *result = malloc(sizeof(npx_layerio_tsseq_t));
  result->timesteps = timesteps;
  result->sequence = (NpxTensorInfo **)malloc(sizeof(NpxTensorInfo *) * timesteps);
  for (int i = 0; i < timesteps; i++)
    result->sequence[i] = NULL;
  result->is_boundary = NULL;
  result->scaled = 0;
  return result;
}

void npx_layerio_tsseq_free(npx_layerio_tsseq_t *p)
{
  assert(p);
  if (p->sequence)
  {
    for (int i = 0; i < p->timesteps; i++)
      if (p->sequence[i])
        npx_tensor_free(p->sequence[i]);
    free(p->sequence);
  }
  free(p);
}

npx_layerio_tsseq_t *npx_output_tsseq_alloc(int timesteps, const int *is_boundary, float scaled, int datatype, int num_dim, int *size_array)
{
  npx_layerio_tsseq_t *output_tsseq = npx_layerio_tsseq_alloc(timesteps);
  output_tsseq->scaled = scaled;
  output_tsseq->is_boundary = is_boundary;
  void *buffer = NULL;

  NpxTensorInfo *all_output_tensor = npx_tensor_alloc_wo_data(num_dim);
  assert(all_output_tensor);
  npx_tensor_set_sizes(all_output_tensor, size_array);
  npx_tensor_set_datatype(all_output_tensor, datatype);
  npx_tensor_set_contiguous_layout(all_output_tensor);
  const int tensor_size = npx_tensor_sizes(all_output_tensor);
  all_output_tensor->addr = malloc(timesteps * tensor_size);
  all_output_tensor->is_array_allocated = 1;

  for (int i = 0; i < timesteps; i++)
  {
    NpxTensorInfo *output_tensor = npx_tensor_generate_subtensor_info(all_output_tensor);
    assert(output_tensor);
    output_tensor->addr = output_tensor->addr + (i * tensor_size);
    output_tsseq->sequence[i] = output_tensor;
  }
  npx_tensor_free(all_output_tensor);
  return output_tsseq;
}

__attribute__((weak)) void npx_forward_flatten_layer_default(npx_flatten_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state)
{
  printf_function();
  NPX_PROFILING_START();
  assert_pointer(2, layer, state);
  assert(state->output_tsseq == NULL);

  const int timesteps = state->input_tsseq->timesteps;
  const NpxTensorInfo *const input_tensor = state->input_tsseq->sequence[0];
  int size[2];
  size[0] = 1;
  size[1] = npx_tensor_elements(input_tensor);
  state->output_tsseq = npx_output_tsseq_alloc(timesteps, state->input_tsseq->is_boundary, state->input_tsseq->scaled, input_tensor->datatype, 2, size);

  for (int i = 0; i < timesteps; i++)
  {
    NpxTensorInfo *input_tensor = state->input_tsseq->sequence[i];
    NpxTensorInfo *output_tensor = state->output_tsseq->sequence[i];
    assert(input_tensor);
    assert(output_tensor);
    npx_tensor_reshape(input_tensor, output_tensor);
    output_tensor->is_binary = input_tensor->is_binary;
  }
  NPX_PROFILING_END();
}

__attribute__((weak)) void npx_forward_maxpool2d_layer_default(npx_maxpool2d_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state)
{
  printf_function();
  NPX_PROFILING_START();
  assert_pointer(2, layer, state);
  assert(state->output_tsseq == NULL);
  assert(layer->pad_options.br.mode == PADMODE_NONE);

  const int timesteps = state->input_tsseq->timesteps;
  const NpxTensorInfo *const input_tensor = state->input_tsseq->sequence[0];
  assert(layer->iodata.in_size[0] == input_tensor->size[0]);
  assert(layer->iodata.in_size[1] == input_tensor->size[1]);
  assert(layer->iodata.out_size[0] == (((input_tensor->size[0] + 2 * layer->pad_options.br.num_rowd - layer->kernel_size) / layer->stride) + 1));
  assert(layer->iodata.out_size[1] == (((input_tensor->size[1] + 2 * layer->pad_options.br.num_rowd - layer->kernel_size) / layer->stride) + 1));

  int size[3];
  size[0] = layer->iodata.out_size[0];
  size[1] = layer->iodata.out_size[1];
  size[2] = input_tensor->size[2];
  state->output_tsseq = npx_output_tsseq_alloc(timesteps, state->input_tsseq->is_boundary, state->input_tsseq->scaled, input_tensor->datatype, 3, size);

  ervp_mdownsample_option_t downsample_option;
  downsample_option.value = 0;
  downsample_option.br.stride_m1 = layer->stride - 1;
  downsample_option.br.downsample_mode = DOWNSAMPLE_MAX;

  ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
  for (int i = 0; i < timesteps; i++)
  {
    NpxTensorInfo *input_tensor3d = state->input_tsseq->sequence[i];
    NpxTensorInfo *output_tensor3d = state->output_tsseq->sequence[i];
    assert(input_tensor3d);
    assert(output_tensor3d);
    ErvpMatrixInfo *input_matrix = NULL;
    ErvpMatrixInfo *output_matrix = NULL;

    for (int j = 0; j < layer->iodata.out_channels; j++)
    {
      input_matrix = npx_tensor_to_iterative_matrix_info(input_tensor3d, 1, input_matrix);
      output_matrix = npx_tensor_to_iterative_matrix_info(output_tensor3d, 1, output_matrix);
      hwtask_busy_fx = mop_mapping->matrix_downsample(mop_mapping, input_matrix, output_matrix, downsample_option.value);
    }
    hwtask_wait_complete(hwtask_busy_fx);
    hwtask_busy_fx = NULL;
    matrix_free(input_matrix);
    matrix_free(output_matrix);
  }
  hwtask_wait_complete(hwtask_busy_fx);
  NPX_PROFILING_END();
}

__attribute__((weak)) void npx_forward_avgpool2d_layer_default(npx_avgpool2d_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state)
{
  printf_function();
  NPX_PROFILING_START();
  assert_pointer(2, layer, state);
  assert(state->output_tsseq == NULL);
  assert(layer->pad_options.br.mode == PADMODE_NONE);

  const int timesteps = state->input_tsseq->timesteps;
  const NpxTensorInfo *const input_tensor = state->input_tsseq->sequence[0];
  assert(layer->iodata.in_size[0] == input_tensor->size[0]);
  assert(layer->iodata.in_size[1] == input_tensor->size[1]);
  assert(layer->iodata.out_size[0] == (((input_tensor->size[0] + 2 * layer->pad_options.br.num_rowd - layer->kernel_size) / layer->stride) + 1));
  assert(layer->iodata.out_size[1] == (((input_tensor->size[1] + 2 * layer->pad_options.br.num_rowd - layer->kernel_size) / layer->stride) + 1));

  int size[3];
  size[0] = ((input_tensor->size[0] + 2 * layer->pad_options.br.num_rowd - layer->kernel_size) / layer->stride) + 1;
  size[1] = ((input_tensor->size[1] + 2 * layer->pad_options.br.num_rowd - layer->kernel_size) / layer->stride) + 1;
  size[2] = input_tensor->size[2];
  state->output_tsseq = npx_output_tsseq_alloc(timesteps, state->input_tsseq->is_boundary, state->input_tsseq->scaled, input_tensor->datatype, 3, size);
  state->output_tsseq->scaled *= (layer->kernel_size * layer->kernel_size);

  ervp_mdownsample_option_t downsample_option;
  downsample_option.value = 0;
  downsample_option.br.stride_m1 = layer->stride - 1;
  downsample_option.br.downsample_mode = DOWNSAMPLE_SUM;

  ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
  for (int i = 0; i < timesteps; i++)
  {
    NpxTensorInfo *input_tensor3d = state->input_tsseq->sequence[i];
    NpxTensorInfo *output_tensor3d = state->output_tsseq->sequence[i];
    assert(input_tensor3d);
    assert(output_tensor3d);
    ErvpMatrixInfo *input_matrix = NULL;
    ErvpMatrixInfo *output_matrix = NULL;

    for (int j = 0; j < layer->iodata.out_channels; j++)
    {
      input_matrix = npx_tensor_to_iterative_matrix_info(input_tensor3d, 1, input_matrix);
      output_matrix = npx_tensor_to_iterative_matrix_info(output_tensor3d, 1, output_matrix);
      // _matrix_advpool(layer, input_matrix, output_matrix);
      hwtask_busy_fx = mop_mapping->matrix_downsample(mop_mapping, input_matrix, output_matrix, downsample_option.value);
    }
    hwtask_wait_complete(hwtask_busy_fx);
    hwtask_busy_fx = NULL;
    matrix_free(input_matrix);
    matrix_free(output_matrix);
  }
  NPX_PROFILING_END();
}

__attribute__((weak)) void npx_forward_conv2d_layer_default(npx_conv2d_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state)
{
  printf_function();
  NPX_PROFILING_START();
  // NOT supported
  if (layer->pad_options.br.num_rowd > 0)
    assert(layer->pad_options.br.mode == PADMODE_ZEROS);
  assert_pointer(2, layer, state);
  assert(state->output_tsseq == NULL);

  const int timesteps = state->input_tsseq->timesteps;
  const NpxTensorInfo *const input_tensor = state->input_tsseq->sequence[0];
  assert(layer->iodata.in_size[0] == input_tensor->size[0]);
  assert(layer->iodata.in_size[1] == input_tensor->size[1]);
  assert(layer->iodata.out_size[0] == (((input_tensor->size[0] + 2 * layer->pad_options.br.num_rowd - layer->kernel_size) / layer->stride) + 1));
  assert(layer->iodata.out_size[1] == (((input_tensor->size[1] + 2 * layer->pad_options.br.num_rowd - layer->kernel_size) / layer->stride) + 1));

  int size[3];
  size[0] = layer->iodata.out_size[0];
  size[1] = layer->iodata.out_size[1];
  size[2] = layer->iodata.out_channels;
  int datatype = layer->iodata.out_is_quantized ? MATRIX_DATATYPE_SINT32 : MATRIX_DATATYPE_FLOAT32;
  state->output_tsseq = npx_output_tsseq_alloc(timesteps, state->input_tsseq->is_boundary, state->input_tsseq->scaled, datatype, 3, size);

  ervp_mconv_option_t conv_option;
  conv_option.value = 0;
  conv_option.br.acc = 1;
  conv_option.value = matrix_conv_set_pad(conv_option.value, layer->pad_options.br.num_rowd, layer->pad_options.br.mode);
  conv_option.br.stride_m1 = layer->stride - 1;

  for (int i = 0; i < timesteps; i++)
  {
    NpxTensorInfo *input_tensor3d = state->input_tsseq->sequence[i];
    NpxTensorInfo *output_tensor3d = state->output_tsseq->sequence[i];
    assert(input_tensor3d);
    assert(output_tensor3d);
    ErvpMatrixInfo *weight_matrix = NULL;
    ErvpMatrixInfo *output_matrix = NULL;

    for (int j = 0; j < layer->iodata.out_channels; j++)
    {
      output_matrix = npx_tensor_to_iterative_matrix_info(output_tensor3d, 1, output_matrix);
      matrix_zero_sw(output_matrix);
      ErvpMatrixInfo *input_matrix = NULL;
      for (int k = 0; k < layer->iodata.in_channels; k++)
      {
        input_matrix = npx_tensor_to_iterative_matrix_info(input_tensor3d, 1, input_matrix);
        weight_matrix = npx_tensor_to_iterative_matrix_info(layer->weight_tensor, 1, weight_matrix);

        matrix_conv_sw(input_matrix, weight_matrix, output_matrix, conv_option.value);
      }
      matrix_free(input_matrix);
    }
    matrix_free(weight_matrix);
    matrix_free(output_matrix);
  }
  NPX_PROFILING_END();
}

__attribute__((weak)) void npx_forward_linear_layer_default(npx_linear_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state)
{
  printf_function();
  NPX_PROFILING_START();

  ervp_hwtask_busy_fx_t (*p_matrix_mult)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
  if (mop_mapping == NULL)
    p_matrix_mult = _matrix_mult_sw;
  else
    p_matrix_mult = mop_mapping->matrix_mult;
  assert_pointer(2, layer, state);
  assert(state->input_tsseq != NULL);
  assert(state->output_tsseq == NULL);
  assert(layer->iodata.in_channels == layer->iodata.out_channels);

  const int timesteps = state->input_tsseq->timesteps;
  const NpxTensorInfo *const input_tensor = state->input_tsseq->sequence[0];
  int size[2];
  size[0] = 1;
  size[1] = layer->out_features;
  int datatype = matrix_datatype_is_float(input_tensor->datatype) ? MATRIX_DATATYPE_FLOAT32 : MATRIX_DATATYPE_SINT32;
  state->output_tsseq = npx_output_tsseq_alloc(timesteps, state->input_tsseq->is_boundary, state->input_tsseq->scaled, datatype, 2, size);

  ErvpMatrixInfo *weight_matrix = npx_tensor_to_matrix_info(layer->weight_tensor, NULL);
  ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;

  for (int i = 0; i < timesteps; i++)
  {
    NpxTensorInfo *input_tensor3d = state->input_tsseq->sequence[i];
    NpxTensorInfo *output_tensor3d = state->output_tsseq->sequence[i];
    assert(input_tensor3d);
    assert(output_tensor3d);
    ErvpMatrixInfo *input_vector = NULL;
    ErvpMatrixInfo *output_vector = NULL;

    for (int j = 0; j < layer->iodata.out_channels; j++)
    {
      input_vector = npx_tensor_to_iterative_matrix_info(input_tensor3d, 1, input_vector);
      output_vector = npx_tensor_to_iterative_matrix_info(output_tensor3d, 1, output_vector);
      hwtask_busy_fx = p_matrix_mult(mop_mapping, weight_matrix, input_vector, output_vector, 0);
    }
    hwtask_wait_complete(hwtask_busy_fx);
    hwtask_busy_fx = NULL;
    matrix_free(input_vector);
    matrix_free(output_vector);
  }
  matrix_free(weight_matrix);
  NPX_PROFILING_END();
}

static void matrix_compare_reset_decay(npx_leaky_layer_t *layer, int mp_scale, ErvpMatrixInfo *mp_matrix, ErvpMatrixInfo *output_matrix)
{
  // NOT supported
  assert(layer->has_reset_delay);

  int threshold_scaled = layer->threshold * mp_scale;
  if (matrix_datatype_is_float(mp_matrix->datatype))
  {
    assert(0);
  }
  else
  {
    for (int i = 0; i < mp_matrix->num_row; i++)
      for (int j = 0; j < mp_matrix->num_col; j++)
      {
        int diff;
        int mp = matrix_read_fixed_element(mp_matrix, i, j);
        int spike = (mp > threshold_scaled); // NOT >=
        matrix_write_fixed_element(output_matrix, i, j, spike);
        int value = 0;
        if (layer->is_hard_reset)
        {
          if (spike)
            value = 0;
          else if (layer->does_decay) // mp * layer->beta
            value = math_div_by_shift(mp * layer->beta_numerator, layer->beta_denominator_rsa);
        }
        else
        {
          value = mp;
          if (layer->does_decay) // mp * layer->beta
            value = math_div_by_shift(mp * layer->beta_numerator, layer->beta_denominator_rsa);
          if (spike)
            value -= threshold_scaled;
        }
        if (spike || layer->does_decay)
          matrix_write_fixed_element(mp_matrix, i, j, value);
      }
  }
}

static ervp_hwtask_busy_fx_t *scale_tensor(ervp_mop_mapping_t *mop_mapping, NpxTensorInfo *input3d, int scale_factor, NpxTensorInfo *output3d)
{
  ErvpMatrixInfo input_matrix;
  ErvpMatrixInfo output_matrix;
  npx_tensor_to_flattened_matrix_info(input3d, &input_matrix);
  npx_tensor_to_flattened_matrix_info(output3d, &output_matrix);

  ervp_hwtask_busy_fx_t (*p_matrix_scalar_mult_fixed)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, int scalar_value, ErvpMatrixInfo *c, unsigned int option_value);
  if (mop_mapping == NULL)
    p_matrix_scalar_mult_fixed = _matrix_scalar_mult_fixed_sw;
  else
    p_matrix_scalar_mult_fixed = mop_mapping->matrix_scalar_mult_fixed;

  return p_matrix_scalar_mult_fixed(mop_mapping, &input_matrix, scale_factor, &output_matrix, 0);
}

__attribute__((weak)) void npx_forward_leaky_layer_default(npx_leaky_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state)
{
  printf_function();
  NPX_PROFILING_START();

  ervp_hwtask_busy_fx_t (*p_matrix_add)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
  if (mop_mapping == NULL)
    p_matrix_add = _matrix_add_sw;
  else
    p_matrix_add = mop_mapping->matrix_add;

  ervp_hwtask_busy_fx_t (*p_matrix_scalar_mult_fixed)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, int scalar_value, ErvpMatrixInfo *c, unsigned int option_value);
  if (mop_mapping == NULL)
    p_matrix_scalar_mult_fixed = _matrix_scalar_mult_fixed_sw;
  else
    p_matrix_scalar_mult_fixed = mop_mapping->matrix_scalar_mult_fixed;

  assert_pointer(2, layer, state);
  const int timesteps = state->input_tsseq->timesteps;
  assert(state->input_tsseq);
  assert(state->input_tsseq->is_boundary);
  assert(state->output_tsseq == NULL);
  const NpxTensorInfo *const input_tensor = state->input_tsseq->sequence[0];
  state->output_tsseq = npx_output_tsseq_alloc(timesteps, state->input_tsseq->is_boundary, 1, MATRIX_DATATYPE_SINT08, input_tensor->num_dim, input_tensor->size);
  NpxTensorInfo *temp_input_buffer = NULL;
  ErvpMatrixInfo temp_input_buffer_matrix;
  int input_scalar_factor = 1;

  ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
  if (state->input_tsseq->scaled != layer->membrane_potential_scaled)
  {
    assert(state->input_tsseq->scaled >= 1);
    int lcm = math_lcm(state->input_tsseq->scaled, layer->membrane_potential_scaled);
    if (lcm != layer->membrane_potential_scaled)
    {
      int scalar_factor = lcm / layer->membrane_potential_scaled;
      // matrix_scalar_mult_fixed_sw(layer->membrane_potential_total, scalar_factor, layer->membrane_potential_total, 0);
      hwtask_busy_fx = p_matrix_scalar_mult_fixed(mop_mapping, layer->membrane_potential_total, scalar_factor, layer->membrane_potential_total, 0);
      layer->membrane_potential_scaled = lcm;
    }
    if (lcm != state->input_tsseq->scaled)
    {
      input_scalar_factor = lcm / state->input_tsseq->scaled;
      temp_input_buffer = npx_tensor_alloc(input_tensor->datatype, input_tensor->num_dim, input_tensor->size);
      npx_tensor_to_flattened_matrix_info(temp_input_buffer, &temp_input_buffer_matrix);
    }
  }

  for (int i = 0; i < timesteps; i++)
  {
    NpxTensorInfo *input_tensor3d = state->input_tsseq->sequence[i];
    NpxTensorInfo *output_tensor3d = state->output_tsseq->sequence[i];
    output_tensor3d->is_binary = 1;
    assert(input_tensor3d);
    assert(output_tensor3d);

    NpxTensorInfo *scaled_input_tensor3d;
    ErvpMatrixInfo *scaled_input_flatten_matrix;
    ErvpMatrixInfo input_flatten_matrix;
    npx_tensor_to_flattened_matrix_info(input_tensor3d, &input_flatten_matrix);
    if (input_scalar_factor == 1)
    {
      scaled_input_tensor3d = input_tensor3d;
      scaled_input_flatten_matrix = &input_flatten_matrix;
    }
    else
    {
      hwtask_busy_fx = p_matrix_scalar_mult_fixed(mop_mapping, &input_flatten_matrix, input_scalar_factor, &temp_input_buffer_matrix, 0);
      scaled_input_tensor3d = temp_input_buffer;
      scaled_input_flatten_matrix = &temp_input_buffer_matrix;
    }
    hwtask_wait_complete(hwtask_busy_fx);
    hwtask_busy_fx = p_matrix_add(mop_mapping, scaled_input_flatten_matrix, layer->membrane_potential_total, layer->membrane_potential_total, 0);

    hwtask_wait_complete(hwtask_busy_fx);
    hwtask_busy_fx = NULL;
    if (state->input_tsseq->is_boundary[i])
    {
      ErvpMatrixInfo *input_matrix = NULL;
      ErvpMatrixInfo *output_matrix = NULL;
      for (int j = 0; j < layer->iodata.out_channels; j++)
      {
        input_matrix = npx_tensor_to_iterative_matrix_info(scaled_input_tensor3d, 1, input_matrix);
        output_matrix = npx_tensor_to_iterative_matrix_info(output_tensor3d, 1, output_matrix);
        matrix_compare_reset_decay(layer, layer->membrane_potential_scaled, layer->membrane_potential[j], output_matrix);
      }
      matrix_free(input_matrix);
      matrix_free(output_matrix);
    }
  }
  if (temp_input_buffer)
    npx_tensor_free(temp_input_buffer);
  NPX_PROFILING_END();
}

__attribute__((weak)) void npx_forward_layer_block_default(npx_layer_block_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state)
{
  state->output_tsseq = npx_foward_layers(layer->layer_compute_seq, state->input_tsseq, 0, layer->num_layer);
}