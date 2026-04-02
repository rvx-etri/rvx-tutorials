#include "ervp_printf.h"
#include "ervp_malloc.h"
#include "ervp_assert.h"
#include "ervp_memory_util.h"
#include "ervp_matrix.h"
#include "ervp_round_int.h"
#include "ervp_stdlib.h"

#include <math.h>
#include <string.h>
// #include <stdlib.h>
#include <stdint.h>
#include "npx_parser.h"
#include "npx_layer.h"
#include "npx_network.h"
#include "npx_tensor.h"

static void parse_global_cfg(texpar_list_t *option_list, npx_network_t *net);
static int is_global(const texpar_section_t *s);
static void parse_conv2d(npx_layer_compute_t *layer_compute, texpar_list_t *option_list, const char *operator);
static void parse_avgpool2d(npx_layer_compute_t *layer_compute, texpar_list_t *option_list, const char *operator);
static void parse_leaky(npx_layer_compute_t *layer_compute, texpar_list_t *option_list, const char *operator);
static void parse_flatten(npx_layer_compute_t *layer_compute, texpar_list_t *option_list, const char *operator);
static void parse_linear(npx_layer_compute_t *layer_compute, texpar_list_t *option_list, const char *operator);
static void parse_maxpool2d(npx_layer_compute_t *layer_compute, texpar_list_t *option_list, const char *operator);

static npx_network_t *npx_network_malloc(int num_layer);

static const int NUM_NON_LAYER = 1;

npx_network_t *npx_parse_network_cfg(const char *net_fname, const char *opt_fname)
{
  texpar_list_t *section_list = texpar_read_file_with_section(net_fname);
  texpar_list_t *opt_section_list = texpar_read_file_with_section(opt_fname);
  npx_network_t *net = npx_network_malloc(section_list->size - NUM_NON_LAYER);

  /* network info parsing */
  assert(section_list->front);
  texpar_section_t *global_section = (texpar_section_t *)section_list->front->val;
  assert(is_global(global_section));
  parse_global_cfg(global_section->option_list, net);
  // texpar_unused(train_option_list);

  /* layers info parsing */
  texpar_iter_t *net_iter = section_list->front;
  texpar_iter_t *opt_iter = opt_section_list->front;
  for (int i = 0; i < NUM_NON_LAYER; i++)
    net_iter = net_iter->next;

  int layer_index = 0;
  // texpar_free_section(net_section);
  while (net_iter)
  {
    const texpar_section_t *net_section = (texpar_section_t *)net_iter->val;
    texpar_list_t *option_list = net_section->option_list;
    const texpar_section_t *opt_section = (texpar_section_t *)opt_iter->val;
    assert(strcmp(net_section->type, opt_section->type) == 0);

    char *operator = texpar_find_str(opt_section->option_list, "operator", "cpu");
    // printf("\nlayer %02d: %s --> %s", layer_index, net_section->type, operator);

    if (strcmp(net_section->type, "[Conv2d]") == 0)
    {
      parse_conv2d(net->layer_compute_seq[layer_index], option_list, operator);
    }
    else if (strcmp(net_section->type, "[Linear]") == 0)
    {
      parse_linear(net->layer_compute_seq[layer_index], option_list, operator);
    }
    else if (strcmp(net_section->type, "[Leaky]") == 0)
    {
      parse_leaky(net->layer_compute_seq[layer_index], option_list, operator);
    }
    else if (strcmp(net_section->type, "[MaxPool2d]") == 0)
    {
      parse_maxpool2d(net->layer_compute_seq[layer_index], option_list, operator);
    }
    else if (strcmp(net_section->type, "[AvgPool2d]") == 0)
    {
      parse_avgpool2d(net->layer_compute_seq[layer_index], option_list, operator);
    }
    else if (strcmp(net_section->type, "[Flatten]") == 0)
    {
      parse_flatten(net->layer_compute_seq[layer_index], option_list, operator);
    }
    else
      assert(0);

    // texpar_free_section(net_section);
    // texpar_free_section(opt_section);
    net_iter = net_iter->next;
    opt_iter = opt_iter->next;
    layer_index++;
  }
  // free_list(section_list);
  // free_list(opt_section_list);

  printf("\nnumber of layers: %d\n", net->num_layer);

  return net;
}

