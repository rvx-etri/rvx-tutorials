#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_malloc.h"
#include "ervp_memory_util.h"
#include "ervp_fakefile.h"

#include <string.h>

#include "texpar_list.h"
#include "texpar_api.h"

#include "npx_sample.h"
#include "npx_util.h"
#include "npx_tensor.h"
#include "npx_dvs.h"

npx_rawinput_t *npx_load_sample_matrix(const char *filename, texpar_list_t *option_list);
npx_rawinput_t *npx_load_mnist_dataset(const char *filename);
npx_rawinput_t *npx_load_cifar10_dataset(const char *filename);
npx_rawinput_t *npx_load_dvsgesture_dataset(const char *filename);
npx_rawinput_t *npx_load_channelresponse_dataset(const char *filename);
npx_rawinput_t *npx_load_speechcommands_dataset(const char *filename);

npx_rawinput_t *npx_load_sample(const char *filename, const char *pre_fname)
{
  texpar_list_t *section_list = texpar_read_file_with_section(pre_fname);
  texpar_section_t *s = (texpar_section_t *)section_list->front->val;
  texpar_list_t *option_list = s->option_list;

  assert(strcmp(s->type, "[preprocess]") == 0);
  char *input_str = texpar_find_str(option_list, "input", NULL);
  assert(input_str);

  printf("\n%s\n", input_str);
  npx_rawinput_t *npx_sample;
  if ((strcmp(input_str, "mnist_opendataset") == 0) || (strcmp(input_str, "fmnist_opendataset") == 0))
  {
    npx_sample = npx_load_mnist_dataset(filename);
  }
  else if ((strcmp(input_str, "cifar10_opendataset") == 0) || (strcmp(input_str, "gtsrb_opendataset") == 0))
  {
    npx_sample = npx_load_cifar10_dataset(filename);
  }
  else if (strcmp(input_str, "dvsgesture_opendataset") == 0)
  {
    npx_sample = npx_load_dvsgesture_dataset(filename);
  }
  else if (strcmp(input_str, "speechcommands_opendataset") == 0)
  {
    npx_sample = npx_load_speechcommands_dataset(filename);
  }
  else if ((strcmp(input_str, "channelresponse_dataset") == 0))
  {
    npx_sample = npx_load_channelresponse_dataset(filename);
  }
  else
  {
    assert_msg(0, "\nunsupported input: %s", input_str);
  }
  printf("\nsample: %s (%s)", filename, input_str);
  return npx_sample;
}

npx_rawinput_t *npx_load_mnist_dataset(const char *filename)
{
  int input_c = 1;
  int input_h = 28;
  int input_w = 28;

  npx_rawinput_t *npx_sample = (npx_rawinput_t *)malloc(sizeof(npx_rawinput_t));
  assert(npx_sample);

  npx_sample->type = MATRIX3D;
  npx_sample->tensor = npx_tensor_alloc_wo_data(3);
  npx_sample->tensor->size[0] = input_w;
  npx_sample->tensor->size[1] = input_h;
  npx_sample->tensor->size[2] = input_c;
  npx_tensor_set_datatype(npx_sample->tensor, MATRIX_DATATYPE_UINT08);
  npx_tensor_alloc_data(npx_sample->tensor);

  // read data
  FAKEFILE *fp = ffopen(filename, "r");
  assert(fp);
  assert(fp->head);
  assert(fp->head->next == NULL);
  int ndata = npx_tensor_elements(npx_sample->tensor);
  int fread_size = ffread(npx_sample->tensor->addr, npx_tensor_get_datasize(npx_sample->tensor), ndata, fp);
  assert(fread_size == (ndata * npx_tensor_get_datasize(npx_sample->tensor)));

  // read label
  ffread(&(npx_sample->label), sizeof(int32_t), 1, fp);

  // scale
  npx_sample->scaled = 255;

  ffclose(fp);

  return npx_sample;
}

