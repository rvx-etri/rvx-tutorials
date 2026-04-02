#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_malloc.h"
#include "ervp_memory_util.h"
#include "ervp_fakefile.h"

#include "texpar_list.h"
#include "texpar_api.h"

#include "npx_preprocess.h"
#include "npx_util.h"
#include "npx_spike_lfsr.h"
#include "npx_dvs.h"
#include "npx_layer.h"
#include "npx_waveform.h"
#include "npx_profiling.h"

npx_layerio_tsseq_t *npx_mnist_preprocess(const npx_network_t *net, NpxTensorInfo *input, texpar_list_t *option_list);
npx_layerio_tsseq_t *npx_dvsgesture_preprocess(const npx_network_t *net, NpxTensorInfo *input, texpar_list_t *option_list);
npx_layerio_tsseq_t *npx_waveform_preprocess(const npx_network_t *net, NpxTensorInfo *input, texpar_list_t *option_list);
static NpxTensorInfo *generate_single_step_by_lfsr(const NpxTensorInfo *input);
static NpxTensorInfo *generate_single_step(const NpxTensorInfo *input, const char *step_generation);

npx_layerio_tsseq_t *npx_preprocess(const char *pre_fname, const npx_network_t *net, const NpxTensorInfo *input, float input_scale)
{
  NPX_PROFILING_START();
  assert(net);
  assert(input);
  assert(input_scale > 0);

  texpar_list_t *section_list = texpar_read_file_with_section(pre_fname);
  texpar_section_t *s = (texpar_section_t *)section_list->front->val;
  texpar_list_t *option_list = s->option_list;

  assert(strcmp(s->type, "[preprocess]") == 0);
  const char *input_str = texpar_find_str(option_list, "input", NULL);
  assert(input_str);
  printf("\npreprocess: %s", input_str);

  npx_layerio_tsseq_t *input_tsseq = NULL;
  if ((strcmp(input_str, "mnist_opendataset") == 0) || (strcmp(input_str, "fmnist_opendataset") == 0) || (strcmp(input_str, "cifar10_opendataset") == 0) || (strcmp(input_str, "gtsrb_opendataset") == 0) || (strcmp(input_str, "channelresponse_dataset") == 0))
  {
    input_tsseq = npx_mnist_preprocess(net, input, option_list);
  }
  else if (strcmp(input_str, "dvsgesture_opendataset") == 0)
  {
    input_tsseq = npx_dvsgesture_preprocess(net, input, option_list);
  }
  else if (strcmp(input_str, "dvs346") == 0)
  {
    int timesteps = texpar_find_int_quiet(option_list, "timesteps", 0);
    input_tsseq = npx_dvs346_preprocess(net, input, timesteps);
  }
  else if (strcmp(input_str, "speechcommands_opendataset") == 0)
  {
    input_tsseq = npx_waveform_preprocess(net, input, option_list);
  }
  else
    assert_msg(0, "%s", input_str);

  // texpar_unused(option_list);
  // texpar_free_section(s);
  // free_list(section_list);

  input_tsseq->scaled = input_scale;
  NPX_PROFILING_END();

  return input_tsseq;
}

npx_layerio_tsseq_t *npx_mnist_preprocess(const npx_network_t *net, NpxTensorInfo *input, texpar_list_t *option_list)
{
  int timesteps = texpar_find_int_quiet(option_list, "timesteps", 0);
  const char *step_generation = texpar_find_str(option_list, "step_generation", "");

  assert(timesteps > 0);
  assert(strcmp(step_generation, "") != 0);

  NpxTensorInfo *resized = npx_tensor_resize(input, NULL, net->h, net->w);
  npx_layerio_tsseq_t *input_tsseq = npx_layerio_tsseq_alloc(timesteps);
  for (int i = 0; i < timesteps; i++)
    input_tsseq->sequence[i] = generate_single_step(resized, step_generation);

  int *is_boundary = (int *)malloc(timesteps * sizeof(int));
  for (int i = 0; i < timesteps; i++)
    is_boundary[i] = 1;
  input_tsseq->is_boundary = is_boundary;

  return input_tsseq;
}

