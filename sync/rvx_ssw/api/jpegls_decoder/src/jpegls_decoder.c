#include <stdio.h>
#include "ervp_printf.h"
#include "jpegls_decoder.h"

#define JLS_DEC_DBG_ON  (0)
int cur_y, cur_c;

static sJLSDecParam jd;
static int J[32] = { 0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,5,5,6,6,7,7,8,9,10,11,12,13,14,15 };
static int zeroLUT[256];

ErvpImage *jpeglsdec_convert_image(const ErvpImage *before, ErvpImage *after)
{
	int img_type;
	int width = before->width;
	int height = before->height;
	int stride;
	int img_w;
	int img_h;

	if(before == NULL)
	{
		printf("[%s] input image is null\n", __func__);
		return 0;
	}
	else if(before->format != IMAGE_FMT_JPEGLS)
	{
		printf("[%s] input image format is not JPEGLS\n", __func__);
		return 0;
	}

	get_jpeg_info(before->addr[0], &img_w, &img_h, &img_type);
    if(width != img_w || height != img_h) {
        printf("bad jpeg header. %d %d\n", img_w, img_h);
    }

	if(img_type) // yuv422
	{
		printf("after image type yuv422\n");
		after = image_alloc(width, height, IMAGE_FMT_YUV_422_PACKED);
	}
	else	// rgb
	{
		printf("after image type argb\n");
		after = image_alloc(width, height, IMAGE_FMT_XBGR_8888_PACKED);
	}
	printf("w:%d h:%d s:%d\n", after->width, after->height, after->stride[0]);

	memset(after->addr[0], 0, after->height*after->stride[0]);
	my_jls_dec(before->addr[0], before->file_size, after->addr[0], after->stride[0]);

	return after;
}

// type : 0-rgb, 1-yuyv
void my_jls_dec(uint8_t *strm, int strm_size, uint8_t *img, int img_stride)
{
	int c, y, w, step, offset;
	uint8_t *pl, *cl, pRc;

	my_jls_dec_init(strm, strm_size, img_stride);

	my_jls_dec_parse_header();

	createzeroLUT(); //

	my_jls_bitinit();

	for(y=0; y<jd.h; y++) {
		

		if(JLS_DEC_DBG_ON) cur_y = y+1;
		if(JLS_DEC_DBG_ON) printf("n(%d)\n", cur_y);

		for(c=0; c<3; c++) {

			if(jd.type == 0) w = jd.w;
			else {
				if(c == 0) w = jd.w;
				else       w = jd.w / 2;
			}
			my_jls_dec_get_pointer(img, y, c, jd.type, jd.w, jd.stride, &pl, &cl, &step, &offset, &pRc);

			if(JLS_DEC_DBG_ON) cur_c = c;

			my_jls_decode_line(pl, cl, w, c, y, step, offset, pRc);
		}
	}
}

