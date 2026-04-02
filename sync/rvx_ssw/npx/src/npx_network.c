#include "ervp_printf.h"
#include "ervp_printf_section.h"
#include "ervp_malloc.h"
#include "ervp_memory_util.h"
#include "ervp_fakefile.h"
#include "ervp_fakefile_system.h"
#include "ervp_matrix.h"
#include "ervp_matrix_element.h"
#include "ervp_special_matrix_op.h"
#include "ervp_assert.h"

#include <string.h>
#include <stdint.h>
#include "npx_network.h"
#include "npx_layer.h"
#include "npx_parser.h"
#include "npx_util.h"
#include "npx_tensor.h"
#include "npx_sample.h"
#include "npx_preprocess.h"
#include "npx_profiling.h"

static int read_data_aligned_by_4bytes(void *ptr, size_t size, size_t count, FAKEFILE *fp);
static void load_conv2d_weights(npx_conv2d_layer_t *layer, FAKEFILE *fp);
static void load_linear_weights(npx_linear_layer_t *layer, FAKEFILE *fp);
static void load_leaky_threshold(npx_leaky_layer_t *layer, FAKEFILE *fp);
static void load_leaky_beta(npx_leaky_layer_t *layer, FAKEFILE *fp);

static int read_data_aligned_by_4bytes(void *ptr, size_t size, size_t count, FAKEFILE *fp)
{
  int bytes = size * count;
  size_t residue_bytes = (4 - (bytes % 4)) % 4;
  int8_t residue[3];
  ffread(ptr, size, count, fp);
  if (residue_bytes > 0)
    ffread(residue, sizeof(int8_t), residue_bytes, fp);

  return bytes + residue_bytes; // read bytes
}

void npx_network_load_parameters(const npx_network_t *net, const char *filename)
{
  assert(net);
  FAKEFILE *fp;
  fp = ffopen(filename, "r");
  printf("\nparam size: %d\n", fakefile_get_size(fp));

  int i;
  for (i = 0; i < net->num_layer; i++)
  {
    npx_layer_type_t layer_type = net->layer_compute_seq[i]->layer_type;
    // printf("load :%d\n", layer->type );
    if (layer_type == NPXL_CONV2D)
    {
      load_conv2d_weights(net->layer_compute_seq[i]->layer, fp);
    }
    else if (layer_type == NPXL_LINEAR)
    {
      load_linear_weights(net->layer_compute_seq[i]->layer, fp);
    }
    else if (layer_type == NPXL_LEAKY)
    {
      load_leaky_threshold(net->layer_compute_seq[i]->layer, fp);
      load_leaky_beta(net->layer_compute_seq[i]->layer, fp);
    }
    else if ((layer_type == NPXL_MAXPOOL2D) || (layer_type == NPXL_AVGPOOL2D) || (layer_type == NPXL_FLATTEN))
      ;
    else
      assert(0);
    // printf("load end :%d\n", layer->type );
  }
  // printf("remained data size %d\n", fp->current->current_size - fp->read_index);

  ffclose(fp);
}

static void load_conv2d_weights(npx_conv2d_layer_t *layer, FAKEFILE *fp)
{
  size_t num = npx_tensor_elements(layer->weight_tensor);
  read_data_aligned_by_4bytes(layer->weight_tensor->addr, matrix_datatype_size(layer->weight_tensor->datatype), num, fp);
#if 0
  matrix_print(layer->weight_matrix[0][0]);
  matrix_print(layer->weight_matrix[layer->iodata.out_channels-1][layer->iodata.in_channels-1]);
#endif
}

static void load_linear_weights(npx_linear_layer_t *layer, FAKEFILE *fp)
{
  size_t num = npx_tensor_elements(layer->weight_tensor);
  read_data_aligned_by_4bytes(layer->weight_tensor->addr, matrix_datatype_size(layer->weight_tensor->datatype), num, fp);
#if 0
  matrix_print(layer->weight_matrix);
#endif
}

