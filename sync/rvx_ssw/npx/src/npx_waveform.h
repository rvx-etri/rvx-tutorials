#ifndef __NPX_WAVEFORM_H__
#define __NPX_WAVEFORM_H__

#include "texpar_list.h"
#include "npx_tensor.h"

void npx_set_mel_spectorgram(texpar_list_t *option_list);
NpxTensorInfo *npx_log_mel_spectrogram(NpxTensorInfo *input, texpar_list_t *option_list);

#endif