int my_jls_decode_line(uint8_t *psl, uint8_t *csl, int w, int color, int y, int step, int offset, uint8_t pRc)
{
	int i, psfix;
	uint8_t Ra, Rb, Rc, Rd;
	int SIGN;
	int cont;

	if(JLS_DEC_DBG_ON) printf("lossless_undoscanline()  no(%d), color(%d) cur_y(%d)\n", w, color, cur_y);

	psfix = 0;

	if(y == 0) {
		Rb = Ra = 0;
	}
	else if(y == 1) {
		Rb = psl[offset];
		Ra = Rb;
	}
	else {
		Rb = psl[offset];
		Ra = Rb;
	}
	Rc = pRc;
	
	i = 0;

	do {
		uint8_t Px;

		if(y == 0) Rd = 0;
		else {
			if(i < w-1)  Rd = psl[(i+1)*step + offset];
			else         Rd = psl[(i)*step + offset]; 
		}

		// Quantize the gradient 
		cont =  jd.vLUT[0][Rd - Rb + 256] + jd.vLUT[1][Rb - Rc + 256] + jd.vLUT[2][Rc - Ra + 256];

		if(JLS_DEC_DBG_ON) printf("i(%d) c(%d) Ra(%d) Rb(%d) Rc(%d) Rd(%d)  cont(%d) cur_y(%d) reg(%08x)\n", i+1, color, Ra, Rb, Rc, Rd, cont, cur_y, jd.s_reg);

		if (cont == 0 ) {
			int n, m;

			m = n = my_jls_decode_process_run(w-i, color); 

			if(JLS_DEC_DBG_ON) printf("process_run_dec() runlen(%d)\n", n);

			if ( m > 0 )  {  

				do {
					csl[i*step + offset] = Ra;
					i++;
				} while(--n > 0);

				if (i >= w) // end of line
					return 0;

				// update context pixels
				if(y == 0) {
					Rb = 0;
					Rd = 0;

				}
				else {
					Rb = psl[i*step + offset];
					Rd = psl[(i+1)*step + offset];
				}
			}

			// Do end of run encoding for LOSSLESS images
			Ra = my_jls_decode_end_of_run(Ra, Rb, (Ra==Rb));
			if(JLS_DEC_DBG_ON) printf("lossless_end_of_run_d() Ra(%d)\n", Ra);
		}  // Run state block 
		else {
			// Regular Context
			Px = my_jls_decode_predict(Rb, Ra, Rc);

			// map symmetric contexts 
			cont = jd.classmap[cont];

			if (cont < 0) {
				SIGN = -1;
				cont = -cont;
			}
			else
				SIGN = +1;

			// decode a Rice code of a given context 
			Ra = my_jls_decode_regular(cont, SIGN, Px);
			if(JLS_DEC_DBG_ON) printf("lossless_regular_mode_d() Ra(%d)\n", Ra);

		}

		//csl[i] = Ra;
		csl[i*step + offset] = Ra;
		Rc = Rb;
		Rb = Rd;
		++i;
	//} while (i <= w);
	} while (i < w);

	return 0;
}

int my_jls_decode_process_run(int lineleft, int color)
{
	int runlen = 0;

	if(JLS_DEC_DBG_ON) printf("process_run_dec() lineleft(%d), color(%d)\n", lineleft, color);

	do {
		int temp, hits;

		temp = zeroLUT[(uint8_t)(~(jd.s_reg >> 24))];   // number of leading ones in the input stream, up to 8
		for(hits=1; hits<=temp; hits++) {
			runlen += jd.melcorder[color];
			if ( runlen >= lineleft ) { // reached end-of-line 
				if ( runlen==lineleft && jd.melcstate[color] < MELCSTATES ) {
					jd.melclen[color] = J[++jd.melcstate[color]];
					jd.melcorder[color] = (1L<<jd.melclen[color]);
				}
				FILLBUFFER(hits); // actual # of 1's consumed 
				return lineleft; 
			}
			if ( jd.melcstate[color] < MELCSTATES ) {
				jd.melclen[color] = J[++jd.melcstate[color]];
				jd.melcorder[color] = (1L<<jd.melclen[color]);
			}
		}
		if(temp != 8) {
			FILLBUFFER(temp + 1);  // consume the leading  0 of the remainder encoding 
			break;
		}
		FILLBUFFER(8);
	} while ( 1 );

	// read the length of the remainder 
	if ( jd.melclen[color] ) {
		int temp;
#if (USE_INLINE_FUNC)
		GETBITS(temp, jd.melclen[color]);  // GETBITS is a macro, not a function 
#else
		temp = GETBITS(jd.melclen[color]);
#endif
		runlen += temp;
	}
	jd.limit_reduce = jd.melclen[color]+1;

	// adjust melcoder parameters 
	if ( jd.melcstate[color] ) {
		jd.melclen[color] = J[--jd.melcstate[color]];
		jd.melcorder[color] = (1L<<jd.melclen[color]);
	}

	return runlen;
}

