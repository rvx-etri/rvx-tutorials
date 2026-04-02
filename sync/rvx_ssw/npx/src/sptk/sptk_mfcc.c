/* ----------------------------------------------------------------- */
/*             The Speech Signal Processing Toolkit (SPTK)           */
/*             developed by SPTK Working Group                       */
/*             http://sp-tk.sourceforge.net/                         */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 1984-2007  Tokyo Institute of Technology           */
/*                           Interdisciplinary Graduate School of    */
/*                           Science and Engineering                 */
/*                                                                   */
/*                1996-2016  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the SPTK working group nor the names of its */
/*   contributors may be used to endorse or promote products derived */
/*   from this software without specific prior written permission.   */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

/****************************************************************

  $Id: _mfcc.c,v 1.16 2016/12/22 10:53:07 fjst15124 Exp $

  Mel-Frequency Cepstral Analysis

  void mfcc(in, mc, sampleFreq, alpha, eps, wlng, flng
  m, n, ceplift, dftmode, usehamming);

  float  *in        : input sequence
  float  *mc        : mel-frequency cepstral coefficients
  float  sampleFreq : sample frequency
  float  alpha      : pre-emphasis coefficient
  float  eps        : epsilon
  int     wlng       : frame length of input sequence
  int     flng       : frame length for fft
  int     m          : order of cepstrum
  int     n          : number of channel for mel-filter bank
  int     ceplift    : liftering coefficients
  Boolean dftmode    : use dft
  Boolean usehamming : use hamming window

  This code was modified based on the site by hujang 
  (https://ratsgo.github.io/speechbook/docs/fe/mfcc#log-mel-spectrum)

 ******************************************************************/

#include "ervp_printf.h"
#include "ervp_malloc.h"
#include "ervp_memory_util.h"
#include "ervp_assert.h"
#include <math.h>

#if defined(WIN32)
#include "SPTK.h"
#else
#include "SPTK.h"
#endif

//#define MEL 1127.01048
#define MEL 2595 
#define EZERO (-1.0E10)

float freq_mel(float freq)
{
  return MEL * log(freq / 700.0 + 1.0);
}


float sample_mel(int sample, int num, float fs)
{
  float freq;
  freq = (float) (sample + 1) / (float) (num) * (fs / 2.0);

  return freq_mel(freq);
}

float cal_energy(float *x, const int leng)
{
  int k;
  float energy = 0.0;
  for (k = 0; k < leng; k++)
    energy += x[k] * x[k];

  return ((energy <= 0) ? EZERO : log(energy));
}

void hamming(float *x, const int leng)
{
  int k;
  float arg;

  arg = M_2PI / (leng - 1);
  for (k = 0; k < leng; k++)
    x[k] *= (0.54 - 0.46 * cos(k * arg));
}

void pre_emph(float *x, float *y, const float alpha, const int leng)
{
  int k;
  y[0] = x[0];// * (1.0 - alpha);
  //for (k = 1; k < leng; k++)
  for (k = leng - 1; k >= 1; k--)
    y[k] = x[k] - x[k - 1] * alpha;
}

#define DEBUG  0

void spec(float *x, float *sp, const int leng)
{
  static int cnt = 0;
  int k, no;
  float *y;

#if 0
  float *mag;
  no = leng / 2;

  y = fgetmem(leng + no);
  mag = y + leng;

  fftr(x, y, leng);
  for (k = 1; k < no; k++) {
    mag[k] = x[k] * x[k] + y[k] * y[k];
    sp[k] = sqrt(mag[k]);
  }
#else
  no = leng / 2 + 1;

  y = fgetmem(leng + no);
  //mag = y + leng;

  fftr(x, y, leng);

#if DEBUG
  if(cnt==0 || cnt==97)
    for(k = 0; k < 3; k++)
      printf("\ndft[%d] %.10f  / %.10fj", k, x[k], y[k]);
#endif

  for (k = 0; k < no; k++) {
    sp[k] = x[k] * x[k] + y[k] * y[k];
    sp[k] /= leng; 
    //sp[k] = sqrt(mag[k]);
    //printf("sp[%d]: %.18f\n", k, sp[k]);
  }
#endif
  cnt++;
  free(y);
}

