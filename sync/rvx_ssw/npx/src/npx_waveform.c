#include <stdint.h>
#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_malloc.h"
#include "texpar_api.h"

#include "SPTK.h"

#include "npx_waveform.h"

void npx_set_mel_spectorgram(texpar_list_t *option_list)
{
  int sample_rate = texpar_find_int_quiet(option_list, "mel_spectrogram.sample_rate", 16000);
  int n_fft = texpar_find_int_quiet(option_list, "mel_spectrogram.n_fft", 512);
  int win_length = texpar_find_int_quiet(option_list, "mel_spectrogram.win_length", 400);
  int n_mels = texpar_find_int_quiet(option_list, "mel_spectrogram.n_mels", 40);

  init_window(HAMMING, win_length, 0);
  init_mel_filter_banks(sample_rate, n_fft, n_mels);
}

NpxTensorInfo *npx_log_mel_spectrogram(NpxTensorInfo *input, texpar_list_t *option_list)
{
  int num_samples = texpar_find_int_quiet(option_list, "mel_spectrogram.num_samples", 16000);
  int sample_rate = texpar_find_int_quiet(option_list, "mel_spectrogram.sample_rate", 16000);
  int n_fft = texpar_find_int_quiet(option_list, "mel_spectrogram.n_fft", 512);
  int win_length = texpar_find_int_quiet(option_list, "mel_spectrogram.win_length", 400);
  int hop_length = texpar_find_int_quiet(option_list, "mel_spectrogram.hop_length", 160);
  int n_mels = texpar_find_int_quiet(option_list, "mel_spectrogram.n_mels", 40);

  assert(input->size[1] == 1);
  assert(input->size[2] == 1);

  int waveform_length = input->size[0];
  int16_t *waveform = input->addr;
  int nframes = (int)(ceil((float)(abs(num_samples - win_length)) / hop_length));
  float *pad_fwaveform = malloc(sizeof(float)*num_samples);
  float *lms = malloc(sizeof(float)*nframes*n_mels);
  
  NpxTensorInfo *mel_spectrogram = npx_tensor_alloc_wo_data(3);
  mel_spectrogram->size[0] = n_mels;
  mel_spectrogram->size[1] = nframes;
  mel_spectrogram->size[2] = 1;
  npx_tensor_set_datatype(mel_spectrogram, MATRIX_DATATYPE_SINT08);
  npx_tensor_alloc_data(mel_spectrogram);

  int8_t *lms_int8 = mel_spectrogram->addr;

  for(int k=0; k < num_samples; k++)
  {
    if(k < waveform_length)
      pad_fwaveform[k] = (float)(waveform[k])/32768;
    else
      pad_fwaveform[k] = 0;
  }

  get_log_mel_spectrogram(pad_fwaveform, num_samples, lms, (float)sample_rate, win_length, hop_length, n_fft, n_mels);

  for(int i = 0; i < nframes*n_mels; i++)
  {
    lms_int8[i] = (int8_t)lms[i];
  }

  free(pad_fwaveform);
  free(lms);

  return mel_spectrogram;
}