static void load_leaky_threshold(npx_leaky_layer_t *layer, FAKEFILE *fp)
{
  read_data_aligned_by_4bytes(&(layer->threshold), sizeof(npx_threshold_t), 1, fp);
#if 0
  debug_printd(layer->threshold);
#endif
}

static void load_leaky_beta(npx_leaky_layer_t *layer, FAKEFILE *fp)
{
  read_data_aligned_by_4bytes(&(layer->beta), sizeof(npx_beta_t), 1, fp);
#if 0
  debug_printx(layer->beta);
  debug_printf(layer->beta);
#endif
  layer->beta_denominator_rsa = 8;
  layer->beta_denominator = pow(2, layer->beta_denominator_rsa);
  layer->beta_numerator = layer->beta * layer->beta_denominator;
  // assert(layer->beta == (((float)(layer->beta_numerator) / layer->beta_denominator)));
}

void npx_layerio_tsseq_print(npx_layerio_tsseq_t *tsseq)
{
  assert(tsseq);
  for (int i = 0; i < tsseq->timesteps; i++)
  {
    npx_tensor_print(tsseq->sequence[i], -1);
  }
}

static const char *_npx_network_convert_to_pattern(npx_network_t *net)
{
  char *net_type_seq = malloc(sizeof(char) * (net->num_layer + 1));
  net_type_seq[net->num_layer] = 0;
  for (int i = 0; i < net->num_layer; i++)
    net_type_seq[i] = npx_layer_type_to_char(net->layer_compute_seq[i]->layer_type);
  return net_type_seq;
}

void npx_network_print(const npx_network_t *net)
{
  assert(net);
  const char *net_type_seq = _npx_network_convert_to_pattern(net);
  printf("\n[structure] %s", net_type_seq);
  for (int i = 0; i < net->num_layer; i++)
  {
    assert(net->layer_compute_seq[i]);
    printf("\nlayer %02d: %s --> %s", i, npx_layer_type_to_str(net->layer_compute_seq[i]->layer_type), net->layer_compute_seq[i]->operator);
    if (net->layer_compute_seq[i]->layer_type == NPXL_BLOCK)
    {
      npx_layer_block_t *layer_block = (npx_layer_block_t *)(net->layer_compute_seq[i]->layer);
      for (int j = 0; j < layer_block->num_layer; j++)
        printf("\n          |-- %s", npx_layer_type_to_str(layer_block->layer_compute_seq[j]->layer_type));
    }
  }
  free(net_type_seq);
}

// state.output_tsseq is newly allocated but is not free
__attribute__((weak))
npx_layerio_tsseq_t *npx_foward_layers(npx_layer_compute_t **layer_compute_seq, const npx_layerio_tsseq_t *input_tsseq, int layer_start_index, int layer_end_index)
{
  npx_layerio_state_t state;
  state.input_tsseq = input_tsseq;
  state.output_tsseq = NULL;

  for (int i = layer_start_index; i < layer_end_index; i++)
  {
    npx_layer_compute_t *layer_compute = layer_compute_seq[i];
    assert_pointer(1, layer_compute);
    assert_pointer(1, layer_compute->forward);
    layer_compute->forward(layer_compute->layer, layer_compute->mop_mapping, &state);
    assert(state.output_tsseq != NULL);
    assert(state.output_tsseq->scaled > 0.0f);
    if (i > layer_start_index)
      npx_layerio_tsseq_free(state.input_tsseq);
    state.input_tsseq = state.output_tsseq;
    state.output_tsseq = NULL;
  }
  return state.input_tsseq;
}

__attribute__((weak))
npx_layerio_tsseq_t *npx_inference(npx_network_t *net, const npx_layerio_tsseq_t *input_tsseq, int layer_start_index, int layer_end_index)
{
  npx_layerio_tsseq_t *result;
  NPX_PROFILING_START();
  assert_pointer(2, net, input_tsseq);
  const int SKIP_SIM = 1;
  if ((layer_end_index - layer_start_index) == 1)
  {
    printf_subsubsection(SKIP_SIM, "Layer %d : %s", layer_start_index, npx_layer_type_to_str(net->layer_compute_seq[layer_start_index]->layer_type));
  }
  else
  {
    printf_subsubsection(SKIP_SIM, "Layer %d ~ %d", layer_start_index, layer_end_index - 1);
  }
  result = npx_foward_layers(net->layer_compute_seq, input_tsseq, layer_start_index, layer_end_index);
  NPX_PROFILING_END();
  return result;
}