void get_mel_filter_banks(float *mfbank, float fs, const int leng, const int n)
{
  int i, k, m;
  int low_freq_mel = 0;
  int f_m_minus, f_m, f_m_plus;
  //int *bin = (int *) getmem((size_t)(n + 2), sizeof(int));
  float *bin = fgetmem(n + 2);
  float *mel_points = fgetmem(n + 2);
  float *hz_points = fgetmem(n + 2);
  float high_freq_mel;
  float mel_points_interval;
  int fbank_width = (int)floor(leng / 2 + 1);
  float (*mfb_2d)[fbank_width] = (float (*)[fbank_width])mfbank;
  //printf("%d\n", fbank_width);

  high_freq_mel = (2595 * log10(1 + (fs / 2) / 700));  // Convert Hz to Mel
  mel_points_interval = (high_freq_mel - low_freq_mel) / (n + 1);

  //mel_points = np.linspace(low_freq_mel, high_freq_mel, n + 2)  # Equally spaced in Mel scale
  mel_points[0] = low_freq_mel;
  for(i = 1; i < (n+2); i++)
  {
    mel_points[i] = mel_points[i-1] + mel_points_interval;
    //printf("mel_points[%d]: %f\n", i, mel_points[i]);
  }

  //hz_points = (700 * (10**(mel_points / 2595) - 1))  # Convert Mel to Hz
  for(i = 0; i < (n+2); i++)
  {
    hz_points[i] = (700 * (pow(10,(mel_points[i] / 2595)) - 1));
    //printf("hz_points[%d]: %f\n", i, hz_points[i]);
  }

  //bin = np.floor((NFFT + 1) * hz_points / sample_rate)
  for(i = 0; i < (n+2); i++)
  {
    bin[i] = floor((leng + 1) * hz_points[i] / fs);
    //printf("bin[%d]: %f\n", i, bin[i]);
  }

  //mfb_2d = np.zeros((n, int(np.floor(NFFT / 2 + 1))))
  //for m in range(1, n + 1):
  //  f_m_minus = int(bin[m - 1])   # left
  //  f_m = int(bin[m])             # center
  //  f_m_plus = int(bin[m + 1])    # right
  //  for k in range(f_m_minus, f_m):
  //    fbank[m - 1, k] = (k - bin[m - 1]) / (bin[m] - bin[m - 1])
  //  for k in range(f_m, f_m_plus):
  //    fbank[m - 1, k] = (bin[m + 1] - k) / (bin[m + 1] - bin[m])
  for(m = 1; m < (n + 1); m++)
  {
    f_m_minus = (int)bin[m - 1];  // left
    f_m = (int)bin[m];             // center
    f_m_plus = (int)bin[m + 1];   // right
    //printf("%d %d %d\n", f_m_minus, f_m, f_m_plus);
    for(k = f_m_minus; k < f_m; k++)
      mfb_2d[m - 1][k] = (k - bin[m - 1]) / (bin[m] - bin[m - 1]);
    for(k = f_m; k < f_m_plus; k++)
      mfb_2d[m - 1][k] = (bin[m + 1] - k) / (bin[m + 1] - bin[m]);
  }
}

static float *mfb = NULL;

void init_mel_filter_banks(float fs, const int leng, const int n)
{
  static int current_leng = -1;
  if(current_leng != leng)
  {
    if(leng > current_leng)
    {
      if (mfb != NULL)
        free(mfb);
      int fbank_width = (int)floor(leng / 2 + 1);
      mfb = fgetmem(n*fbank_width);
    }
    get_mel_filter_banks(mfb, fs, leng, n);
    current_leng = leng;
  }
}