npx_layerio_tsseq_t *npx_dvsgesture_preprocess(const npx_network_t *net, NpxTensorInfo *input, texpar_list_t *option_list)
{
  int timesteps = texpar_find_int_quiet(option_list, "timesteps", 0);
  assert(timesteps > 0);

  NpxTensorInfo *frame = npx_dvs_to_frame(input, 260, 346, timesteps, NULL);
  NpxTensorInfo *resized = npx_tensor_resize(frame, NULL, net->h, net->w);

  npx_layerio_tsseq_t *input_tsseq = npx_layerio_tsseq_alloc(timesteps);
  for (int t = 0; t < timesteps; t++)
  {
    input_tsseq->sequence[t] = npx_tensor_alloc_wo_data(3);
    input_tsseq->sequence[t]->size[0] = resized->size[0];
    input_tsseq->sequence[t]->size[1] = resized->size[1];
    input_tsseq->sequence[t]->size[2] = resized->size[2];
    npx_tensor_set_datatype(input_tsseq->sequence[t], resized->datatype);
    npx_tensor_set_contiguous_layout(input_tsseq->sequence[t]);
    input_tsseq->sequence[t]->addr = resized->addr + t * resized->stride[3];
  }

  int *is_boundary = (int *)malloc(timesteps * sizeof(int));
  for (int i = 0; i < timesteps; i++)
    is_boundary[i] = 1;
  input_tsseq->is_boundary = is_boundary;

  return input_tsseq;
}

npx_layerio_tsseq_t *npx_waveform_preprocess(const npx_network_t *net, NpxTensorInfo *input, texpar_list_t *option_list)
{
  int timesteps = texpar_find_int_quiet(option_list, "timesteps", 0);
  const char *step_generation = texpar_find_str(option_list, "step_generation", "");
  const char *feature = texpar_find_str(option_list, "feature", "");

  assert(timesteps > 0);
  assert(strcmp(step_generation, "") != 0);

  NpxTensorInfo *preprocessed_input = input;
  if (strcmp(feature, "mel_spectrogram") == 0)
  {
    npx_set_mel_spectorgram(option_list);
    preprocessed_input = npx_log_mel_spectrogram(input, option_list);
  }

  npx_layerio_tsseq_t *input_tsseq = npx_layerio_tsseq_alloc(timesteps);
  for (int i = 0; i < timesteps; i++)
    input_tsseq->sequence[i] = generate_single_step(preprocessed_input, step_generation);

  int *is_boundary = (int *)malloc(timesteps * sizeof(int));
  for (int i = 0; i < timesteps; i++)
    is_boundary[i] = 1;
  input_tsseq->is_boundary = is_boundary;

  return input_tsseq;
}

static NpxTensorInfo *generate_single_step_by_lfsr(const NpxTensorInfo *input)
{
  NpxTensorInfo *output;
  // printf("%s\n", __func__);
  static uint8_t lfsr = 0xff;
  output = npx_gen_spike_lfsr(input, &lfsr);
  return output;
}

static NpxTensorInfo *generate_single_step(const NpxTensorInfo *input, const char *step_generation)
{
  NpxTensorInfo *output = NULL;
  if (strcmp(step_generation, "lfsr") == 0)
  {
    output = generate_single_step_by_lfsr(input);
  }
  else if (strcmp(step_generation, "direct") == 0)
  {
    output = input;
  }
  else
    assert_msg(0, "%s", step_generation);

  return output;
}