static npx_network_t *npx_network_malloc(int num_layer)
{
  npx_network_t *net = (npx_network_t *)calloc(1, sizeof(npx_network_t));
  net->num_layer = num_layer;
  net->layer_compute_seq = npx_layer_compute_seq_alloc(num_layer);
  for (int i = 0; i < net->num_layer; i++)
    net->layer_compute_seq[i] = npx_layer_compute_alloc();
  return net;
}

static void set_array_from_str(char *size_str, int *size_array, int num)
{
  char *p;
  if (size_str[0] == '(')
    size_str++;
  int len = strlen(size_str);
  if (size_str[len - 1] == ')')
    size_str[len - 1] = 0;
  p = strtok(size_str, ",");
  for (int i = 0; i < num; i++)
  {
    assert(p);
    size_array[num - 1 - i] = atoi(p);
    p = strtok(NULL, ",");
  }
}

static void parse_global_cfg(texpar_list_t *option_list, npx_network_t *net)
{
  char *input_size_str = texpar_find_str(option_list, "input_size", "28,28");
  int input_channels = texpar_find_int_quiet(option_list, "input_channels", 1);
  int classes = texpar_find_int_quiet(option_list, "output_classes", 1);

  int size[2];
  set_array_from_str(input_size_str, size, 2);
  net->h = size[0];
  net->w = size[1];
  net->c = input_channels;
  net->classes = classes;

  printf("\nh,w,c : %d,%d,%d", net->h, net->w, net->c);
  printf("\noutput_classes: %d", net->classes);
}

static int is_global(const texpar_section_t *section)
{
  return (strcmp(section->type, "[global]") == 0);
}

static void parse_iodata2d(npx_layer2d_iodata_t *iodata, texpar_list_t *option_list)
{
  iodata->in_channels = texpar_find_int_quiet(option_list, "in_channels", 0);
  iodata->out_channels = texpar_find_int_quiet(option_list, "out_channels", 0);
  char *input_size_str = texpar_find_str(option_list, "in_size", "");
  char *output_size_str = texpar_find_str(option_list, "out_size", "");
  set_array_from_str(input_size_str, iodata->in_size, 2);
  set_array_from_str(output_size_str, iodata->out_size, 2);
  iodata->in_is_quantized = texpar_find_int_quiet(option_list, "in_is_quantized", -1);
  iodata->out_is_quantized = texpar_find_int_quiet(option_list, "out_is_quantized", -1);
  iodata->in_is_binary = texpar_find_int_quiet(option_list, "in_is_binary", -1);
  iodata->out_is_binary = texpar_find_int_quiet(option_list, "out_is_binary", -1);

  assert(iodata->in_channels > 0);
  assert(iodata->out_channels > 0);
  assert(iodata->in_size[0] > 0);
  assert(iodata->in_size[1] > 0);
  assert(iodata->out_size[0] > 0);
  assert(iodata->out_size[1] > 0);
  assert(iodata->in_is_quantized >= 0);
  assert(iodata->out_is_quantized >= 0);
}

static int get_weight_datatype(texpar_list_t *option_list)
{
  int weight_bitwidth = texpar_find_int_quiet(option_list, "weight_bitwidth", 0);
  assert(weight_bitwidth > 0);
  weight_bitwidth = round_up_power2(weight_bitwidth);
  int weight_datatype = MATRIX_DATATYPE_SINT08;
  if (weight_bitwidth == 8)
    weight_datatype = MATRIX_DATATYPE_SINT08;
  else if (weight_bitwidth == 16)
    weight_datatype = MATRIX_DATATYPE_SINT16;
  else if (weight_bitwidth == 32)
    weight_datatype = MATRIX_DATATYPE_SINT32;
  else
    assert(0);
  return weight_datatype;
}