void fbank(float *x, float *fb, const float eps, const float fs,
    const int leng, const int n)
{
  int fbank_width = (int)floor(leng / 2 + 1);
  int i, k;
  float sum;

  init_mel_filter_banks(fs, leng, n);

  //for (k = 0; k < fbank_width; k++) printf("sp[%d]: %.18f\n", k, x[k]);

  float (*mfb_2d)[fbank_width] = (float (*)[fbank_width])mfb;
  //for(k = 0; k < n; k++)
  //	for (i = 0; i < fbank_width; i++)
  //		printf("mfb[%d][%d]: %.18f\n", k, i, mfb_2d[k][i]);

  for(k = 0; k < n; k++)
  {
    sum = 0;
    for(i = 0; i < fbank_width; i++)
    {
      sum += x[i]*mfb_2d[k][i];
    }
    fb[k] = sum;
    //printf("###fb[%d]: %.18f\n", k, fb[k]);
  }

  for(k = 0; k < n; k++)
  {
    if (fb[k] < eps)
      fb[k] = eps;
    fb[k] = 10 * log10(fb[k]); // hujang 20 -> 10 (251106)
    //printf("\nfb[%d]: %.18f - 0x%08x", k, fb[k], &fb[k]);
  }

#if 0
  int k, fnum, no, chanNum = 0;
  int *noMel;
  float *w, *countMel;
  float maxMel, kMel;

  no = leng / 2;
  noMel = (int *) getmem((size_t) no, sizeof(int));
  countMel = fgetmem(n + 1 + no);
  w = countMel + n + 1;
  maxMel = freq_mel(fs / 2.0);

  for (k = 0; k <= n; k++)
    countMel[k] = (float) (k + 1) / (float) (n + 1) * maxMel;
  for (k = 1; k < no; k++) {
    kMel = sample_mel(k - 1, no, fs);
    while (countMel[chanNum] < kMel && chanNum <= n)
      chanNum++;
    noMel[k] = chanNum;
  }

  for (k = 1; k < no; k++) {
    chanNum = noMel[k];
    kMel = sample_mel(k - 1, no, fs);
    w[k] = (countMel[chanNum] - kMel) / (countMel[0]);
  }

  for (k = 1; k < no; k++) {
    fnum = noMel[k];
    if (fnum > 0)
      fb[fnum] += x[k] * w[k];
    if (fnum <= n)
      fb[fnum + 1] += (1 - w[k]) * x[k];
  }

  free(noMel);
  free(countMel);

  for (k = 1; k <= n; k++) {
    if (fb[k] < eps)
      fb[k] = eps;
    fb[k] = log(fb[k]);
  }
#endif
}



void lifter(float *x, float *y, const int m, const int leng)
{
  int k;
  float theta;
  for (k = 0; k < m; k++) {
    theta = PI * (float) k / (float) leng;
    y[k] = (1.0 + (float) leng / 2.0 * sin(theta)) * x[k];
  }
}