int my_jls_decode_end_of_run(uint8_t Ra, uint8_t Rb, int RItype)
{
	int Ix,	Errval,	absErrval, MErrval, k, Q, oldmap, Nt, At;
	int eor_limit;
	int limit = 4*8 -8 - 1; // limit for unary part of Golomb code
	

	if(JLS_DEC_DBG_ON) printf("lossless_end_of_run_d()  Ra(%d), Rb(%d), RItype(%d)\n", Ra, Rb, RItype);


	Q = EOR_0 + RItype;
	Nt = jd.N[Q]; 
	At = jd.A[Q];

	if( RItype )
		At += Nt/2;

	// Estimate k 
	for(k=0; Nt < At; Nt *=2, k++);

	// read and decode the Golomb code Get the number of leading zeros 
	MErrval = 0;
	do {
		int temp;

		temp = zeroLUT[jd.s_reg >> 24];
		MErrval += temp;
		if (temp != 8) {
			FILLBUFFER(temp + 1);
			break;
		}
		FILLBUFFER(8);
	} while (1);

	eor_limit = limit - jd.limit_reduce;

	if ( MErrval < eor_limit ) {
		// now add the binary part of the Golomb code 
		if (k) {
			uint32_t temp;
			MErrval <<= k;
#if (USE_INLINE_FUNC)
			GETBITS(temp,k);
#else
			temp = GETBITS(k);
#endif
			MErrval += temp;
		}
	}
	else {
		// the original unary would have been too long:	(mapped value)-1 was sent verbatim 
#if (USE_INLINE_FUNC)
		GETBITS(MErrval, 8);
#else
		MErrval = GETBITS(8);
#endif
		MErrval ++;
	}

	oldmap = ( k==0 && (RItype||MErrval) && (2*jd.B[Q]<Nt));
	// Note: the Boolean variable 'oldmap' is not identical to the variable 'map' in the
	//	JPEG-LS draft. We have	oldmap = (qdiff<0) ? (1-map) : map;
	MErrval += ( RItype + oldmap );

	if ( MErrval & 1 ) { // negative 
		Errval = oldmap - (MErrval+1)/2;
		absErrval = -Errval-RItype;
		jd.B[Q]++;
	}
	else { // nonnegative 
		Errval = MErrval/2;
		absErrval = Errval-RItype;
	}

	if ( Rb < Ra )
		Ix = ( Rb - Errval ) & (256-1);
	else
		Ix = ( Rb + Errval ) & (256-1);


	// update stats 
	jd.A[Q] += absErrval;
	if (jd.N[Q] == jd.RESET) {
		jd.N[Q] >>= 1;
		jd.A[Q] >>= 1;
		jd.B[Q] >>= 1;
	}

	jd.N[Q]++;  // for next pixel 

	return Ix;			
}

uint8_t my_jls_decode_predict(uint8_t Ra, uint8_t Rb, uint8_t Rc)
{
	uint8_t minx, maxx, Px;

	if (Rb > Ra) {
		minx = Ra;
		maxx = Rb;
	} 
	else {
		maxx = Ra;
		minx = Rb;
	}

	if     (Rc >= maxx)	Px = minx;
	else if(Rc <= minx)	Px = maxx;
	else			    Px = Ra + Rb - Rc;

	return Px;
}

int clip(int x, int alpha)
{
	int highmask = -256;

	if(x & highmask) {
		if(x < 0)
			x = 0;
		else
			x = alpha - 1;
	}

	return x;
}

