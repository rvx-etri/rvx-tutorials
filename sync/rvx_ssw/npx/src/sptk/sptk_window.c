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
   $Id: _window.c,v 1.23 2016/12/22 10:53:14 fjst15124 Exp $

   Window function
   ---------------

       float  window( name, x, size, pnflg );

       char *name : window name

                    blackman, hamming,
                    hanning,  bartlett, trapezoid

       real *x    : 1 frame data
       int  size  : window(frame) size
       int  nflg  : normalizing flag

            nflg = 0 : don't normalize
                   1 : normalize by power
                   2 : normalize by magnitude

       set windowed value to "*x" and return "normalizing gain".
*****************************************************************/

#include "ervp_printf.h"
#include "ervp_malloc.h"
#include "ervp_platform_api.h"
#include <math.h>

#if defined(WIN32)
#include "SPTK.h"
#else
#include "SPTK.h"
#endif

/************************************************
   Blackman window

       float  *blackman(w, leng)

       float  *w   : window values
       int     leng : window length
************************************************/

static float *blackman(float *w, const int leng)
{
   int i;
   float arg, x;
   float *p;

   arg = M_2PI / (leng - 1);
   for (p = w, i = 0; i < leng; i++) {
      x = arg * i;
      *p++ = 0.42 - 0.50 * cos(x) + 0.08 * cos(x + x);
   }
   return (w);
}


/************************************************
   Hamming window

       float  *hamming(w, leng)
       float  *w   : window values
       int     leng : window length
************************************************/

static float *hamming(float *w, const int leng)
{
   int i;
   float arg;
   float *p;

   arg = M_2PI / (leng - 1);
   for (p = w, i = 0; i < leng; i++)
      *p++ = 0.54 - 0.46 * cos(i * arg);

   return (w);
}


/************************************************
   Hanning window

       float  *hanning(w, leng)
       float  *w   : window values
       int     leng : window length
************************************************/

static float *hanning(float *w, const int leng)
{
   int i;
   float arg;
   float *p;

   arg = M_2PI / (leng - 1);
   for (p = w, i = 0; i < leng; i++)
      *p++ = 0.5 * (1 - cos(i * arg));

   return (w);
}


/************************************************
   Bartlett window

       float  *bartlett(w, leng)
       float  *w   : window values
       int     leng : window length
************************************************/

static float *bartlett(float *w, const int leng)
{
   int k, m;
   float *p, slope;

   m = leng / 2;
   slope = 2.0 / (float) (leng - 1);

   for (k = 0, p = w; k < m; k++)
      *p++ = slope * k;
   for (; k < leng; k++)
      *p++ = 2.0 - slope * k;

   return (w);
}


/************************************************
   trapezoid window

       float  *trapezoid(w, leng)
       float  *w   : window values
       int     leng : window length
************************************************/

static float *trapezoid(float *w, const int leng)
{
   int k, m1, m2;
   float *p, slope;

   m1 = leng / 4;
   m2 = (leng * 3) / 4;
   slope = 4.0 / (float) (leng - 1);

   for (k = 0, p = w; k < m1; k++)
      *p++ = slope * k;
   for (; k < m2; k++)
      *p++ = 1.0;
   for (; k < leng; k++)
      *p++ = 4.0 - slope * k;

   return (w);
}


/************************************************
   rectangular window

       float  *rectangular(w, leng)
       float  *w   : window values
       int     leng : window length
************************************************/

static float *rectangular(float *w, const int leng)
{
   int k;
   float *p;

   for (k = 0, p = w; k < leng; k++)
      *p++ = 1.0;

   return (w);
}

static float *pwindow = NULL;

float init_window(int type, const int size, const int nflg)
{
   int i;
   static float g;
   static Window ptype = (Window) - 1;
   static int psize = -1, pnflg = -1;

   if ((type != ptype) || (size != psize) || (nflg != pnflg)) {
      if (size > psize) {
         if (pwindow != NULL)
            free(pwindow);
         pwindow = fgetmem(size);
      }
      switch (type) {
      case BLACKMAN:
         blackman(pwindow, size);
         break;
      case HAMMING:
         hamming(pwindow, size);
         break;
      case HANNING:
         hanning(pwindow, size);
         break;
      case BARTLETT:
         bartlett(pwindow, size);
         break;
      case TRAPEZOID:
         trapezoid(pwindow, size);
         break;
      case RECTANGULAR:
         rectangular(pwindow, size);
         break;
      default:
         printf("window : Unknown window type %d!\n", (int) type);
         exit(1);
      }

      switch (nflg) {
      case 1:
         for (i = 0, g = 0.0; i < size; i++)
            g += pwindow[i] * pwindow[i];
         g = sqrt(g);
         for (i = 0; i < size; i++)
            pwindow[i] /= g;
         break;
      case 2:
         for (i = 0, g = 0.0; i < size; i++)
            g += pwindow[i];
         for (i = 0; i < size; i++)
            pwindow[i] /= g;
         break;
      case 0:
      default:
         g = 1.0;
      }

      ptype = type;
      psize = size;
      pnflg = nflg;
   }

   return (g);
}

float window(int type, float *x, const int size, const int nflg)
{
   int i;
   
   init_window(type, size, nflg);

   for (i = 0; i < size; i++)
      x[i] = x[i] * pwindow[i];

   return (0);
}