void mfcc(float *in, float *mc, const float sampleFreq, const float alpha,
    const float eps, const int wlng, const int flng, const int m,
    const int n, const int ceplift, const int dftmode,
    const int usehamming)
{
  static float *x = NULL, *px, *wx, *sp, *fb, *dc;
  float energy = 0.0;
  // float c0 = 0.0;
  int k;
  //printf("%0.10f\n", in[0]);
  //printf("%0.10f\n", in[1]);
  //printf("%0.10f\n", in[2]);
  //printf("m %d\n", m);
  //printf("n %d\n", n);
  //printf("ceplift %d\n", ceplift);
  //printf("dftmode %d\n", dftmode);
  //printf("usehamming %d\n", usehamming);

  int memory_size = wlng + wlng + flng + flng + n * 2 + 1 + m * 2;
  if (x == NULL) {
    x = fgetmem(memory_size);
    px = x + wlng;
    wx = px + wlng;
    sp = wx + flng;
    fb = sp + flng;
    dc = fb + n * 2 + 1;
  } else {
    free(x);
    x = fgetmem(memory_size);
    px = x + wlng;
    wx = px + wlng;
    sp = wx + flng;
    fb = sp + flng;
    dc = fb + n * 2 + 1;
  }
  /* need to intizlize dct workspace to zeros */
  fillz(fb + 1, n * 2 + m * 2, sizeof(float));

  movem(in, x, sizeof(*in), wlng);
  /* calculate energy */
  //energy = cal_energy(x, wlng);
  //pre_emph(x, px, alpha, wlng);
  /*no pre emphasis*/
  //movem(x, px, sizeof(*x), wlng);
  //printf("window\n");
  /* apply hamming window */
  if (usehamming)
  {
    //window(HANNING, px, wlng, 0);
    window(HAMMING, x, wlng, 0);
    //printf("use windowing\n");
  }
  for (k = 0; k < wlng; k++)
  {
    wx[k] = x[k];
    //printf("wx[%d]: %.8f   ", k, wx[k]);
  }
  //printf("\n");
  //printf("spec\n");
  spec(wx, sp, flng);
  //printf("fbank\n");
  fbank(sp, fb, eps, sampleFreq, flng, n);
  /* calculate 0'th coefficient */
  //for (k = 1; k <= n; k++)
  //   c0 += fb[k];
  //c0 *= sqrt(2.0 / (float) n);
  //dct(fb+1, dc, n, m, dftmode, 0);
  //printf("dct\n");
  dct(fb, dc, n, m+1, dftmode, 0);
  //printf("dc: %.8f\n", dc[0]);
  //printf("dc: %.8f\n", dc[1]);
  //printf("dc: %.8f\n", dc[2]);

  /* liftering */
  //if (ceplift > 0)
  //   lifter(dc, mc, m, ceplift);
  //else
  //   movem(dc, mc, sizeof(*dc), m);
  movem(dc+1, mc, sizeof(*dc), m);
  //printf("end\n");

#if 0
  for (k = 0; k < m - 1; k++)
    mc[k] = mc[k + 1];
  mc[m - 1] = c0;
  mc[m] = energy;
#endif

}

void get_mfcc(float *in, int in_len, float *mfccs, const float fs,
    const int wsize, const int wstride, const int fftsize, const int dcc, const int nmfb)
{
  assert(in_len>=wsize);
  int i;
  const float pre_emphasis = 0.97;
  const float eps = 1.11e-16;
  int nframes = (int)(floor((float)(in_len - wsize) / wstride)) + 1; // without pad
  //int nframes = (int)(ceil((float)(in_len - wsize) / wstride)) + 1; // with pad
  float *data;
  float *pre_in = fgetmem(in_len);
  float *mc;

  pre_emph(in, pre_in, pre_emphasis, in_len);

#if 0
  printf("nframes: %d\n", nframes);
  printf("in: %.8f\n", in[0]);
  printf("in: %.8f\n", in[1]);
  printf("in: %.8f\n", in[2]);
  printf("in_len: %d\n", in_len);
  printf("fs: %f\n", fs);
  printf("wsize: %d\n", wsize);
  printf("wstride: %d\n", wstride);
  printf("fftsize: %d\n", fftsize);
  printf("dcc: %d\n", dcc);
  printf("nmfb: %d\n", nmfb);
#endif

  for(i = 0; i < nframes; i++)
  //for(i = 0; i < 1; i++)
  {
    //printf("############### frame id: %d ################\n", i);
    mc = &mfccs[i*dcc];
    data = &pre_in[i*wstride];
    mfcc(data, mc, fs, 0, eps, wsize, fftsize, dcc, nmfb, 0, 0, 1);
  }

  free(pre_in);

#if 0
  printf("mfcc: %.8f\n", mfccs[0]);
  printf("mfcc: %.8f\n", mfccs[1]);
  printf("mfcc: %.8f\n", mfccs[2]);
#endif
}