int my_jls_decode_regular(int Q, int SIGN, int Px)
{
	int At, Bt, Nt, Errval, absErrval;
	int current, k;
	int limit = 4*8 -8 - 1; // limit for unary part of Golomb code


	if(JLS_DEC_DBG_ON) printf("lossless_regular_mode_d()  Q(%d), SIGN(%d), Px(%d)\n", Q, SIGN, Px);
	if(JLS_DEC_DBG_ON) printf("  NQ(%d) AQ(%d) BQ(%d) CQ(%d)\n", jd.N[Q], jd.A[Q], jd.B[Q], jd.C[Q]);


	// This function is called only for regular contexts. 
	 //  End_of_run context is treated separately 

	Nt = jd.N[Q];
	At = jd.A[Q];
	{
		// Estimate k 
	    int nst = Nt;
	    for(k=0; nst < At; nst *=2, k++);
	}

	// Get the number of leading zeros 
	absErrval = 0;
	do {
		int temp;

		temp = zeroLUT[jd.s_reg >> 24];
		absErrval += temp;
		if (temp != 8) {
			FILLBUFFER(temp + 1);
			break;
		}
		FILLBUFFER(8);
	} while (1);

	if ( absErrval < limit ) {
		// now add the binary part of the Rice code 
		if (k) {
			register unsigned long temp;
			absErrval <<= k;
#if (USE_INLINE_FUNC)
			GETBITS(temp,k);
#else
			temp = GETBITS(k);
#endif
			absErrval += temp;
		}
	}
	else {
	    // the original unary would have been too long:  (mapped value)-1 was sent verbatim 
#if (USE_INLINE_FUNC)
		GETBITS(absErrval, 8);
#else
		absErrval = GETBITS(8);
#endif
		absErrval ++;
	}

	// Do the Rice mapping 
	if ( absErrval & 1 ) { // negative 
		absErrval = (absErrval + 1) / 2;
		Errval = -absErrval;
	} else {
		absErrval /= 2;
		Errval = absErrval;
	}

	Bt = jd.B[Q];

	if ( k==0 && (2*Bt <= -Nt) ) {
		// special case: see encoder side 
		Errval = -(Errval+1);
		absErrval = (Errval<0)? (-Errval):Errval;
	}

	// center, clip if necessary, and mask final error 
	if ( SIGN == -1 ) {
		Px -= jd.C[Q];
		Px = clip(Px, 256);

		// this is valid if alpha is a power of 2 
		current = (Px - Errval)&(256-1);
	}
	else {
		Px += jd.C[Q];
		Px = clip(Px,256);
		// valid if alpha is a power of 2 
		current = (Px + Errval)&(256-1);
	}

	// update bias stats 
	jd.B[Q] = (Bt += Errval);

	// update Golomb-Rice stats 
	jd.A[Q] += absErrval;

	// check reset (joint for Rice-Golomb and bias cancelation) 
	if(Nt == jd.RESET) {
		jd.N[Q] = (Nt >>= 1);
		jd.A[Q] >>= 1;
		jd.B[Q] = (Bt >>= 1);
	}


	// Do bias estimation for NEXT pixel 
	jd.N[Q] = (++Nt);
	if  ( Bt <= -Nt ) {
		if (jd.C[Q] > MIN_C)
			--jd.C[Q];

		Bt = (jd.B[Q] += Nt);

		if ( Bt <= -Nt ) 
			jd.B[Q] = -Nt+1;
	} 
	else if ( Bt > 0 ) {
		if (jd.C[Q] < MAX_C)
			++jd.C[Q];

		Bt = (jd.B[Q] -= Nt);

		if ( Bt > 0 )
			jd.B[Q] = 0;
	}

	return current;
}