npx_rawinput_t *npx_load_cifar10_dataset(const char *filename)
{
  int input_c = 3;
  int input_h = 32;
  int input_w = 32;

  npx_rawinput_t *npx_sample = (npx_rawinput_t *)malloc(sizeof(npx_rawinput_t));
  assert(npx_sample);

  npx_sample->type = MATRIX3D;
  npx_sample->tensor = npx_tensor_alloc_wo_data(3);
  npx_sample->tensor->size[0] = input_w;
  npx_sample->tensor->size[1] = input_h;
  npx_sample->tensor->size[2] = input_c;
  npx_tensor_set_datatype(npx_sample->tensor, MATRIX_DATATYPE_UINT08);
  npx_tensor_alloc_data(npx_sample->tensor);

  // read data
  FAKEFILE *fp = ffopen(filename, "r");
  assert(fp);
  assert(fp->head);
  assert(fp->head->next == NULL);
  int ndata = npx_tensor_elements(npx_sample->tensor);
  int fread_size = ffread(npx_sample->tensor->addr, npx_tensor_get_datasize(npx_sample->tensor), ndata, fp);
  assert(fread_size == (ndata * npx_tensor_get_datasize(npx_sample->tensor)));

  // read label
  ffread(&(npx_sample->label), sizeof(int32_t), 1, fp);

  // scale
  npx_sample->scaled = 255;

  ffclose(fp);

  return npx_sample;
}

npx_rawinput_t *npx_load_dvsgesture_dataset(const char *filename)
{
  FAKEFILE *fp = ffopen(filename, "r");
  assert(fp);
  assert(fp->head);
  assert(fp->head->next == NULL);

  npx_rawinput_t *npx_sample = (npx_rawinput_t *)malloc(sizeof(npx_rawinput_t));
  assert(npx_sample);
  npx_sample->type = DVS;
  npx_sample->tensor = npx_tensor_alloc_wo_data(2);
  npx_sample->tensor->size[0] = 4;
  npx_sample->tensor->size[1] = (fp->head->size - 4) / sizeof(npx_dvs_t);
  npx_tensor_set_datatype(npx_sample->tensor, MATRIX_DATATYPE_SINT32);
  npx_tensor_alloc_data(npx_sample->tensor);

  // read data
  int ndata = npx_tensor_elements(npx_sample->tensor);
  int fread_size = ffread(npx_sample->tensor->addr, npx_tensor_get_datasize(npx_sample->tensor), ndata, fp);
  assert(fread_size == npx_tensor_sizes(npx_sample->tensor));

  // read label
  ffread(&(npx_sample->label), sizeof(int32_t), 1, fp);

  // scale
  npx_sample->scaled = 1;

  ffclose(fp);

  return npx_sample;
}

npx_rawinput_t *npx_load_channelresponse_dataset(const char *filename)
{
  int input_c = 1;
  int input_h = 9;
  int input_w = 9;

  npx_rawinput_t *npx_sample = (npx_rawinput_t *)malloc(sizeof(npx_rawinput_t));
  assert(npx_sample);

  npx_sample->type = MATRIX3D;
  npx_sample->tensor = npx_tensor_alloc_wo_data(3);
  npx_sample->tensor->size[0] = input_w;
  npx_sample->tensor->size[1] = input_h;
  npx_sample->tensor->size[2] = input_c;
  npx_tensor_set_datatype(npx_sample->tensor, MATRIX_DATATYPE_UINT08);
  npx_tensor_alloc_data(npx_sample->tensor);

  // read data
  FAKEFILE *fp = ffopen(filename, "r");
  assert(fp);
  assert(fp->head);
  assert(fp->head->next == NULL);
  int ndata = npx_tensor_elements(npx_sample->tensor);
  int fread_size = ffread(npx_sample->tensor->addr, npx_tensor_get_datasize(npx_sample->tensor), ndata, fp);
  assert(fread_size == (ndata * npx_tensor_get_datasize(npx_sample->tensor)));

  // read label
  ffread(&(npx_sample->label), sizeof(int32_t), 1, fp);

  // scale
  npx_sample->scaled = 255;

  ffclose(fp);

  return npx_sample;
}