static void transpose_matrix_info(ErvpMatrixInfo **src_info_list, ErvpMatrixInfo **dst_info_list, int row, int col)
{
  for (int i = 0; i < row; i++)
    for (int j = 0; j < col; j++)
      dst_info_list[j * row + i] = src_info_list[i * col + j];
}

static void parse_conv2d(npx_layer_compute_t *layer_compute, texpar_list_t *option_list, const char *operator)
{
  npx_conv2d_layer_t *conv2d_layer = (npx_conv2d_layer_t *)calloc(1, sizeof(npx_conv2d_layer_t));
  layer_compute->layer_type = NPXL_CONV2D;
  layer_compute->layer = conv2d_layer;
  conv2d_layer->kernel_size = texpar_find_int_quiet(option_list, "kernel_size", 0);
  conv2d_layer->stride = texpar_find_int_quiet(option_list, "stride", 1);

  int pad_amount = texpar_find_int_quiet(option_list, "padding", 0);
  conv2d_layer->pad_options.value = 0;
  conv2d_layer->pad_options.br.is_regular = 1;
  conv2d_layer->pad_options.br.num_rowu = pad_amount;
  conv2d_layer->pad_options.br.num_rowd = pad_amount;
  conv2d_layer->pad_options.br.num_colu = pad_amount;
  conv2d_layer->pad_options.br.num_cold = pad_amount;
  if (pad_amount == 0)
    conv2d_layer->pad_options.br.mode = PADMODE_NONE;
  else
    conv2d_layer->pad_options.br.mode = PADMODE_ZEROS;

  assert(conv2d_layer->kernel_size > 0);
  assert(conv2d_layer->stride > 0);
  assert(conv2d_layer->stride < 16);

  parse_iodata2d(&(conv2d_layer->iodata), option_list);

  // weight
  conv2d_layer->weight_tensor = npx_tensor_alloc_wo_data(4);
  conv2d_layer->weight_tensor->size[0] = conv2d_layer->kernel_size;
  conv2d_layer->weight_tensor->size[1] = conv2d_layer->kernel_size;
  conv2d_layer->weight_tensor->size[2] = conv2d_layer->iodata.in_channels;
  conv2d_layer->weight_tensor->size[3] = conv2d_layer->iodata.out_channels;
  npx_tensor_set_datatype(conv2d_layer->weight_tensor, get_weight_datatype(option_list));
  npx_tensor_alloc_data(conv2d_layer->weight_tensor);

  conv2d_layer->weight_matrix_info_list_for_output_reuse = npx_tensor_to_matrix_info_list(conv2d_layer->weight_tensor, 1, conv2d_layer->iodata.out_channels * conv2d_layer->iodata.in_channels);
  conv2d_layer->weight_matrix_info_list_for_input_reuse = npx_tensor_to_matrix_info_list(conv2d_layer->weight_tensor, 1, conv2d_layer->iodata.out_channels * conv2d_layer->iodata.in_channels);
  transpose_matrix_info(conv2d_layer->weight_matrix_info_list_for_output_reuse, conv2d_layer->weight_matrix_info_list_for_input_reuse, conv2d_layer->iodata.out_channels, conv2d_layer->iodata.in_channels);

  // operator
  layer_compute->operator = operator;
  if (strcmp(layer_compute->operator, "dca") == 0)
    layer_compute->forward = npx_forward_conv2d_layer_reuse;
  else
    layer_compute->forward = npx_forward_conv2d_layer_default;
}