int my_jls_dec_init(uint8_t *strm, int strm_size, int img_stride)
{
	int i, j, idx;
	int q1, q2, q3, n1, n2, n3, ineg, sgn;
	int initabstat, slack;


	memset(&jd, 0, sizeof(sJLSDecParam));

	jd.stride   = img_stride;

	jd.s_buf    = strm;  
	jd.s_pos    = 0;    
	jd.s_length = strm_size;
	jd.s_bits   = 0;
	jd.s_reg    = 0;

	// fixed value
	jd.T1 = 3; 
	jd.T2 = 7;
	jd.T3 = 21;
	jd.RESET = 64;


	// LUT
	for(i=-255; i<256; i++) {
		if     (i <= -jd.T3) idx = 7; // .... -T3
		else if(i <= -jd.T2) idx = 5; // -(T3-1) ... -T2
		else if(i <= -jd.T1) idx = 3; // -(T2-1) ... -T1
		else if(i <=     -1) idx = 1; // -(T1-1) ... -1
		else if(i ==      0) idx = 0; // just 0
		else if(i <   jd.T1) idx = 2; // 1 ... T1-1
		else if(i <   jd.T2) idx = 4; // T1 ... T2-1
		else if(i <   jd.T3) idx = 6; // T2 ... T3-1
		else                 idx = 8; // T3 ... 

		jd.vLUT[0][i + 256] = 9 * 9 * idx;
		jd.vLUT[1][i + 256] = 9 * idx;
		jd.vLUT[2][i + 256] = idx;
	}

	// prepare context mapping table (symmetric context merging)
	jd.classmap[0] = 0;
	for(i=1, j=0; i<9*9*9; i++) {
		n1 = n2 = n3 = 0;

		if(jd.classmap[i])
			continue;

		q1 = i / (9 * 9); // first digit
		q2 = (i / 9) % 9; // second digit
		q3 = i % 9;       // third digit


		if((q1%2)||(q1==0 && (q2%2))||(q1==0 && q2==0 && (q3%2)))
			sgn = -1;
		else
			sgn = 1;

		// compute negative context 
		if(q1) n1 = q1 + ((q1%2) ? 1 : -1);
		if(q2) n2 = q2 + ((q2%2) ? 1 : -1);
		if(q3) n3 = q3 + ((q3%2) ? 1 : -1);

		ineg = (n1*9+n2)*9+n3;
		j++ ;    // next class number 
		jd.classmap[i]    =  sgn*j;
		jd.classmap[ineg] = -sgn*j;
	}

	// maxrun is ignored when using MELCODE, 
	for(i=0; i<3; i++) {
		jd.melcstate[i] = 0;                // index to the state array
		jd.melclen[i]   = J[0];             // contents of the state array location indexed by melcstate: the "expected" run length is 2^melclen, shorter runs are
		// encoded by a 1 followed by the run length in binary representation, wit a fixed length of melclen bits.
		jd.melcorder[i] = 1<<jd.melclen[i]; // 2^melclen
	}

	// initialize A[], B[], C[], N[]
	slack = 1 << INITABSLACK;
	initabstat = (256 + slack/2) / slack;
	if(initabstat < MIN_INITABSTAT) initabstat = MIN_INITABSTAT;

	// do the statistics initialization
	for(i=0; i<TOT_CONTEXTS; i++) {
		jd.C[i] = jd.B[i] = 0;
		jd.N[i] = INITNSTAT;
		jd.A[i] = initabstat;
	}

	return 1;
}

int my_jls_dec_parse_header(void)
{
	int ret;
	int marker;
	int pos_soi, pos_sof_ls;


	// read SOI
	pos_soi = my_jls_dec_seek_marker(&marker);
	if(pos_soi == 0) {
		printf("error : premature end of file seeking SOI\n");
		return 0;
	}
	if(marker != SOI) {
		printf("error : %04X found: first marker must be SOI\n", marker);
		return 0;
	}

	// read SOF_LS
	pos_sof_ls = my_jls_dec_seek_marker(&marker);
	if(pos_sof_ls == 0) {
		printf("error : premature end of file seeking SOF_LS\n");
		return 0;
	}
	if(marker != SOF_LS) {
		printf("error : %04X found: second marker must be SOF_LS\n", marker);
		return 0;
	}
	// read the frame header
	ret = my_jls_dec_read_jpegls_frame();
	if(ret == 0) {
		printf("error : premature end of file reading frame header\n");
		return 0;
	}


	// read SOS
	ret = my_jls_dec_seek_marker(&marker);
	if(ret == 0) {
		printf("error : premature end of file seeking SOS\n");
		return 0;
	}
	if(marker != SOS) {
		printf("error : %04X found: second marker must be SOS\n", marker);
		return 0;
	}
	// read the scan header
	ret = my_jls_dec_read_jpegls_scan();
	if(ret == 0) {
		printf("error : premature end of file reading scan header\n");
		return 0;
	}
	
	if(jd.samplingx[0] == 1) jd.type = 0; // rgb
	else                     jd.type = 1; // yuyv

	return 1;
}


