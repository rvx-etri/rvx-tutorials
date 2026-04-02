#ifndef __ERVP_JPEGLS_DECODER_H__
#define __ERVP_JPEGLS_DECODER_H__

#include "ervp_image.h"

#include <stdint.h>

#define USE_INLINE_FUNC	(0)

//  Marker identifiers 
#define	SOI		0xFFD8	// start of image 
#define EOI		0xFFD9	// end of image 
#define SOS		0xFFDA  // Start of scan
#define SOF_LS	0xFFF7	// Start of JPEG-LS regular frame 

#define EOR_CONTEXTS	2						   // Number of end-of-run contexts
#define CONTEXTS		((9*9*9+1)/2)              // all regions, with symmetric merging
#define EOR_0			(CONTEXTS)                 // index of first end-of-run context
#define TOT_CONTEXTS	(CONTEXTS +  EOR_CONTEXTS) // Total number of contexts

#define INITNSTAT		1	 // init value for N[] 
#define MIN_INITABSTAT	2    // min init value for A[] 
#define INITABSLACK		6    // init value for A is roughly 2^(bpp-INITABSLACK) but not less than above 
#define INITBIASTAT		0	 // init value for B[]

#define MAX_C			127
#define MIN_C			-128

#define MELCSTATES		32	 // number of melcode states 


#if (USE_INLINE_FUNC)

#define FILLBUFFER(no) { \
	uint8_t x;           \
	\
	jd.s_reg <<= no;     \
	jd.s_bits += no;     \
	while (jd.s_bits >= 0) {  \
	    x = jd.s_buf[jd.s_pos++]; \
	    if ( x == 0xff ) {        \
	        if ( jd.s_bits < 8 ) {    \
	            jd.s_buf[--jd.s_pos] = 0xff; \
	            break;                  \
	        }				\
			else {			\
			    x = jd.s_buf[jd.s_pos++];  \
			    if ( !(x & 0x80) )  { 	              \
			        jd.s_reg |= (0xff << jd.s_bits) | ((x & 0x7f)<<(jd.s_bits-7));\
			        jd.s_bits -= 15;		\
			    }			\
			    else {			\
			        jd.s_reg |= (0xff << jd.s_bits) | (x <<(jd.s_bits-8));\
			        jd.s_bits -= 16;		\
			    }			        \
			    continue;		    \
			}				        \
	    }				            \
	    jd.s_reg |= x << jd.s_bits; \
	    jd.s_bits -= 8;             \
	}                               \
}


#  define GETBITS(x, n)\
 { x = jd.s_reg >> (32 - (n));\
 FILLBUFFER(n);\
 }

#else
int GETBITS(int n);
void FILLBUFFER(int no);
#endif

#pragma pack(push)  // save the original data alignment
#pragma pack(1)     // Set data alignment to 1 byte boundary

typedef struct {
	// stream
	uint8_t *s_buf;    // stream buf;
	int32_t  s_pos;    // stream position. byte unity
	int32_t  s_length; // stream max size
	int32_t  s_bits;
	uint32_t s_reg;

	// basic info
	int32_t  n_w[3]; // 각 컴포넌트별 크기
	int32_t  w;
	int32_t  h;
	int32_t  comp;
	int32_t  comp_ids[3];

	uint8_t samplingx[3];
	uint8_t samplingy[3];

	int      type;
	int      stride;

	int32_t  T1, T2, T3;

	int32_t  vLUT[3][2 * 256];
	int32_t  classmap[9*9*9];
	uint32_t melcorder[3];
	int32_t  melcstate[3];
	int32_t  melclen[3];
	int32_t  limit_reduce;
	int32_t  N[TOT_CONTEXTS];
	int32_t  A[TOT_CONTEXTS];
	int32_t  B[TOT_CONTEXTS];
	int32_t  C[TOT_CONTEXTS];
	int32_t  RESET;
} sJLSDecParam;

#pragma pack(pop)  // restore the previous pack setting

// functions
void my_jls_dec(uint8_t *strm, int strm_size, uint8_t *img, int img_stride);
int my_jls_decode_line(uint8_t *psl, uint8_t *csl, int w, int color, int y, int step, int offset, uint8_t pRc);
int my_jls_decode_process_run(int lineleft, int color);
int my_jls_decode_end_of_run(uint8_t Ra, uint8_t Rb, int RItype);
uint8_t my_jls_decode_predict(uint8_t Ra, uint8_t Rb, uint8_t Rc);
int clip(int x, int alpha);
int my_jls_decode_regular(int Q, int SIGN, int Px);
int my_jls_dec_init(uint8_t *strm, int strm_size, int img_stride);
int my_jls_dec_parse_header(void);
int my_jls_dec_seek_marker(int *marker);
uint32_t my_jls_dec_read_n_bytes(int n);
int my_jls_dec_read_marker(int *marker);
int my_jls_dec_read_jpegls_frame(void);
int my_jls_dec_read_jpegls_scan(void);
void my_jls_dec_copy_line(uint8_t *src, int y, int w, int c, int type, uint8_t *buf);
void my_jls_dec_get_pointer(uint8_t *img, int y, int c, int type, int w, int stride, uint8_t **pl, uint8_t **cl, int *step, int *offset, uint8_t *pRc);
void createzeroLUT(void);
void get_jpeg_info(uint8_t *strm, int *w, int *h, int *type);
void my_jls_bitinit(void);

ErvpImage *jpeglsdec_convert_image(const ErvpImage *before, ErvpImage *after);

#endif