npx_layerio_tsseq_t *npx_dvs_preprocess(const npx_network_t *net, const NpxTensorInfo *input, texpar_list_t *option_list)
{
  int timesteps = texpar_find_int_quiet(option_list, "timesteps", 0);
  assert(timesteps > 0);

  NpxTensorInfo *frame = npx_dvs_to_frame(input, 128, 128, timesteps, NULL);
  NpxTensorInfo *resized = npx_tensor_resize(frame, NULL, net->h, net->w);

  npx_layerio_tsseq_t *input_tsseq = npx_layerio_tsseq_alloc(timesteps);
  for (int t = 0; t < timesteps; t++)
  {
    input_tsseq->sequence[t] = npx_tensor_alloc_wo_data(3);
    input_tsseq->sequence[t]->size[0] = resized->size[0];
    input_tsseq->sequence[t]->size[1] = resized->size[1];
    input_tsseq->sequence[t]->size[2] = resized->size[2];
    npx_tensor_set_datatype(input_tsseq->sequence[t], resized->datatype);
    npx_tensor_set_contiguous_layout(input_tsseq->sequence[t]);
    input_tsseq->sequence[t]->addr = resized->addr + t * resized->stride[3];
  }

  int *is_boundary = (int *)malloc(timesteps * sizeof(int));
  for (int i = 0; i < timesteps; i++)
    is_boundary[i] = 1;
  input_tsseq->is_boundary = is_boundary;

  return input_tsseq;
}

// npx_layerio_tsseq_t *npx_dvs346_preprocess(const npx_network_t *net, const NpxTensorInfo *input, texpar_list_t *option_list)
npx_layerio_tsseq_t *npx_dvs346_preprocess(const npx_network_t *net, const NpxTensorInfo *input, int timesteps)
{
  // int timesteps = texpar_find_int_quiet(option_list, "timesteps", 0);
  assert(timesteps > 0);

#if 0
  NpxTensorInfo *denoised = npx_dvs_denoise(input, 260, 346, 10000);
  NpxTensorInfo *frame = npx_dvs_to_frame(denoised, 260, 346, timesteps, NULL);
  NpxTensorInfo *resized = npx_tensor_resize(frame, NULL, net->h, net->w);
#else
#if 0
  // down_sample & resize
  NpxTensorInfo *denoised = npx_dvs_denoise(input, 260, 346, 10000);
  NpxTensorInfo *downsampled = npx_dvs_downsample(denoised, 260, 346, net->h, net->w);
  // NpxTensorInfo *downsampled = npx_dvs_downsample(input, 260, 346, net->h, net->w);
  NpxTensorInfo *resized = npx_dvs_to_frame(downsampled, net->h, net->w, timesteps, NULL);
#else
  NpxTensorInfo *denoised = npx_dvs_denoise(input, 260, 346, 10000);
  NpxTensorInfo *downsampled = npx_dvs_downsample(denoised, 260, 346, 64, 64);
  // NpxTensorInfo *downsampled = npx_dvs_downsample(input, 260, 346, net->h, net->w);
  NpxTensorInfo *frame = npx_dvs_to_frame(downsampled, 64, 64, timesteps, NULL);
  NpxTensorInfo *resized = npx_tensor_resize(frame, NULL, net->h, net->w);
#endif
#endif

  npx_layerio_tsseq_t *input_tsseq = npx_layerio_tsseq_alloc(timesteps);
  for (int t = 0; t < timesteps; t++)
  {
    input_tsseq->sequence[t] = npx_tensor_alloc_wo_data(3);
    input_tsseq->sequence[t]->size[0] = resized->size[0];
    input_tsseq->sequence[t]->size[1] = resized->size[1];
    input_tsseq->sequence[t]->size[2] = resized->size[2];
    npx_tensor_set_datatype(input_tsseq->sequence[t], resized->datatype);
    npx_tensor_set_contiguous_layout(input_tsseq->sequence[t]);
    input_tsseq->sequence[t]->addr = resized->addr + t * resized->stride[3];
  }

  int *is_boundary = (int *)malloc(timesteps * sizeof(int));
  for (int i = 0; i < timesteps; i++)
    is_boundary[i] = 1;
  input_tsseq->is_boundary = is_boundary;

  return input_tsseq;
}