int my_jls_dec_seek_marker(int *marker)
{
	int c, c2;

	while(jd.s_pos < jd.s_length-1) {
		c = jd.s_buf[jd.s_pos++];
		if(c == 0xFF) {
			c2 = jd.s_buf[jd.s_pos++];
			if(c2 & 0x80) {
				*marker = (c<<8) | c2;
				return jd.s_pos;
			}
		}
	}

	return 0;
}

uint32_t my_jls_dec_read_n_bytes(int n)
{
	int i;
	uint32_t m = 0;

	for(i=0; i<n; i++) {
		m = (m<<8) | jd.s_buf[jd.s_pos++];
	}

	return m;
}

int my_jls_dec_read_marker(int *marker)
{
	uint32_t m;

	m = my_jls_dec_read_n_bytes(2);
	if(jd.s_pos >= jd.s_length)
		return 0;
	if((m & 0xFF00) != 0xFF00) {
		printf("read_marker : expected marker, got %04X\n", m);
		return 0;
	}

	*marker = m;

	return 2;
}

int my_jls_dec_read_jpegls_frame(void)
{
	int i, marker_len, bpp, tq;
	int ct = 0;


	// read marker length
	marker_len = my_jls_dec_read_n_bytes(2);
	ct += 2;

	// read the bits per pixel
	bpp = my_jls_dec_read_n_bytes(1); // bpp must be 8
	ct += 1;

	// read the rows and columns
	jd.h = my_jls_dec_read_n_bytes(2);
	ct += 2;
	jd.w = my_jls_dec_read_n_bytes(2);
	ct += 2;

	// read component information
	jd.comp = my_jls_dec_read_n_bytes(1);
	ct += 1;

	for(i=0; i<jd.comp; i++) {
		int sx, sy, cid;

		cid = my_jls_dec_read_n_bytes(1);
		ct += 1;
		sx  = my_jls_dec_read_n_bytes(1);
		ct += 1;
		tq  = my_jls_dec_read_n_bytes(1);
		ct += 1;

		sy = sx & 0x0f;
		sx >>= 4;

		jd.samplingx[i] = sx;
		jd.samplingy[i] = sy;
		jd.comp_ids[i]  = cid;
	}

	if(marker_len != 8 + 3*jd.comp) {
		printf("error : jpeg_readls_frame\n");
		return 0;
	}

	return ct;
}

int my_jls_dec_read_jpegls_scan(void)
{
	int i, marker_len, comp, near, color_mode, shift;
	int ct = 0;


	// read marker length
	marker_len = my_jls_dec_read_n_bytes(2);
	ct += 2;

	comp = my_jls_dec_read_n_bytes(1);
	ct += 1;

	if(comp != jd.comp) {
		return 0;
	}

	for(i=0; i<comp; i++) {
		int cid, tm;
		cid = my_jls_dec_read_n_bytes(1); // component identifier
		ct += 1;
		tm  = my_jls_dec_read_n_bytes(1); // table identifier
		ct += 1;

		if(tm) {
			printf("error : read_jpegls_scan : found nonzero table identifier\n");
			return 0;
		}
	}

	near = my_jls_dec_read_n_bytes(1); // 
	ct += 1;

	color_mode = my_jls_dec_read_n_bytes(1); // 
	ct += 1;

	shift = my_jls_dec_read_n_bytes(1); // 
	ct += 1;

	// near == 0 , color_mode == 1(line), shift == 0
	if(near!=0 || color_mode != 1 || shift != 0) {
		printf("error : read_jpegls_scan : not supported mode\n");
		return 0;
	}

	return ct;
}


