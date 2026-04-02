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


#include "ervp_printf.h"
#include "ervp_malloc.h"
#include "ervp_memory_util.h"
#include <math.h>

#if defined(WIN32)
#include "SPTK.h"
#else
#include "SPTK.h"
#endif

/* workspace */
static int dct_table_size = 0;
static float *dct_workspace = NULL;
static float *pLocalReal = NULL;
static float *pLocalImag = NULL;
static float *pWeightReal = NULL;
static float *pWeightImag = NULL;

static int dct_table_size_fft = 0;
static float *dct_workspace2 = NULL;
static float *pLocalReal2 = NULL;
static float *pLocalImag2 = NULL;
static float *pWeightReal2 = NULL;
static float *pWeightImag2 = NULL;


int dft(float *pReal, float *pImag, const int nDFTLength)
{
   int k, n;
   float *pTempReal, *pTempImag, TempReal, TempImag;

   pTempReal = fgetmem(nDFTLength);
   pTempImag = fgetmem(nDFTLength);

   memcpy(pTempReal, pReal, sizeof(float) * nDFTLength);
   memcpy(pTempImag, pImag, sizeof(float) * nDFTLength);

   for (k = 0; k < nDFTLength; k++) {
      TempReal = 0;
      TempImag = 0;
      for (n = 0; n < nDFTLength; n++) {
         TempReal += (float)(pTempReal[n] * cos(2.0 * PI * n * k / (double) nDFTLength)
             + pTempImag[n] * sin(2.0 * PI * n * k / (double) nDFTLength));
         TempImag += (float)(-pTempReal[n] * sin(2.0 * PI * n * k / (double) nDFTLength)
             + pTempImag[n] * cos(2.0 * PI * n * k / (double) nDFTLength));
      }
      pReal[k] = TempReal;
      pImag[k] = TempImag;
      //printf("pReal[%d] %f\n", k, pReal[k]);
      //printf("pImag[%d] %f\n", k, pImag[k]);
   }
   free(pTempReal);
   free(pTempImag);

   return (0);
}

int dct_create_table_fft(const int nSize)
{
   register int k;

   if (nSize == dct_table_size_fft) {
      /* no needs to resize workspace. */
      return (0);
   } else {
      /* release resources to resize workspace. */
      if (dct_workspace2 != NULL) {
         free(dct_workspace2);
         dct_workspace2 = NULL;
      }
      pLocalReal2 = NULL;
      pLocalImag2 = NULL;
      pWeightReal2 = NULL;
      pWeightImag2 = NULL;
   }

   /* getting resources. */
   if (nSize <= 0) {
      dct_table_size_fft = 0;
      return (0);
   } else {
      dct_table_size_fft = nSize;
      dct_workspace2 = fgetmem(dct_table_size_fft * 6);
      pWeightReal2 = dct_workspace2;
      pWeightImag2 = dct_workspace2 + dct_table_size_fft;
      pLocalReal2 = dct_workspace2 + (2 * dct_table_size_fft);
      pLocalImag2 = dct_workspace2 + (4 * dct_table_size_fft);

      for (k = 0; k < dct_table_size_fft; k++) {
         pWeightReal2[k] =
             cos(k * PI / (2.0 * dct_table_size_fft)) /
             sqrt(2.0 * dct_table_size_fft);
         pWeightImag2[k] =
             -sin(k * PI / (2.0 * dct_table_size_fft)) /
             sqrt(2.0 * dct_table_size_fft);
      }
      pWeightReal2[0] /= sqrt(2.0);
      pWeightImag2[0] /= sqrt(2.0);
   }

   return (0);
}

int dct_create_table(const int nSize)
{
   register int k;

   if (nSize == dct_table_size) {
      /* no needs to resize workspace. */
      return (0);
   } else {
      /* release resources to resize workspace. */
      if (dct_workspace != NULL) {
         free(dct_workspace);
         dct_workspace = NULL;
      }
      pLocalReal = NULL;
      pLocalImag = NULL;
      pWeightReal = NULL;
      pWeightImag = NULL;
   }

   /* getting resources. */
   if (nSize <= 0) {
      dct_table_size = 0;
      return (0);
   } else {
      dct_table_size = nSize;
      dct_workspace = fgetmem(dct_table_size * 6);
      pWeightReal = dct_workspace;
      pWeightImag = dct_workspace + dct_table_size;
      pLocalReal = dct_workspace + (2 * dct_table_size);
      pLocalImag = dct_workspace + (4 * dct_table_size);

      for (k = 0; k < dct_table_size; k++) {
         pWeightReal[k] =
             cos(k * PI / (2.0 * dct_table_size)) / sqrt(2.0 * dct_table_size);
         pWeightImag[k] =
             -sin(k * PI / (2.0 * dct_table_size)) / sqrt(2.0 * dct_table_size);
      }
      pWeightReal[0] /= sqrt(2.0);
      pWeightImag[0] /= sqrt(2.0);
      //for (k = 0; k < dct_table_size; k++) printf("pWeightReal[%d] %f\n", k, pWeightReal[k]);
      //for (k = 0; k < dct_table_size; k++) printf("pWeightImag[%d] %f\n", k, pWeightImag[k]);
   }

   return (0);
}

