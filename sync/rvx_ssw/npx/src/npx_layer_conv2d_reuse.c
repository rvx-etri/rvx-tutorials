#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_malloc.h"
#include "ervp_assert.h"
#include "ervp_round_int.h"
#include "ervp_math.h"
#include "ervp_matrix_op_sw.h"
#include "ervp_matrix_op_transform.h"
#include "ervp_special_matrix_op.h"
#include "npx_profiling.h"

#include <string.h>
#include <stdint.h>
#include <limits.h>

#include "npx_tensor.h"
#include "npx_layer.h"

static void npx_forward_conv2d_layer_matrix_old(npx_conv2d_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state)
{
  if (mop_mapping == NULL)
    npx_forward_conv2d_layer(layer, NULL, state);
  else
  {
    printf_function();
    assert(mop_mapping);
    assert_pointer(2, layer, state);

    const int timesteps = state->input_tsseq->timesteps;
    assert(state->output_tsseq == NULL);
    assert(layer->iodata.out_channels > 0);
    assert(timesteps > 0);

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

    ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
    ervp_mconv_option_t conv_option;
    conv_option.value = 0;
    conv_option.br.acc = 1;

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
        hwtask_wait_complete(hwtask_busy_fx);
        mop_mapping->matrix_zero(mop_mapping, output_matrix);
        ErvpMatrixInfo *input_matrix = NULL;
        for (int k = 0; k < layer->iodata.in_channels; k++)
        {
          input_matrix = npx_tensor_to_iterative_matrix_info(input_tensor3d, 1, input_matrix);
          weight_matrix = npx_tensor_to_iterative_matrix_info(layer->weight_tensor, 1, weight_matrix);

          hwtask_wait_complete(hwtask_busy_fx);
          hwtask_busy_fx = mop_mapping->matrix_conv(mop_mapping, input_matrix, weight_matrix, output_matrix, conv_option.value);
        }
        hwtask_wait_complete(hwtask_busy_fx);
        hwtask_busy_fx = NULL;
        matrix_free(input_matrix);
      }
      matrix_free(weight_matrix);
      matrix_free(output_matrix);
    }
  }
}

static void _forward_conv2d_layer_matrix_sharedoutput(npx_conv2d_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state)
{
  printf_function();
  assert(mop_mapping->matrix_conv_sharedoutput);

  const int timesteps = state->input_tsseq->timesteps;
  assert(timesteps > 0);
  assert(layer->iodata.out_channels > 0);

  assert(layer->pad_options.br.num_rowd == 0);

  int size[3];
  size[0] = layer->iodata.out_size[0];
  size[1] = layer->iodata.out_size[1];
  size[2] = layer->iodata.out_channels;
  int datatype = layer->iodata.out_is_quantized ? MATRIX_DATATYPE_SINT32 : MATRIX_DATATYPE_FLOAT32;
  state->output_tsseq = npx_output_tsseq_alloc(timesteps, state->input_tsseq->is_boundary, state->input_tsseq->scaled, datatype, 3, size);

  ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
  ervp_mconv_option_t conv_option;
  conv_option.value = 0;
  conv_option.br.acc = 1;
  conv_option.br.stride_m1 = layer->stride - 1;

  for (int i = 0; i < timesteps; i++)
  {
    NpxTensorInfo *input_tensor3d = state->input_tsseq->sequence[i];
    NpxTensorInfo *output_tensor3d = state->output_tsseq->sequence[i];
    assert(input_tensor3d);
    assert(output_tensor3d);

    ErvpMatrixInfo **input_matrix_info_list = npx_tensor_to_matrix_info_list(input_tensor3d, 1, layer->iodata.in_channels);
    ErvpMatrixInfo *output_matrix = NULL;
    for (int j = 0; j < layer->iodata.out_channels; j++)
    {
      output_matrix = npx_tensor_to_iterative_matrix_info(output_tensor3d, 1, output_matrix);
      hwtask_busy_fx = mop_mapping->matrix_conv_sharedoutput(mop_mapping, layer->iodata.in_channels, input_matrix_info_list, &(layer->weight_matrix_info_list_for_output_reuse[j * layer->iodata.in_channels]), output_matrix, conv_option.value, 1);
    }
    hwtask_wait_complete(hwtask_busy_fx);
    hwtask_busy_fx = NULL;
    matrix_list_free(input_matrix_info_list, layer->iodata.in_channels);
    matrix_free(output_matrix);
  }
}