/*
int npx_get_testvector_index(npx_network_t *net, int layer_index)
{
  int tv_index = 0;
  for (int i = 0; i < layer_index; i++)
  {
    npx_layer_compute_t *layer_compute = net->layer_compute_seq[i];
    if (layer_compute->layer_type == NPXL_BLOCK)
    {
      npx_layer_block_t *layer_block = (npx_layer_block_t *)(layer_compute->layer);
      tv_index += layer_block->num_layer;
    }
    else
      tv_index++;
  }
  return tv_index;
}
*/

npx_testvector_t *npx_load_testvector(const char *tv_fname, npx_network_t *net)
{
  npx_testvector_t *result = malloc(sizeof(npx_testvector_t));
  FAKEFILE *fp = ffopen(tv_fname, "r");
  assert(fp->head);
  assert(fp->head->next == NULL);
  result->tv_size = fp->head->size;
  result->tv_data = (int8_t *)fp->head->data;
  ffclose(fp);
  printf("\ntestvector: %s (%d)", tv_fname, result->tv_size);

  result->num_layer = net->num_layer;
  result->layer_output_size = malloc(sizeof(int) * result->num_layer);
  result->testvector_acc_size = malloc(sizeof(int) * result->num_layer);
  assert(result->layer_output_size);
  assert(result->testvector_acc_size);
  int testvector_acc_size = 0;
  for (int i = 0; i < result->num_layer; i++)
  {
    npx_layer_compute_t *layer_compute = net->layer_compute_seq[i];
    result->layer_output_size[i] = npx_layer_output_size(layer_compute->layer_type, layer_compute->layer);
    testvector_acc_size += npx_layer_testvector_size(layer_compute->layer_type, layer_compute->layer);
    result->testvector_acc_size[i] = testvector_acc_size;
  }
  assert(result->tv_size % (testvector_acc_size * 4) == 0);
  return result;
}

static void _npx_layer_compute_seq_reset(npx_layer_compute_t **layer_compute_seq, int num_layer)
{
  for (int i = 0; i < num_layer; i++)
  {
    npx_layer_compute_t *layer_compute = layer_compute_seq[i];
    if (layer_compute->layer_type == NPXL_LEAKY)
    {
      npx_leaky_layer_t *leaky_layer = (npx_leaky_layer_t *)(layer_compute->layer);
      int num_channel = leaky_layer->iodata.in_channels;
      for (int j = 0; j < num_channel; j++)
        matrix_zero_sw(leaky_layer->membrane_potential[j]);
    }
    else if (layer_compute->layer_type == NPXL_BLOCK)
    {
      npx_layer_block_t *layer_block = (npx_layer_block_t *)(layer_compute->layer);
      _npx_layer_compute_seq_reset(layer_block->layer_compute_seq, layer_block->num_layer);
    }
  }
}

void npx_network_reset(npx_network_t *net)
{
  _npx_layer_compute_seq_reset(net->layer_compute_seq, net->num_layer);
}

void npx_network_map_matrix_operator(npx_network_t *net, int layer_index, ervp_mop_mapping_t *mop_mapping)
{
  int start_index;
  int end_index;
  if (layer_index == (-1))
  {
    start_index = 0;
    end_index = net->num_layer;
  }
  else
  {
    start_index = layer_index;
    end_index = layer_index + 1;
  }
  for (int i = start_index; i < end_index; i++)
    net->layer_compute_seq[i]->mop_mapping = mop_mapping;
}