static void parse_avgpool2d(npx_layer_compute_t *layer_compute, texpar_list_t *option_list, const char *operator)
{
  npx_avgpool2d_layer_t *avgpool2d_layer = (npx_avgpool2d_layer_t *)calloc(1, sizeof(npx_avgpool2d_layer_t));
  layer_compute->layer_type = NPXL_AVGPOOL2D;
  layer_compute->layer = avgpool2d_layer;
  avgpool2d_layer->kernel_size = texpar_find_int_quiet(option_list, "kernel_size", 0);
  avgpool2d_layer->stride = texpar_find_int_quiet(option_list, "stride", 1);

  const int pad_amount = texpar_find_int_quiet(option_list, "padding", 0);
  avgpool2d_layer->pad_options.value = 0;
  avgpool2d_layer->pad_options.br.is_regular = 1;
  avgpool2d_layer->pad_options.br.num_rowu = pad_amount;
  avgpool2d_layer->pad_options.br.num_rowd = pad_amount;
  avgpool2d_layer->pad_options.br.num_colu = pad_amount;
  avgpool2d_layer->pad_options.br.num_cold = pad_amount;
  if (pad_amount == 0)
    avgpool2d_layer->pad_options.br.mode = PADMODE_NONE;
  else
    avgpool2d_layer->pad_options.br.mode = PADMODE_ZEROS;

  assert(avgpool2d_layer->kernel_size > 0);
  assert(avgpool2d_layer->stride > 0);
  assert(avgpool2d_layer->stride < 16);

  parse_iodata2d(&(avgpool2d_layer->iodata), option_list);

  layer_compute->operator = operator;
  layer_compute->forward = npx_forward_avgpool2d_layer_default;
}

static void parse_leaky(npx_layer_compute_t *layer_compute, texpar_list_t *option_list, const char *operator)
{
  npx_leaky_layer_t *leaky_layer = (npx_leaky_layer_t *)calloc(1, sizeof(npx_leaky_layer_t));
  layer_compute->layer_type = NPXL_LEAKY;
  layer_compute->layer = leaky_layer;
  // leaky_layer->beta = texpar_find_float_quiet(option_list, "beta", 0.0f);
  leaky_layer->reset_mechanism = texpar_find_str(option_list, "reset_mechanism", "");
  leaky_layer->reset_delay = texpar_find_str(option_list, "reset_delay", "");

  //
  leaky_layer->is_hard_reset = (strcmp(leaky_layer->reset_mechanism, "zero") == 0);
  leaky_layer->is_soft_reset = (strcmp(leaky_layer->reset_mechanism, "subtract") == 0);
  assert(leaky_layer->is_hard_reset || leaky_layer->is_soft_reset);
  leaky_layer->has_reset_delay = (strcmp(leaky_layer->reset_delay, "True") == 0);
  leaky_layer->does_decay = (leaky_layer->beta != 1.0f);

  // leaky_layer->beta_denominator_rsa = 8;
  // leaky_layer->beta_denominator = pow(2, leaky_layer->beta_denominator_rsa);
  // leaky_layer->beta_numerator = leaky_layer->beta * leaky_layer->beta_denominator;
  // assert(leaky_layer->beta == (((float)(leaky_layer->beta_numerator) / leaky_layer->beta_denominator)));
  leaky_layer->beta_denominator_rsa = 0;
  leaky_layer->beta_denominator = 0;
  leaky_layer->beta_numerator = 0;

  parse_iodata2d(&(leaky_layer->iodata), option_list);

  int num_channel = leaky_layer->iodata.in_channels;
  int datatype = leaky_layer->iodata.in_is_quantized ? MATRIX_DATATYPE_SINT32 : MATRIX_DATATYPE_FLOAT32;
  leaky_layer->membrane_potential = (ErvpMatrixInfo **)calloc(num_channel, sizeof(ErvpMatrixInfo *));
#if 1
  leaky_layer->membrane_potential_total = matrix_alloc(datatype, num_channel * leaky_layer->iodata.in_size[1], leaky_layer->iodata.in_size[0], NULL);
  for (int i = 0; i < num_channel; i++)
  {
    leaky_layer->membrane_potential[i] = matrix_generate_submatrix_info(leaky_layer->membrane_potential_total, NULL);
    leaky_layer->membrane_potential[i]->addr = matrix_get_row_addr(leaky_layer->membrane_potential_total, i * leaky_layer->iodata.in_size[1]);
    leaky_layer->membrane_potential[i]->num_row = leaky_layer->iodata.in_size[1];
  }
#else
  for (int i = 0; i < num_channel; i++)
    leaky_layer->membrane_potential[i] = matrix_alloc(datatype, leaky_layer->iodata.in_size[1], leaky_layer->iodata.in_size[0], NULL);
#endif
  if (leaky_layer->beta_denominator == leaky_layer->beta_numerator)
    leaky_layer->membrane_potential_scaled = 1;
  else
    leaky_layer->membrane_potential_scaled = 4; // increase to ensure reserved precision

  layer_compute->operator = operator;
  layer_compute->forward = npx_forward_leaky_layer_default;
}