static void _forward_conv2d_layer_matrix_sharedinput(npx_conv2d_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state)
{
  printf_function();
  assert(mop_mapping->matrix_conv_sharedinput);

  const int timesteps = state->input_tsseq->timesteps;
  assert(timesteps > 0);
  assert(layer->iodata.out_channels > 0);

  int size[3];
  size[0] = layer->iodata.out_size[0];
  size[1] = layer->iodata.out_size[1];
  size[2] = layer->iodata.out_channels;
  int datatype = layer->iodata.out_is_quantized ? MATRIX_DATATYPE_SINT32 : MATRIX_DATATYPE_FLOAT32;
  state->output_tsseq = npx_output_tsseq_alloc(timesteps, state->input_tsseq->is_boundary, state->input_tsseq->scaled, datatype, 3, size);

  ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
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

    ErvpMatrixInfo **output_matrix_info_list = npx_tensor_to_matrix_info_list(output_tensor3d, 1, layer->iodata.out_channels);
    for (int j = 0; j < layer->iodata.out_channels; j++)
      hwtask_busy_fx = mop_mapping->matrix_zero(mop_mapping, output_matrix_info_list[j]);

    ErvpMatrixInfo *input_matrix = NULL;
    for (int k = 0; k < layer->iodata.in_channels; k++)
    {
      input_matrix = npx_tensor_to_iterative_matrix_info(input_tensor3d, 1, input_matrix);
      hwtask_wait_complete(hwtask_busy_fx);
      hwtask_busy_fx = mop_mapping->matrix_conv_sharedinput(mop_mapping, layer->iodata.out_channels, input_matrix, &(layer->weight_matrix_info_list_for_input_reuse[k * layer->iodata.out_channels]), output_matrix_info_list, conv_option.value);
    }
    hwtask_wait_complete(hwtask_busy_fx);
    hwtask_busy_fx = NULL;
    matrix_list_free(output_matrix_info_list, layer->iodata.out_channels);
    matrix_free(input_matrix);
  }
}

static void _check_conv2d_layer(npx_conv2d_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state)
{
  assert_pointer(2, layer, state);
  assert(state->output_tsseq == NULL);

  const NpxTensorInfo *const input_tensor = state->input_tsseq->sequence[0];
  assert(layer->iodata.in_size[0] == input_tensor->size[0]);
  assert(layer->iodata.in_size[1] == input_tensor->size[1]);
  assert(layer->iodata.out_size[0] == (((input_tensor->size[0] + 2 * layer->pad_options.br.num_rowd - layer->kernel_size) / layer->stride) + 1));
  assert(layer->iodata.out_size[1] == (((input_tensor->size[1] + 2 * layer->pad_options.br.num_rowd - layer->kernel_size) / layer->stride) + 1));
}

__attribute__((weak)) void npx_forward_conv2d_layer_reuse(npx_conv2d_layer_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state)
{
  assert(mop_mapping);
  _check_conv2d_layer(layer, mop_mapping, state);
  // NOT supported
  if (layer->pad_options.br.num_rowd > 0)
    assert(layer->pad_options.br.mode == PADMODE_ZEROS);

  int use_multi_input = 0;
  if (layer->pad_options.br.num_rowd != 0)
    use_multi_input = 0;
  else if (layer->stride != 1)
    use_multi_input = 0;
  else if ((layer->iodata.in_channels == 1) && (layer->iodata.out_channels == 1))
    use_multi_input = 1;
  else if (layer->iodata.out_channels == 1)
    use_multi_input = 1;
  else if (layer->iodata.in_channels == 1)
    use_multi_input = 0;
  else if (mop_mapping->matrix_conv_sharedoutput != matrix_conv_sharedoutput_tf)
    use_multi_input = 1;
  else
    use_multi_input = 0;

  NPX_PROFILING_START();
  if (use_multi_input)
  {
    assert(mop_mapping->matrix_conv_sharedoutput);
    _forward_conv2d_layer_matrix_sharedoutput(layer, mop_mapping, state);
  }
  else
  {
    assert(mop_mapping->matrix_conv_sharedinput);
    _forward_conv2d_layer_matrix_sharedinput(layer, mop_mapping, state);
  }
  NPX_PROFILING_END();
}