//log_mel_spectrum(data, p_lms, fs, 0, eps, wsize, fftsize, nmfb, 1);
// wlng: frame size
// flng: fft size
// n: mel filter bank
void log_mel_spectrum(float *in, float *out, const float sampleFreq, const float alpha,
    const float eps, const int wlng, const int flng, const int n, const int usehamming)
{
  static float *x = NULL, *px, *wx, *sp, *fb;
  int k;
  int memory_size = wlng + wlng + flng + flng + n * 2 + 1;
  if (x == NULL) {
    x = fgetmem(memory_size);
    px = x + wlng;
    wx = px + wlng;
    sp = wx + flng;
    fb = sp + flng;
  } else {
    free(x);
    x = fgetmem(memory_size);
    px = x + wlng;
    wx = px + wlng;
    sp = wx + flng;
    fb = sp + flng;
  }

  /* need to intizlize dct workspace to zeros */
  fillz(fb, n * 2 + 1, sizeof(float));

  movem(in, x, sizeof(*in), wlng);

  /*no pre emphasis*/
  //movem(x, px, sizeof(*x), wlng);
  //printf("window\n");

  /* apply hamming window */
  if (usehamming)
  {
    //window(HANNING, px, wlng, 0);
    window(HAMMING, x, wlng, 0);
    //printf("use windowing\n");
  }

  for (k = 0; k < flng; k++)
  {
    if(k < wlng)
      wx[k] = x[k];
    else
      wx[k] = 0;
  }
#if DEBUG
  if(cnt==0 || cnt==97)
  {
    for(k = 0; k < 3; k++)
      printf("\nwx[%d]: %.10f", k, wx[k]);
    for(k = flng-3; k < flng; k++)
      printf("\nwx[%d] %.10f", k, wx[k]);
  }
#endif

  //printf("spec\n");
  spec(wx, sp, flng);
#if DEBUG
  if(cnt==0 || cnt==97)
    for(k = 0; k < 3; k++)
      printf("\nsp[%d]: %.10f", k, sp[k]*10000000000);
#endif

  //printf("fbank\n");
  fbank(sp, fb, eps, sampleFreq, flng, n);

  movem(fb, out, sizeof(*fb), n);

#if DEBUG
  if(cnt==0 || cnt==97)
    for(k = 0; k < 3; k++)
      printf("\nout[%d]: %.10f - 0x%08x", k, out[k], &out[k]);
  cnt++;
#endif
}

void get_log_mel_spectrogram(float *in, int in_len, float *lms, const float fs,
    const int wsize, const int wstride, const int fftsize, const int nmfb)
{
  assert(in_len>=wsize);
  int i;
  const float pre_emphasis = 0.97;
  const float eps = 1.11e-16;
  int nframes = (int)(floor((float)(in_len - wsize) / wstride)) + 1; // without pad
  //int nframes = (int)(ceil((float)(in_len - wsize) / wstride)) + 1; // with pad
  float *data;
  float *pre_in = fgetmem(in_len);
  float *p_lms;

  pre_emph(in, pre_in, pre_emphasis, in_len);

#if 0
  printf("\nin_len: %d", in_len);
  printf("\nnframes: %d", nframes);
  printf("\nfs: %f", fs);
  printf("\nwsize: %d", wsize);
  printf("\nwstride: %d", wstride);
  printf("\nfftsize: %d", fftsize);
  printf("\nnmfb: %d", nmfb);
#endif

#if 0
  printf("in: %.8f\n", in[0]);
  printf("in: %.8f\n", in[1]);
  printf("in: %.8f\n", in[2]);
  printf("in: %.8f\n", pre_in[0]);
  printf("in: %.8f\n", pre_in[1]);
  printf("in: %.8f\n", pre_in[2]);
  printf("in: %.8f\n", pre_in[3]);
#endif

  for(i = 0; i < nframes; i++)
  //for(i = 0; i < 1; i++)
  {
    //printf("############### frame id: %d ################\n", i);
    p_lms = &lms[i*nmfb];
    data = &pre_in[i*wstride];
    log_mel_spectrum(data, p_lms, fs, 0, eps, wsize, fftsize, nmfb, 1);
  }
#if 0
  printf("\n%f", lms[0]);
  printf("\n%f", lms[1]);
  printf("\n%f", lms[2]);
  printf("\n%f", lms[nframes*nmfb-3]);
  printf("\n%f", lms[nframes*nmfb-2]);
  printf("\n%f", lms[nframes*nmfb-1]);
#endif

  free(pre_in);
}