static void parse_flatten(npx_layer_compute_t *layer_compute, texpar_list_t *option_list, const char *operator)
{
  npx_flatten_layer_t *flatten_layer = (npx_flatten_layer_t *)calloc(1, sizeof(npx_flatten_layer_t));
  layer_compute->layer_type = NPXL_FLATTEN;
  layer_compute->layer = flatten_layer;

  parse_iodata2d(&(flatten_layer->iodata), option_list);

  layer_compute->operator = operator;
  layer_compute->forward = npx_forward_flatten_layer_default;
}

static void parse_linear(npx_layer_compute_t *layer_compute, texpar_list_t *option_list, const char *operator)
{
  npx_linear_layer_t *linear_layer = (npx_linear_layer_t *)calloc(1, sizeof(npx_linear_layer_t));
  layer_compute->layer_type = NPXL_LINEAR;
  layer_compute->layer = linear_layer;
  linear_layer->in_features = texpar_find_int_quiet(option_list, "in_features", 0);
  linear_layer->out_features = texpar_find_int_quiet(option_list, "out_features", 0);

  assert(linear_layer->in_features >= 0);
  assert(linear_layer->out_features >= 0);

  parse_iodata2d(&(linear_layer->iodata), option_list);

  // weight
  linear_layer->weight_tensor = npx_tensor_alloc_wo_data(2);
  linear_layer->weight_tensor->size[0] = linear_layer->in_features;
  linear_layer->weight_tensor->size[1] = linear_layer->out_features;
  npx_tensor_set_datatype(linear_layer->weight_tensor, get_weight_datatype(option_list));
  npx_tensor_alloc_data(linear_layer->weight_tensor);

  layer_compute->operator = operator;
  layer_compute->forward = npx_forward_linear_layer_default;
}

static void parse_maxpool2d(npx_layer_compute_t *layer_compute, texpar_list_t *option_list, const char *operator)
{
  npx_maxpool2d_layer_t *maxpool2d_layer = (npx_maxpool2d_layer_t *)calloc(1, sizeof(npx_maxpool2d_layer_t));
  layer_compute->layer_type = NPXL_MAXPOOL2D;
  layer_compute->layer = maxpool2d_layer;
  maxpool2d_layer->kernel_size = texpar_find_int_quiet(option_list, "kernel_size", 0);
  maxpool2d_layer->stride = texpar_find_int_quiet(option_list, "stride", 1);

  int pad_amount = texpar_find_int_quiet(option_list, "padding", 0);
  maxpool2d_layer->pad_options.value = 0;
  maxpool2d_layer->pad_options.br.is_regular = 1;
  maxpool2d_layer->pad_options.br.num_rowu = pad_amount;
  maxpool2d_layer->pad_options.br.num_rowd = pad_amount;
  maxpool2d_layer->pad_options.br.num_colu = pad_amount;
  maxpool2d_layer->pad_options.br.num_cold = pad_amount;
  if (pad_amount == 0)
    maxpool2d_layer->pad_options.br.mode = PADMODE_NONE;
  else
    maxpool2d_layer->pad_options.br.mode = PADMODE_ZEROS;

  assert(maxpool2d_layer->kernel_size > 0);
  assert(maxpool2d_layer->stride > 0);
  assert(maxpool2d_layer->stride < 16);

  parse_iodata2d(&(maxpool2d_layer->iodata), option_list);

  layer_compute->operator = operator;
  layer_compute->forward = npx_forward_maxpool2d_layer_default;
}