npx_rawinput_t *npx_load_speechcommands_dataset(const char *filename)
{
  npx_rawinput_t *npx_sample = (npx_rawinput_t *)malloc(sizeof(npx_rawinput_t));
  assert(npx_sample);
  int file_size = fakefile_get_size_by_name(filename);
  int waveform_length = (file_size - sizeof(int32_t)) / sizeof(int16_t);
  // int waveform_length = 16000;

  npx_sample->type = WAVEFORM;
  npx_sample->tensor = npx_tensor_alloc_wo_data(3);
  npx_sample->tensor->size[0] = waveform_length;
  npx_sample->tensor->size[1] = 1;
  npx_sample->tensor->size[2] = 1;
  npx_tensor_set_datatype(npx_sample->tensor, MATRIX_DATATYPE_SINT16);
  npx_tensor_alloc_data(npx_sample->tensor);

  // read data
  FAKEFILE *fp = ffopen(filename, "r");
  assert(fp);
  assert(fp->head);
  assert(fp->head->next == NULL);
  int ndata = npx_tensor_elements(npx_sample->tensor);
  int fread_size = ffread(npx_sample->tensor->addr, npx_tensor_get_datasize(npx_sample->tensor), ndata, fp);
  assert(fread_size == (ndata * npx_tensor_get_datasize(npx_sample->tensor)));

  // read label
  ffread(&(npx_sample->label), sizeof(int32_t), 1, fp);

  // scale
  npx_sample->scaled = 1;
  // npx_tensor_print(npx_sample->tensor, 5);
  printf("\nlabel: %d", npx_sample->label);

  ffclose(fp);

  return npx_sample;
}

#if 0
npx_rawinput_t *npx_load_sample_matrix(char *filename, char *net_fname)
{
  FAKEFILE *fp;
  int fread_size = 0;
  int ndata;
  texpar_list_t *section_list = texpar_read_file_with_section(net_fname);
  texpar_iter_t *node = section_list->front;
  texpar_section_t *s = (texpar_section_t *)node->val;
  texpar_list_t *option_list = s->option_list;

  int timesteps = texpar_find_int_quiet(option_list, "timesteps", 1);
  printf("timesteps: %d\n", timesteps);
  int input_c = texpar_find_int_quiet(option_list, "input_c", 1);
  printf("input_c: %d\n", input_c);
  int input_h = texpar_find_int_quiet(option_list, "input_h", 1);
  printf("input_h: %d\n", input_h);
  int input_w = texpar_find_int_quiet(option_list, "input_w", 1);
  printf("input_w: %d\n", input_w);

  npx_rawinput_t *npx_sample = (npx_rawinput_t *)malloc(sizeof(npx_rawinput_t));
  assert(npx_sample);

  if(strstr(filename, "matrix4d")!=NULL)
  {
    ndata = input_h * input_w * input_c * timesteps;
    npx_sample->type = MATRIX4D;
    npx_sample->tensor = npx_tensor_alloc_wo_data(4);
    npx_sample->tensor->size[3] = timesteps;
    npx_tensor_set_datatype(npx_sample->tensor, MATRIX_DATATYPE_SINT08);
  }
  else if(strstr(filename, "matrix3d")!=NULL)
  {
    ndata = input_h * input_w * input_c;
    npx_sample->type = MATRIX3D;
    npx_sample->tensor = npx_tensor_alloc_wo_data(3);
    npx_tensor_set_datatype(npx_sample->tensor, MATRIX_DATATYPE_SINT08);
  }
  else
    npx_sample->type = UNKNOWN;

  printf("npx_sample->type: %d\n", npx_sample->type);
  npx_sample->tensor->size[0] = input_w;
  npx_sample->tensor->size[1] = input_h;
  npx_sample->tensor->size[2] = input_c;
  npx_tensor_alloc_data(npx_sample->tensor);
  printf("npx_sample: %d %d, %d\n", npx_sample->tensor->size[2], npx_sample->tensor->size[1], npx_sample->tensor->size[0]);

  fp = ffopen(filename, "r");
  // printf("npx_sample File Size: %d\n", fakefile_get_size(fp));
  // printf("number of pixels %d\n", ndata);
  // read data
  fread_size = ffread(npx_sample->tensor->addr, npx_sample->tensor->stride[0], ndata, fp);
  if (fread_size != ndata*npx_sample->tensor->stride[0])
  {
    printf("npx_sample file is wrong (fread_size: %d / data_size: %d)\n", fread_size, ndata);
  }

  // read target
  ffread(&npx_sample->class, sizeof(int32_t), 1, fp);
  printf("npx_sample->class: %d\n", npx_sample->class);

  ffclose(fp);

  npx_tensor_print(npx_sample->tensor, 0, 0);

  return npx_sample;
}
#endif