static ErvpMatrixInfo *generate_output_matrix_info(NpxTensorInfo *output_tensor, int channel_index, ErvpMatrixInfo *preallocated)
{
  ErvpMatrixInfo *output_matrix;
  if (channel_index == 0)
  {
    assert(preallocated == NULL);
    output_matrix = matrix_generate_info(output_tensor->datatype, output_tensor->size[1], output_tensor->size[0], output_tensor->addr, NULL);
  }
  else
  {
    assert(preallocated != NULL);
    output_matrix = preallocated;
    uint8_t *addr = (uint8_t *)(output_tensor->addr);
    addr += channel_index * matrix_num_bytes(output_matrix);
    output_matrix->addr = addr;
  }
  return output_matrix;
}

static ErvpMatrixInfo *generate_tv_matrix_info(int size[2], npx_testvector_t *tv, int timesteps, int layer_index, int channel_index, ErvpMatrixInfo *preallocated)
{
  ErvpMatrixInfo *ref_matrix;
  if (channel_index == 0)
  {
    assert(preallocated == NULL);
    ref_matrix = matrix_generate_info(MATRIX_DATATYPE_SINT32, size[1], size[0], NULL, NULL);
  }
  else
  {
    assert(preallocated != NULL);
    ref_matrix = preallocated;
  }

  uint32_t *addr = (uint32_t *)(tv->tv_data);
  int index = 0;
  index += timesteps * tv->testvector_acc_size[tv->num_layer - 1];
  index += (tv->testvector_acc_size[layer_index] - tv->layer_output_size[layer_index]);
  if (channel_index > 0)
    index += channel_index * size[1] * size[0];
  ref_matrix->addr = &(addr[index]);

  return ref_matrix;
}

static const int SKIP_SIM = 1;
static const int PRINT_VERIFY_ALL_RESULT = 0;

void npx_verify_with_testvector(npx_layerio_tsseq_t *output_tsseq, npx_testvector_t *tv, int layer_index)
{
  assert_pointer(2, output_tsseq, tv);
  assert(output_tsseq->timesteps > 0);
  printf_section(SKIP_SIM, "%s %d %d", __func__, layer_index, npx_tensor_elements(output_tsseq->sequence[0]) * output_tsseq->timesteps);
  int t = -1;
  for (int i = 0; i < output_tsseq->timesteps; i++)
  {
    if (output_tsseq->is_boundary[i])
      t++;
    else
      continue;

    NpxTensorInfo *output_tensor = output_tsseq->sequence[i];
    ErvpMatrixInfo *output_matrix = NULL;
    ErvpMatrixInfo *ref_matrix = NULL;
    assert(output_tensor->num_dim <= 3);
    int out_channels = (output_tensor->num_dim == 3) ? output_tensor->size[2] : 1;
    for (int j = 0; j < out_channels; j++)
    {
      output_matrix = generate_output_matrix_info(output_tensor, j, output_matrix);
      ref_matrix = generate_tv_matrix_info(output_tensor->size, tv, t, layer_index, j, ref_matrix);
      int all_correct = matrix_compare_one_by_one(output_matrix, ref_matrix, 0.01, 1);

      if ((!all_correct) || PRINT_VERIFY_ALL_RESULT)
      {
        printf_subsection(SKIP_SIM, "timestep, out_channels: %d, %d", t, j);
        printf_subsubsection(SKIP_SIM, "ref_matrix");
        matrix_print(ref_matrix);
        if (!all_correct)
        {
          printf_subsubsection(SKIP_SIM, "output_matrix");
          matrix_print(output_matrix);
        }
      }
      assert_must(all_correct);
    }
    free(output_matrix);
    free(ref_matrix);
  }
}