void my_jls_dec_copy_line(uint8_t *src, int y, int w, int c, int type, uint8_t *buf)
{
	int x;
	uint8_t *ps, *pd;
	int step;

	if(type == 0) { // rgb
		ps = src;
		pd = buf + 4*w*y + c + 1; // argb

		for(x=0; x<w; x++) {
			pd[0] = *ps++;
			pd   += 4;
		}
	}
	else { // yuyv

		ps = src;

		if     (c == 0) pd = buf + 2*w*y + 0;
		else if(c == 1) pd = buf + 4*w*y + 1;
		else if(c == 2) pd = buf + 4*w*y + 3;

		if(c == 0) step = 2;
		else       step = 4;

		for(x=0; x<w; x++) {
			pd[0] = *ps++;
			pd   += step;
		}
	}
}

void my_jls_dec_get_pointer(uint8_t *img, int y, int c, int type, int w, int stride, uint8_t **pl, uint8_t **cl, int *step, int *offset, uint8_t *pRc)
{
	*pl = img + stride*(y-1);
	*cl = img + stride*(y  );
	
	if(type == 0) { // argb
		*step = 4;
		*offset = c + 1;

	}
	else {
		if(c == 0) {
			*step = 2;
			*offset = 0;
		}
		else if(c == 1) {
			*step = 4;
			*offset = 1;
		}
		else if(c == 2) {
			*step = 4;
			*offset = 3;
		}
	}

	if(y < 2) *pRc = 0;
	else 	  *pRc = *(uint8_t *)(img + stride*(y-2) + *offset);
}


#if (USE_INLINE_FUNC == 0)
int GETBITS(int n)
{
	int x = jd.s_reg >> (32 - n);
	FILLBUFFER(n);
	return x;
}

void FILLBUFFER(int no)
{
	uint8_t x;     

	jd.s_reg <<= no;
	jd.s_bits += no;
	while (jd.s_bits >= 0) {
		//x = mygetc(in);
		x = jd.s_buf[jd.s_pos++];
		if ( x == 0xff ) {
			if ( jd.s_bits < 8 ) {
				//myungetc(0xff, in);
				jd.s_buf[--jd.s_pos] = 0xff;
				break;
			}
			else {
				//x = mygetc(in);
				x = jd.s_buf[jd.s_pos++];
				if ( !(x & 0x80) )  { // non-marker: drop 0
					jd.s_reg |= (0xff << jd.s_bits) | ((x & 0x7f)<<(jd.s_bits-7));
					jd.s_bits -= 15;
				}
				else {
					// marker: hope we know what we're doing 
					// the "1" bit following ff is NOT dropped
					jd.s_reg |= (0xff << jd.s_bits) | (x <<(jd.s_bits-8));
					jd.s_bits -= 16;
				}
				continue;
			}
		}
	
		jd.s_reg |= x << jd.s_bits;
		jd.s_bits -= 8;
	}

}
#endif

// creates the bit counting look-up table. 
void createzeroLUT(void)
{
	int i, j, k, l;

	j = k = 1; l = 8;
	for (i = 0; i < 256; ++i) {
		zeroLUT[i] = l;
		--k;
		if (k == 0) {
			k = j;
			--l;
			j *= 2;
		}
	}
}

void get_jpeg_info(uint8_t *strm, int *w, int *h, int *type)
{
	*h = ((int)strm[7]<<8) + strm[8];
	*w = ((int)strm[9]<<8) + strm[10];
	*type = (strm[13] == 0x11) ? 0 : 1;
}

void my_jls_bitinit(void)
{
	jd.s_bits = jd.s_reg = 0;
	FILLBUFFER(24);
}