int dct_based_on_fft(float *pReal, float *pImag, const float *pInReal,
                     const float *pInImag)
{
   register int n, k;


   for (n = 0; n < dct_table_size_fft; n++) {
      pLocalReal2[n] = pInReal[n];
      pLocalImag2[n] = pInImag[n];
      pLocalReal2[dct_table_size_fft + n] = pInReal[dct_table_size_fft - 1 - n];
      pLocalImag2[dct_table_size_fft + n] = pInImag[dct_table_size_fft - 1 - n];
   }


   fft(pLocalReal2, pLocalImag2, dct_table_size_fft * 2);       /* float input */


   for (k = 0; k < dct_table_size_fft; k++) {
      pReal[k] =
          pLocalReal2[k] * pWeightReal2[k] - pLocalImag2[k] * pWeightImag2[k];
      pImag[k] =
          pLocalReal2[k] * pWeightImag2[k] + pLocalImag2[k] * pWeightReal2[k];
   }

   return (0);
}

int dct_based_on_dft(float *pReal, float *pImag, const float *pInReal,
                     const float *pInImag)
{
   register int n, k;

   for (n = 0; n < dct_table_size; n++) {
      pLocalReal[n] = pInReal[n];
      pLocalImag[n] = pInImag[n];
      pLocalReal[dct_table_size + n] = pInReal[dct_table_size - 1 - n];
      pLocalImag[dct_table_size + n] = pInImag[dct_table_size - 1 - n];
   }

   dft(pLocalReal, pLocalImag, dct_table_size * 2);

   for (k = 0; k < dct_table_size; k++) {
      pReal[k] =
          pLocalReal[k] * pWeightReal[k] - pLocalImag[k] * pWeightImag[k];
      pImag[k] =
          pLocalReal[k] * pWeightImag[k] + pLocalImag[k] * pWeightReal[k];
   }

   return (0);
}

void dct(float *in, float *out, const int size, const int m,
         const int dftmode, const int compmode)
{
   int k, i, j, iter;
   float *pReal, *pImag;
   float *x, *y;
   float *x2, *y2;
   //printf("in: %.8f\n", in[0]);
   //printf("in: %.8f\n", in[1]);
   //printf("in: %.8f\n", in[2]);
   //printf("size: %d\n", size);
   //printf("m: %d\n", m);
   //printf("compmode: %d\n", compmode);
   //printf("out: %.8f\n", out[0]);
   //printf("out: %.8f\n", out[1]);
   //printf("out: %.8f\n", out[2]);

   x = fgetmem(2 * size);
   y = x + size;
   pReal = fgetmem(2 * size);
   pImag = pReal + size;
   x2 = fgetmem(2 * size);
   y2 = x2 + size;

   for (k = 0; k < size; k++) {
      x[k] = in[k];
      //y[k] = in[k + size];
      y[k] = 0;
      x2[k] = x[k];
      y2[k] = y[k];
   }

   iter = 0;
   i = size;
   while ((i /= 2) != 0) {
      iter++;
   }
   j = 1;
   for (i = 1; i <= iter; i++) {
      j *= 2;
   }
   //printf("%d %d\n", size, j);
   if (size != j || dftmode) {
      //printf("usedct %d\n", dftmode);
      dct_create_table(size);
      dct_based_on_dft(pReal, pImag, x2, y2);
   } else {
      dct_create_table_fft(size);
      dct_based_on_fft(pReal, pImag, x2, y2);
   }
	//printf("pReal: %.8f\n", pReal[0]);
	//printf("pReal: %.8f\n", pReal[1]);
	//printf("pReal: %.8f\n", pReal[2]);

   for (k = 0; k < m; k++) {
      out[k] = pReal[k];
      if (compmode == TR) {
         out[k + size] = pImag[k];
         //printf("COMPMODE\n");
      }
   }
   //printf("out: %.8f\n", out[0]);
   //printf("out: %.8f\n", out[1]);
   //printf("out: %.8f\n", out[2]);

   free(x);
   free(x2);
   free(pReal);
}