int npx_classify(npx_network_t *net, const npx_layerio_tsseq_t *output_tsseq, int min_acc_value, const npx_rawinput_t *sample)
{
  assert(output_tsseq);
  assert(output_tsseq->sequence[0]->num_dim == 2);
  assert(output_tsseq->sequence[0]->size[0] == 1);
  assert(output_tsseq->sequence[0]->size[1] == net->classes);

  int *output_acc = (int *)calloc(net->classes, sizeof(int));
  ErvpMatrixInfo *output_matrix = NULL;
  for (int i = 0; i < output_tsseq->timesteps; i++)
  {
    const NpxTensorInfo *const output_tensor = output_tsseq->sequence[i];
    output_matrix = npx_tensor_to_matrix_info(output_tensor, output_matrix);
    for (int j = 0; j < net->classes; j++)
      output_acc[j] += matrix_read_fixed_element(output_matrix, j, 0);
  }
  int max_index = -1;
  int max_value = -1;
  for (int j = 0; j < net->classes; j++)
  {
    int value = output_acc[j];
    if (value >= min_acc_value)
      if (value > max_value)
      {
        max_index = j;
        max_value = value;
      }
  }
  if (sample)
  {
    int correct = (max_index == sample->label);
    if (correct)
      printf("\nclassification CORRECT: %d (label:%d)", max_index, sample->label);
    else
      printf("\nclassification NOT CORRECT: %d (label:%d)", max_index, sample->label);
  }

  free(output_acc);
  free(output_matrix);

  return max_index;
}

static int find_pattern_index(npx_network_t *net, const char *pattern)
{
  int index;
  const char *net_type_seq = _npx_network_convert_to_pattern(net);
  const char *pos = strstr(net_type_seq, pattern);
  if (pos == NULL)
    index = -1;
  else
    index = (int)(pos - net_type_seq);
  free(net_type_seq);
  return index;
}

static int _npx_network_optimize(npx_network_t *net, const char *pattern, const char *operator, void (*forward)(void *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state))
{
  int index = find_pattern_index(net, pattern);
  if (index >= 0)
  {
    int pattern_size = strlen(pattern);
    assert(pattern_size <= net->num_layer);

    npx_layer_block_t *layer_block = malloc(sizeof(npx_layer_block_t));
    layer_block->num_layer = pattern_size;
    layer_block->layer_compute_seq = npx_layer_compute_seq_alloc(pattern_size);
    for (int i = 0; i < pattern_size; i++)
    {
      layer_block->layer_compute_seq[i] = net->layer_compute_seq[index + i];
      net->layer_compute_seq[index + i] = NULL;
    }
    npx_layer_compute_t *layer_compute = npx_layer_compute_alloc();
    layer_compute->layer_type = NPXL_BLOCK;
    layer_compute->layer = layer_block;
    layer_compute->mop_mapping = NULL;
    layer_compute->forward = forward;
    layer_compute->operator= operator;

    int num_layer_optimized = net->num_layer - pattern_size + 1;
    npx_layer_compute_t **layer_compute_seq_optimized = npx_layer_compute_seq_alloc(num_layer_optimized);

    int exclude_start = index;
    int exclude_end = index + pattern_size;

    int write_idx = 0;
    for (int i = 0; i < net->num_layer; ++i)
    {
      if (i == index)
      {
        assert(net->layer_compute_seq[i] == NULL);
        layer_compute_seq_optimized[write_idx++] = layer_compute;
      }
      else if ((i >= exclude_start) && (i < exclude_end))
      {
        assert(net->layer_compute_seq[i] == NULL);
      }
      else
      {
        layer_compute_seq_optimized[write_idx++] = net->layer_compute_seq[i];
      }
    }
    assert((write_idx + pattern_size - 1) == net->num_layer);
    npx_layer_compute_seq_free(net->layer_compute_seq);
    net->layer_compute_seq = layer_compute_seq_optimized;
    net->num_layer = num_layer_optimized;
  }
  return index;
}

__attribute__((weak))
void npx_network_optimize(npx_network_t *net, const char *pattern, int num_change, const char *operator, void (*forward)(void *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state))
{
  if (num_change < 0)
    num_change = net->num_layer;
  while (num_change > 0)
  {
    int index = _npx_network_optimize(net, pattern, operator, forward);
    if (index < 0)
      break;
    num_change--;
  }
}