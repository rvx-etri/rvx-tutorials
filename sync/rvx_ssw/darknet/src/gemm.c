#include "platform_info.h"
#include "ervp_malloc.h"
#include "ervp_printf.h"

#include "gemm.h"
#include "utils.h"
#include "im2col.h"
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdint.h>

#include "ervp_matrix.h"

#ifdef __cplusplus
#define PUT_IN_REGISTER
#else
#define PUT_IN_REGISTER register
#endif

void gemm_bin(int M, int N, int K, float ALPHA,
        char  *A, int lda,
        float *B, int ldb,
        float *C, int ldc)
{
    int i,j,k;
    for(i = 0; i < M; ++i){
        for(k = 0; k < K; ++k){
            char A_PART = A[i*lda+k];
            if(A_PART){
                for(j = 0; j < N; ++j){
                    C[i*ldc+j] += B[k*ldb+j];
                }
            } else {
                for(j = 0; j < N; ++j){
                    C[i*ldc+j] -= B[k*ldb+j];
                }
            }
        }
    }
}

float *random_matrix(int rows, int cols)
{
    int i;
    float* m = (float*)calloc(rows * cols, sizeof(float));
    for(i = 0; i < rows*cols; ++i){
        m[i] = (float)rand()/32767;
    }
    return m;
}

void time_random_matrix(int TA, int TB, int m, int k, int n)
{
    float *a;
    if(!TA) a = random_matrix(m,k);
    else a = random_matrix(k,m);
    int lda = (!TA)?k:m;
    float *b;
    if(!TB) b = random_matrix(k,n);
    else b = random_matrix(n,k);
    int ldb = (!TB)?n:k;

    float *c = random_matrix(m,n);
    int i;
    clock_t start = clock(), end;
    for(i = 0; i<10; ++i){
        gemm_cpu(TA,TB,m,n,k,1,a,lda,b,ldb,1,c,n);
    }
    end = clock();
    printf("Matrix Multiplication %dx%d * %dx%d, TA=%d, TB=%d: %lf ms\n",m,k,k,n, TA, TB, (float)(end-start)/CLOCKS_PER_SEC);
    free(a);
    free(b);
    free(c);
}


void gemm(int TA, int TB, int M, int N, int K, float ALPHA,
        float *A, int lda,
        float *B, int ldb,
        float BETA,
        float *C, int ldc)
{
    gemm_cpu( TA,  TB,  M, N, K, ALPHA,A,lda, B, ldb,BETA,C,ldc);
}


//--------------------------------------------
// XNOR bitwise GEMM for binary neural network
//--------------------------------------------


static inline unsigned char xnor(unsigned char a, unsigned char b) {
    //return a == b;
    return !(a^b);
}

// INT-32
static inline uint32_t get_bit_int32(uint32_t const*const src, size_t index) {
    size_t src_i = index / 32;
    int src_shift = index % 32;
    unsigned char val = (src[src_i] & (1 << src_shift)) > 0;
    return val;
}

static inline uint32_t xnor_int32(uint32_t a, uint32_t b) {
    return ~(a^b);
}

static inline uint64_t xnor_int64(uint64_t a, uint64_t b) {
    return ~(a^b);
}


static inline uint32_t fill_bit_int32(char src) {
    if (src == 0) return 0x00000000;
    else return  0xFFFFFFFF;
}

static inline uint64_t fill_bit_int64(char src) {
    if (src == 0) return 0x0000000000000000;
    else return  0xFFFFFFFFFFFFFFFF;
}

void binary_int32_printf(uint32_t src) {
    int i;
    for (i = 0; i < 32; ++i) {
        if (src & 1) printf("1");
        else printf("0");
        src = src >> 1;
    }
    printf("\n");
}

void binary_int64_printf(uint64_t src) {
    int i;
    for (i = 0; i < 64; ++i) {
        if (src & 1) printf("1");
        else printf("0");
        src = src >> 1;
    }
    printf("\n");
}

/*
void gemm_nn_custom_bin_mean(int M, int N, int K, float ALPHA_UNUSED,
    unsigned char *A, int lda,
    unsigned char *B, int ldb,
    float *C, int ldc, float *mean_arr)
{
    int *count_arr = calloc(M*N, sizeof(int));

    int i, j, k;
    for (i = 0; i < M; ++i) {   // l.n - filters [16 - 55 - 1024]
        for (k = 0; k < K; ++k) {   // l.size*l.size*l.c - one filter size [27 - 9216]
            char a_bit = get_bit(A, i*lda + k);

            for (j = 0; j < N; ++j) { // out_h*out_w - one channel output size [169 - 173056]
                char b_bit = get_bit(B, k*ldb + j);
                count_arr[i*ldc + j] += xnor(a_bit, b_bit);
            }
        }
    }

    for (i = 0; i < M; ++i) {
        float mean_val = mean_arr[i];
        for (j = 0; j < N; ++j) {
            C[i*ldc + j] = (2 * count_arr[i*ldc + j] - K) * mean_val;
        }
    }
    free(count_arr);
}
*/

/*
void gemm_nn_custom_bin_mean_transposed(int M, int N, int K, float ALPHA_UNUSED,
    unsigned char *A, int lda,
    unsigned char *B, int ldb,
    float *C, int ldc, float *mean_arr)
{
    int *count_arr = calloc(M*N, sizeof(int));

    int i, j, k;
    for (i = 0; i < M; ++i) {   // l.n - filters [16 - 55 - 1024]
        for (j = 0; j < N; ++j) { // out_h*out_w - one channel output size [169 - 173056]
            for (k = 0; k < K; ++k) {   // l.size*l.size*l.c - one filter size [27 - 9216]
                char a_bit = get_bit(A, i*lda + k);
                char b_bit = get_bit(B, j*ldb + k);
                count_arr[i*ldc + j] += xnor(a_bit, b_bit);
            }
        }
    }

    for (i = 0; i < M; ++i) {
        float mean_val = mean_arr[i];
        for (j = 0; j < N; ++j) {
            C[i*ldc + j] = (2 * count_arr[i*ldc + j] - K) * mean_val;
        }
    }
    free(count_arr);
}
*/

/*
void gemm_nn_custom_bin_mean(int M, int N, int K, float ALPHA_UNUSED,
    unsigned char *A, int lda,
    unsigned char *B, int ldb,
    float *C, int ldc, float *mean_arr)
{
    int *count_arr = calloc(M*N, sizeof(int));

    int i;

#pragma omp parallel for
    for (i = 0; i < M; ++i) {   // l.n - filters [16 - 55 - 1024]
        int j, k, h;
        for (k = 0; k < K; ++k) {   // l.size*l.size*l.c - one filter size [27 - 9216]
            const char a_bit = get_bit(A, i*lda + k);
            uint64_t a_bit64 = fill_bit_int64(a_bit);
            int  k_ldb = k*ldb;

            for (j = 0; j < N; j += 64) { // out_h*out_w - one channel output size [169 - 173056]
                if ((N - j > 64) && (k_ldb % 8 == 0)) {
                    uint64_t b_bit64 = *((uint64_t *)(B + (k_ldb + j) / 8));
                    uint64_t c_bit64 = xnor_int64(a_bit64, b_bit64);
                    //printf("\n %d \n",__builtin_popcountll(c_bit64)); // gcc
                    printf("\n %d \n", __popcnt64(c_bit64));    // msvs

                    int h;
                    for (h = 0; h < 64; ++h)
                        if ((c_bit64 >> h) & 1) count_arr[i*ldc + j + h] += 1;

                    //binary_int64_printf(a_bit64);
                    //binary_int64_printf(b_bit64);
                    //binary_int64_printf(c_bit64);
                }
                else {
                    for (; j < N; ++j) { // out_h*out_w - one channel output size [169 - 173056]
                        char b_bit = get_bit(B, k_ldb + j);
                        if (xnor(a_bit, b_bit)) count_arr[i*ldc + j] += 1;
                    }
                }

            }
        }
    }

    if (mean_arr) {
        //int K_2 = K / 2;
        for (i = 0; i < M; ++i) {
            float mean_val = mean_arr[i];
            //float mean_val2 = 2 * mean_val;
            for (j = 0; j < N; ++j) {
                C[i*ldc + j] = (2 * count_arr[i*ldc + j] - K) * mean_val;
                //C[i*ldc + j] = (count_arr[i*ldc + j] - K_2) *mean_val2;
            }
        }
    }
    else {
        for (i = 0; i < M; ++i) {
            for (j = 0; j < N; ++j) {
                C[i*ldc + j] = count_arr[i*ldc + j] - K / 2;
            }
        }
    }

    free(count_arr);

    //getchar();
}
*/


/*
void gemm_nn_custom_bin_mean_transposed(int M, int N, int K, float ALPHA_UNUSED,
    unsigned char *A, int lda,
    unsigned char *B, int ldb,
    float *C, int ldc, float *mean_arr)
{
    int i;

#pragma omp parallel for
    for (i = 0; i < M; ++i) {   // l.n - filters [16 - 55 - 1024]
        int j, k, h;
        float mean_val = mean_arr[i];

        for (j = 0; j < N; ++j) { // out_h*out_w - one channel output size [169 - 173056]
            int count = 0;

            for (k = 0; k < K; k += 64) {   // l.size*l.size*l.c - one filter size [27 - 9216]
                uint64_t a_bit64 = *((uint64_t *)(A + (i*lda + k) / 8));
                uint64_t b_bit64 = *((uint64_t *)(B + (j*ldb + k) / 8));
                uint64_t c_bit64 = xnor_int64(a_bit64, b_bit64);

#ifdef WIN32
                int tmp_count = __popcnt64(c_bit64);
#else
                int tmp_count = __builtin_popcountll(c_bit64);
#endif

                if (K - k < 64)  tmp_count = tmp_count - (64 - (K - k));    // remove extra bits
                count += tmp_count;
                //binary_int64_printf(c_bit64);
                //printf(", count = %d \n\n", tmp_count);
            }

            C[i*ldc + j] = (2 * count - K) * mean_val;
        }
    }
}
*/

//----------------------------

// is not used
void transpose_32x32_bits_my(uint32_t *A, uint32_t *B, int lda, int ldb)
{
    unsigned int x, y;
    for (y = 0; y < 32; ++y) {
        for (x = 0; x < 32; ++x) {
            if (A[y * lda] & (1 << x)) B[x * ldb] |= (uint32_t)1 << y;
        }
    }
}

uint8_t reverse_8_bit(uint8_t a) {
    return ((a * 0x0802LU & 0x22110LU) | (a * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
}

uint32_t reverse_32_bit(uint32_t a)
{
    // unsigned int __rbit(unsigned int val) // for ARM    //__asm__("rbit %0, %1\n" : "=r"(output) : "r"(input));
    return (reverse_8_bit(a >> 24) << 0) |
        (reverse_8_bit(a >> 16) << 8) |
        (reverse_8_bit(a >> 8) << 16) |
        (reverse_8_bit(a >> 0) << 24);
}

#define swap(a0, a1, j, m) t = (a0 ^ (a1 >>j)) & m; a0 = a0 ^ t; a1 = a1 ^ (t << j);

void transpose32_optimized(uint32_t A[32]) {
    int j, k;
    unsigned m, t;

    //m = 0x0000FFFF;
    //for (j = 16; j != 0; j = j >> 1, m = m ^ (m << j)) {
    //    for (k = 0; k < 32; k = (k + j + 1) & ~j) {
    //        t = (A[k] ^ (A[k + j] >> j)) & m;
    //        A[k] = A[k] ^ t;
    //        A[k + j] = A[k + j] ^ (t << j);
    //    }
    //}

    j = 16;
    m = 0x0000FFFF;
    for (k = 0; k < 32; k = (k + j + 1) & ~j) { swap(A[k], A[k + j], j, m); }

    j = 8;
    m = 0x00ff00ff;
    for (k = 0; k < 32; k = (k + j + 1) & ~j) { swap(A[k], A[k + j], j, m); }

    j = 4;
    m = 0x0f0f0f0f;
    for (k = 0; k < 32; k = (k + j + 1) & ~j) { swap(A[k], A[k + j], j, m); }

    j = 2;
    m = 0x33333333;
    for (k = 0; k < 32; k = (k + j + 1) & ~j) { swap(A[k], A[k + j], j, m); }

    j = 1;
    m = 0x55555555;
    for (k = 0; k < 32; k = (k + j + 1) & ~j) { swap(A[k], A[k + j], j, m); }

    // reverse Y
    for (j = 0; j < 16; ++j) {
        uint32_t tmp = A[j];
        A[j] = reverse_32_bit(A[31 - j]);
        A[31 - j] = reverse_32_bit(tmp);
    }
}

void transpose_32x32_bits_reversed_diagonale(uint32_t *A, uint32_t *B, int m, int n)
{
    unsigned A_tmp[32];
    int i;
    #pragma unroll
    for (i = 0; i < 32; ++i) A_tmp[i] = A[i * m];
    transpose32_optimized(A_tmp);
    #pragma unroll
    for (i = 0; i < 32; ++i) B[i*n] = A_tmp[i];
}


void transpose_8x8_bits_my(unsigned char *A, unsigned char *B, int lda, int ldb)
{
    unsigned x, y;
    for (y = 0; y < 8; ++y) {
        for (x = 0; x < 8; ++x) {
            if (A[y * lda] & (1 << x)) B[x * ldb] |= 1 << y;
        }
    }
}

unsigned char reverse_byte_1(char a)
{
    return ((a & 0x1) << 7) | ((a & 0x2) << 5) |
        ((a & 0x4) << 3) | ((a & 0x8) << 1) |
        ((a & 0x10) >> 1) | ((a & 0x20) >> 3) |
        ((a & 0x40) >> 5) | ((a & 0x80) >> 7);
}

unsigned char reverse_byte(unsigned char a)
{
    return ((a * 0x0802LU & 0x22110LU) | (a * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
}

static unsigned char lookup[16] = {
    0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
    0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };

unsigned char reverse_byte_3(unsigned char n) {
    // Reverse the top and bottom nibble then swap them.
    return (lookup[n & 0b1111] << 4) | lookup[n >> 4];
}


void transpose8rS32_reversed_diagonale(unsigned char* A, unsigned char* B, int m, int n)
{
    unsigned x, y, t;

    x = y = 0;
    // Load the array and pack it into x and y.
    //x = (A[0] << 24) | (A[m] << 16) | (A[2 * m] << 8) | A[3 * m];
    //y = (A[4 * m] << 24) | (A[5 * m] << 16) | (A[6 * m] << 8) | A[7 * m];

    t = (x ^ (x >> 7)) & 0x00AA00AA;  x = x ^ t ^ (t << 7);
    t = (y ^ (y >> 7)) & 0x00AA00AA;  y = y ^ t ^ (t << 7);

    t = (x ^ (x >> 14)) & 0x0000CCCC;  x = x ^ t ^ (t << 14);
    t = (y ^ (y >> 14)) & 0x0000CCCC;  y = y ^ t ^ (t << 14);

    t = (x & 0xF0F0F0F0) | ((y >> 4) & 0x0F0F0F0F);
    y = ((x << 4) & 0xF0F0F0F0) | (y & 0x0F0F0F0F);
    x = t;

    B[7 * n] = reverse_byte(x >> 24);  B[6 * n] = reverse_byte(x >> 16);  B[5 * n] = reverse_byte(x >> 8);  B[4 * n] = reverse_byte(x);
    B[3 * n] = reverse_byte(y >> 24);  B[2 * n] = reverse_byte(y >> 16);  B[1 * n] = reverse_byte(y >> 8);  B[0 * n] = reverse_byte(y);
}

/*
// transpose by 8-bit
void transpose_bin(char *A, char *B, const int n, const int m,
    const int lda, const int ldb, const int block_size)
{
    //printf("\n n = %d, ldb = %d \t\t m = %d, lda = %d \n", n, ldb, m, lda);
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i += 8) {
        int j;
        for (j = 0; j < m; j += 8) {
            int a_index = i*lda + j;
            int b_index = j*ldb + i;
            //transpose_8x8_bits_my(&A[a_index/8], &B[b_index/8], lda/8, ldb/8);
            transpose8rS32_reversed_diagonale(&A[a_index / 8], &B[b_index / 8], lda / 8, ldb / 8);
        }
        for (; j < m; ++j) {
            if (get_bit(A, i*lda + j)) set_bit(B, j*ldb + i);
        }
    }
}
*/


// transpose by 32-bit
void transpose_bin(uint32_t *A, uint32_t *B, const int n, const int m,
    const int lda, const int ldb, const int block_size)
{
    //printf("\n n = %d (n mod 32 = %d), m = %d (m mod 32 = %d) \n", n, n % 32, m, m % 32);
    //printf("\n lda = %d (lda mod 32 = %d), ldb = %d (ldb mod 32 = %d) \n", lda, lda % 32, ldb, ldb % 32);
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i += 32) {
        int j;
        for (j = 0; j < m; j += 32) {
            int a_index = i*lda + j;
            int b_index = j*ldb + i;
            transpose_32x32_bits_reversed_diagonale(&A[a_index / 32], &B[b_index / 32], lda / 32, ldb / 32);
            //transpose_32x32_bits_my(&A[a_index/32], &B[b_index/32], lda/32, ldb/32);
        }
        for (; j < m; ++j) {
            if (get_bit((const unsigned char* const)A, i * lda + j)) set_bit((unsigned char* const)B, j * ldb + i);
        }
    }
}

static inline int popcnt_32(uint32_t val32) {
    int tmp_count = __builtin_popcount(val32);
    return tmp_count;
}
//----------------------------


int is_avx() {
    return 0;
}

int is_fma_avx2() {
    return 0;
}

void gemm_nn(int M, int N, int K, float ALPHA,
    float *A, int lda,
    float *B, int ldb,
    float *C, int ldc)
{
//#ifdef PACT_SUPPORT_FLOAT32
#if 0
      ErvpMatrixInfo matrix_a;
      ErvpMatrixInfo matrix_b;
      ErvpMatrixInfo matrix_c;

      matrix_a.datatype = MATRIX_DATATYPE_FLOAT32;
      matrix_a.addr = (void *)A;
      matrix_a.num_row = M;
      matrix_a.num_col = K;
      matrix_a.stride = sizeof(float)*lda;

      matrix_b.datatype = MATRIX_DATATYPE_FLOAT32;
      matrix_b.addr = (void *)B;
      matrix_b.num_row = K;
      matrix_b.num_col = N;
      matrix_b.stride = sizeof(float)*ldb;

      matrix_c.datatype = MATRIX_DATATYPE_FLOAT32;
      matrix_c.addr = (void *)C;
      matrix_c.num_row = M;
      matrix_c.num_col = N;
      matrix_c.stride = sizeof(float)*ldc;

      matrix_mult_opt(&matrix_a, &matrix_b, &matrix_c);
#else
    int i, j, k;
    for (i = 0; i < M; ++i) {
        for (k = 0; k < K; ++k) {
            PUT_IN_REGISTER float A_PART = ALPHA * A[i * lda + k];
            for (j = 0; j < N; ++j) {
                C[i*ldc + j] += A_PART*B[k*ldb + j];
            }
        }
    }
#endif
}

void gemm_nn_fast(int M, int N, int K, float ALPHA,
    float *A, int lda,
    float *B, int ldb,
    float *C, int ldc)
{
    int i, j, k;
    #pragma omp parallel for
    for (i = 0; i < M; ++i) {
        for (k = 0; k < K; ++k) {
            PUT_IN_REGISTER float A_PART = ALPHA*A[i*lda + k];
            for (j = 0; j < N; ++j) {
                C[i*ldc + j] += A_PART*B[k*ldb + j];
            }
        }
    }
}

void gemm_nn_bin_32bit_packed(int M, int N, int K, float ALPHA,
    uint32_t *A, int lda,
    uint32_t *B, int ldb,
    float *C, int ldc, float *mean_arr)
{
    int i;
    #pragma omp parallel for
    for (i = 0; i < M; ++i) {   // l.n
        int j, s;
        float mean_val = mean_arr[i];
        //printf(" l.mean_arr[i] = %d \n ", l.mean_arr[i]);
        for (s = 0; s < K; ++s) // l.size*l.size*l.c/32  or (l.size*l.size*l.c)
        {
            //PUT_IN_REGISTER float A_PART = 1*a[i*k + s];
            PUT_IN_REGISTER uint32_t A_PART = A[i * lda + s];
            for (j = 0; j < N; ++j) // out_h*out_w;
            {
                //c[i*n + j] += A_PART*b[s*n + j];
                PUT_IN_REGISTER uint32_t B_PART = B[s * ldb + j];
                uint32_t xnor_result = ~(A_PART ^ B_PART);
                //printf(" xnor_result = %d, ", xnor_result);
                int32_t count = popcnt_32(xnor_result);  // must be Signed int

                C[i*ldc + j] += (2 * count - 32) * mean_val;
                //c[i*n + j] += count*mean;
            }
        }
    }
}


void convolution_2d(int w, int h, int ksize, int n, int c, int pad, int stride,
    float *weights, float *input, float *output, float *mean)
{
    const int out_h = (h + 2 * pad - ksize) / stride + 1;    // output_height=input_height for stride=1 and pad=1
    const int out_w = (w + 2 * pad - ksize) / stride + 1;    // output_width=input_width for stride=1 and pad=1
    //int i, f, j;

    int fil;
    // filter index
    #pragma omp parallel for      // "omp parallel for" - automatic parallelization of loop by using OpenMP
    for (fil = 0; fil < n; ++fil) {
        int chan, y, x, f_y, f_x;
        // channel index
        for (chan = 0; chan < c; ++chan)
            // input - y
            for (y = 0; y < h; ++y)
                // input - x
                for (x = 0; x < w; ++x)
                {
                    int const output_index = fil*w*h + y*w + x;
                    int const weights_pre_index = fil*c*ksize*ksize + chan*ksize*ksize;
                    int const input_pre_index = chan*w*h;
                    float sum = 0;

                    // filter - y
                    for (f_y = 0; f_y < ksize; ++f_y)
                    {
                        int input_y = y + f_y - pad;
                        // filter - x
                        for (f_x = 0; f_x < ksize; ++f_x)
                        {
                            int input_x = x + f_x - pad;
                            if (input_y < 0 || input_x < 0 || input_y >= h || input_x >= w) continue;

                            int input_index = input_pre_index + input_y*w + input_x;
                            int weights_index = weights_pre_index + f_y*ksize + f_x;

                            sum += input[input_index] * weights[weights_index];
                        }
                    }
                    // l.output[filters][width][height] +=
                    //        state.input[channels][width][height] *
                    //        l.weights[filters][channels][filter_width][filter_height];
                    output[output_index] += sum;
                }
    }
}

static inline int popcnt_64(uint64_t val64) {
#if defined(__x86_64__) || defined(__aarch64__)  // Linux 64-bit
    int tmp_count = __builtin_popcountll(val64);
#else  // Linux 32-bit
    int tmp_count = __builtin_popcount(val64);
    tmp_count += __builtin_popcount(val64 >> 32);
#endif
    return tmp_count;
}

void gemm_nn_custom_bin_mean_transposed(int M, int N, int K, float ALPHA_UNUSED,
    unsigned char *A, int lda,
    unsigned char *B, int ldb,
    float *C, int ldc, float *mean_arr)
{
    int i;

    #pragma omp parallel for
    for (i = 0; i < M; ++i) {   // l.n - filters [16 - 55 - 1024]
        int j, k;
        float mean_val = mean_arr[i];

        for (j = 0; j < N; ++j) { // out_h*out_w - one channel output size [169 - 173056]
            int count = 0;

            for (k = 0; k < K; k += 64) {   // l.size*l.size*l.c - one filter size [27 - 9216]
                uint64_t a_bit64 = *((uint64_t *)(A + (i*lda + k) / 8));
                uint64_t b_bit64 = *((uint64_t *)(B + (j*ldb + k) / 8));
                uint64_t c_bit64 = xnor_int64(a_bit64, b_bit64);

                int tmp_count = popcnt_64(c_bit64);

                if (K - k < 64)  tmp_count = tmp_count - (64 - (K - k));    // remove extra bits
                count += tmp_count;
                //binary_int64_printf(c_bit64);
                //printf(", count = %d \n\n", tmp_count);
            }

            C[i*ldc + j] = (2 * count - K) * mean_val;
        }
    }
}

void im2col_cpu_custom_transpose(float* data_im,
    int channels, int height, int width,
    int ksize, int stride, int pad, float* data_col, int ldb_align)
{
    printf("\n im2col_cpu_custom_transpose() isn't implemented without AVX \n");
}

//From Berkeley Vision's Caffe!
//https://github.com/BVLC/caffe/blob/master/LICENSE
void im2col_cpu_custom(float* data_im,
    int channels, int height, int width,
    int ksize, int stride, int pad, float* data_col)
{
    im2col_cpu(data_im, channels, height, width, ksize, stride, pad, data_col);
    return;

    int c;
    const int height_col = (height + 2 * pad - ksize) / stride + 1;
    const int width_col = (width + 2 * pad - ksize) / stride + 1;
    const int channels_col = channels * ksize * ksize;

    // optimized version
    if (height_col == height && width_col == width && stride == 1 && pad == 1)
    {
        #pragma omp parallel for
        for (c = 0; c < channels_col; ++c) {
            int h, w;
            int w_offset = c % ksize;
            int h_offset = (c / ksize) % ksize;
            int c_im = c / ksize / ksize;
            for (h = pad; h < height_col - pad; ++h) {
                for (w = pad; w < width_col - pad; ++w) {
                    int im_row = h_offset + h - pad;
                    int im_col = w_offset + w - pad;
                    int col_index = (c * height_col + h) * width_col + w;

                    data_col[col_index] = data_im[im_col + width*(im_row + height*c_im)];
                }

                for (; w < width_col - pad; ++w) {
                    int im_row = h_offset + h - pad;
                    int im_col = w_offset + w - pad;
                    int col_index = (c * height_col + h) * width_col + w;

                    data_col[col_index] = data_im[im_col + width*(im_row + height*c_im)];
                }
    }

            {
                w = 0;
                for (h = 0; h < height_col; ++h) {
                    int im_row = h_offset + h;
                    int im_col = w_offset + w;
                    int col_index = (c * height_col + h) * width_col + w;
                    data_col[col_index] = im2col_get_pixel(data_im, height, width, channels,
                        im_row, im_col, c_im, pad);
                }
            }

            {
                w = width_col - 1;
                for (h = 0; h < height_col; ++h) {
                    int im_row = h_offset + h;
                    int im_col = w_offset + w;
                    int col_index = (c * height_col + h) * width_col + w;
                    data_col[col_index] = im2col_get_pixel(data_im, height, width, channels,
                        im_row, im_col, c_im, pad);
                }
            }

            {
                h = 0;
                for (w = 0; w < width_col; ++w) {
                    int im_row = h_offset + h;
                    int im_col = w_offset + w;
                    int col_index = (c * height_col + h) * width_col + w;
                    data_col[col_index] = im2col_get_pixel(data_im, height, width, channels,
                        im_row, im_col, c_im, pad);
                }
            }

            {
                h = height_col - 1;
                for (w = 0; w < width_col; ++w) {
                    int im_row = h_offset + h;
                    int im_col = w_offset + w;
                    int col_index = (c * height_col + h) * width_col + w;
                    data_col[col_index] = im2col_get_pixel(data_im, height, width, channels,
                        im_row, im_col, c_im, pad);
                }
            }
        }

    }
    else {
        //printf("\n Error: is no non-optimized version \n");
        im2col_cpu(data_im, channels, height, width, ksize, stride, pad, data_col);
    }
}


//From Berkeley Vision's Caffe!
//https://github.com/BVLC/caffe/blob/master/LICENSE
void im2col_cpu_custom_bin(float* data_im,
    int channels, int height, int width,
    int ksize, int stride, int pad, float* data_col, int bit_align)
{
    int c;
    const int height_col = (height + 2 * pad - ksize) / stride + 1;
    const int width_col = (width + 2 * pad - ksize) / stride + 1;
    const int channels_col = channels * ksize * ksize;

    // optimized version
    if (height_col == height && width_col == width && stride == 1 && pad == 1)
    {
        int new_ldb = bit_align;

        #pragma omp parallel for
        for (c = 0; c < channels_col; ++c) {
            int h, w;
            int w_offset = c % ksize;
            int h_offset = (c / ksize) % ksize;
            int c_im = c / ksize / ksize;
            for (h = pad; h < height_col - pad; ++h) {
                for (w = pad; w < width_col - pad - 8; w += 1) {
                    int im_row = h_offset + h - pad;
                    int im_col = w_offset + w - pad;
                    //int col_index = (c * height_col + h) * width_col + w;
                    int col_index = c * new_ldb + h * width_col + w;

                    float val = data_im[im_col + width*(im_row + height*c_im)];
                    if (val > 0) set_bit((unsigned char*)data_col, col_index);
                }

                for (; w < width_col - pad; ++w) {
                    int im_row = h_offset + h - pad;
                    int im_col = w_offset + w - pad;
                    //int col_index = (c * height_col + h) * width_col + w;
                    int col_index = c * new_ldb + h * width_col + w;

                    //data_col[col_index] = data_im[im_col + width*(im_row + height*c_im)];
                    float val = data_im[im_col + width*(im_row + height*c_im)];
                    if (val > 0) set_bit((unsigned char*)data_col, col_index);
                }
            }

            {
                w = 0;
                for (h = 0; h < height_col; ++h) {
                    int im_row = h_offset + h;
                    int im_col = w_offset + w;
                    //int col_index = (c * height_col + h) * width_col + w;
                    int col_index = c * new_ldb + h * width_col + w;

                    //data_col[col_index] = im2col_get_pixel(data_im, height, width, channels, im_row, im_col, c_im, pad);
                    float val = im2col_get_pixel(data_im, height, width, channels, im_row, im_col, c_im, pad);
                    if (val > 0) set_bit((unsigned char*)data_col, col_index);
                }
            }

            {
                w = width_col - 1;
                for (h = 0; h < height_col; ++h) {
                    int im_row = h_offset + h;
                    int im_col = w_offset + w;
                    //int col_index = (c * height_col + h) * width_col + w;
                    int col_index = c * new_ldb + h * width_col + w;

                    //data_col[col_index] = im2col_get_pixel(data_im, height, width, channels, im_row, im_col, c_im, pad);
                    float val = im2col_get_pixel(data_im, height, width, channels, im_row, im_col, c_im, pad);
                    if (val > 0) set_bit((unsigned char*)data_col, col_index);
                }
            }

            {
                h = 0;
                for (w = 0; w < width_col; ++w) {
                    int im_row = h_offset + h;
                    int im_col = w_offset + w;
                    //int col_index = (c * height_col + h) * width_col + w;
                    int col_index = c * new_ldb + h * width_col + w;

                    //data_col[col_index] = im2col_get_pixel(data_im, height, width, channels, im_row, im_col, c_im, pad);
                    float val = im2col_get_pixel(data_im, height, width, channels, im_row, im_col, c_im, pad);
                    if (val > 0) set_bit((unsigned char*)data_col, col_index);
                }
            }

            {
                h = height_col - 1;
                for (w = 0; w < width_col; ++w) {
                    int im_row = h_offset + h;
                    int im_col = w_offset + w;
                    //int col_index = (c * height_col + h) * width_col + w;
                    int col_index = c * new_ldb + h * width_col + w;

                    //data_col[col_index] = im2col_get_pixel(data_im, height, width, channels, im_row, im_col, c_im, pad);
                    float val = im2col_get_pixel(data_im, height, width, channels, im_row, im_col, c_im, pad);
                    if (val > 0) set_bit((unsigned char*)data_col, col_index);
                }
            }
        }

    }
    else {
        printf("\n Error: is no non-optimized version \n");
        //im2col_cpu(data_im, channels, height, width, ksize, stride, pad, data_col); // must be aligned for transpose after float_to_bin
        // float_to_bit(b, t_input, src_size);
        // transpose_bin(t_input, *t_bit_input, k, n, bit_align, new_ldb, 8);
    }
}


void activate_array_cpu_custom(float *x, const int n, const ACTIVATION a)
{
    int i;
    if (a == LINEAR)
    {
    }
    else if (a == LEAKY)
    {
        for (i = 0; i < n; ++i) {
            x[i] = (x[i]>0) ? x[i] : .1*x[i];
        }
    }
    else {
        for (i = 0; i < n; ++i) {
            x[i] = activate(x[i], a);
        }
    }
}

void float_to_bit(float *src, unsigned char *dst, size_t size)
{
    size_t dst_size = size / 8 + 1;
    memset(dst, 0, dst_size);

    size_t i;
    char* byte_arr = (char*)calloc(size, sizeof(char));
    for (i = 0; i < size; ++i) {
        if (src[i] > 0) byte_arr[i] = 1;
    }

    //for (i = 0; i < size; ++i) {
    //    dst[i / 8] |= byte_arr[i] << (i % 8);
    //}

    for (i = 0; i < size; i += 8) {
        char dst_tmp = 0;
        dst_tmp |= byte_arr[i + 0] << 0;
        dst_tmp |= byte_arr[i + 1] << 1;
        dst_tmp |= byte_arr[i + 2] << 2;
        dst_tmp |= byte_arr[i + 3] << 3;
        dst_tmp |= byte_arr[i + 4] << 4;
        dst_tmp |= byte_arr[i + 5] << 5;
        dst_tmp |= byte_arr[i + 6] << 6;
        dst_tmp |= byte_arr[i + 7] << 7;
        dst[i / 8] = dst_tmp;
    }
    //free(byte_arr);
}

static inline void transpose_scalar_block(float *A, float *B, const int lda, const int ldb, const int block_size)
{
    int i;
    //#pragma omp parallel for
    for (i = 0; i<block_size; i++) {
        int j;
        for (j = 0; j<block_size; j++) {
            B[j*ldb + i] = A[i*lda + j];
        }
    }
}

void transpose_block_SSE4x4(float *A, float *B, const int n, const int m,
    const int lda, const int ldb, const int block_size)
{
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i += block_size) {
        int j, i2, j2;
        for (j = 0; j < m; j += block_size) {
            int max_i2 = i + block_size < n ? i + block_size : n;
            int max_j2 = j + block_size < m ? j + block_size : m;
            for (i2 = i; i2 < max_i2; ++i2) {
                for (j2 = j; j2 < max_j2; ++j2) {
                    B[j2*ldb + i2] = A[i2*lda + j2];
                }
                }
            }
        }
}

void forward_maxpool_layer_avx(float *src, float *dst, int *indexes, int size, int w, int h, int out_w, int out_h, int c,
    int pad, int stride, int batch)
{
    int b, k;
    const int w_offset = -pad / 2;
    const int h_offset = -pad / 2;

    for (b = 0; b < batch; ++b) {
        #pragma omp parallel for
        for (k = 0; k < c; ++k) {
            int i, j, m, n;
            for (i = 0; i < out_h; ++i) {
                for (j = 0; j < out_w; ++j) {
                    int out_index = j + out_w*(i + out_h*(k + c*b));
                    float max = -FLT_MAX;
                    int max_i = -1;
                    for (n = 0; n < size; ++n) {
                        for (m = 0; m < size; ++m) {
                            int cur_h = h_offset + i*stride + n;
                            int cur_w = w_offset + j*stride + m;
                            int index = cur_w + w*(cur_h + h*(k + b*c));
                            int valid = (cur_h >= 0 && cur_h < h &&
                                cur_w >= 0 && cur_w < w);
                            float val = (valid != 0) ? src[index] : -FLT_MAX;
                            max_i = (val > max) ? index : max_i;
                            max = (val > max) ? val : max;
                        }
                    }
                    dst[out_index] = max;
                    indexes[out_index] = max_i;
                }
            }
        }
    }
}


// 32 channels -> 1 channel (with 32 floats)
// 256 channels -> 8 channels (with 32 floats)
void repack_input(float *input, float *re_packed_input, int w, int h, int c)
{
    const int items_per_channel = w * h;
    int chan, i;
    for (chan = 0; chan < c; chan += 32)
    {
        for (i = 0; i < items_per_channel; ++i)
        {
            int c_pack;
            for (c_pack = 0; c_pack < 32; ++c_pack) {
                float src = input[(chan + c_pack)*items_per_channel + i];

                re_packed_input[chan*items_per_channel + i * 32 + c_pack] = src;
            }
        }
    }
}

void transpose_uint32(uint32_t *src, uint32_t *dst, int src_h, int src_w, int src_align, int dst_align)
{
    //l.bit_align - algined (n) by 32
    //new_ldb - aligned (k) by 256

    int i;
    //#pragma omp parallel for
    for (i = 0; i < src_h; i += 1)  // l.size*l.size*l.c;
    {
        int j;
        for (j = 0; j < src_w; j += 1)  // out_h*out_w;
        {
            ((uint32_t *)dst)[j*dst_align / 32 + i] = ((uint32_t *)src)[i*src_align + j];
        }
    }
}

void gemm_nn_bin_transposed_32bit_packed(int M, int N, int K, float ALPHA,
    uint32_t *A, int lda,
    uint32_t *B, int ldb,
    float *C, int ldc, float *mean_arr)
{
    int i;
    #pragma omp parallel for
    for (i = 0; i < M; ++i) {   // l.n
        int j, s;
        float mean_val = mean_arr[i];
        for (j = 0; j < N; ++j) // out_h*out_w;
        {
            float val = 0;
            for (s = 0; s < K; ++s) // l.size*l.size*l.c/32  or (l.size*l.size*l.c)
            {
                PUT_IN_REGISTER uint32_t A_PART = ((uint32_t*)A)[i*lda + s];
                PUT_IN_REGISTER uint32_t B_PART = ((uint32_t*)B)[j * ldb + s];
                uint32_t xnor_result = ~(A_PART ^ B_PART);
                int32_t count = popcnt_32(xnor_result);  // must be Signed int

                val += (2 * count - 32) * mean_val;
            }
            C[i*ldc + j] += val;
        }
    }
}

void convolution_repacked(uint32_t *packed_input, uint32_t *packed_weights, float *output,
    int w, int h, int c, int n, int size, int pad, int new_lda, float *mean_arr)
{
    int fil;
    // filter index
    #pragma omp parallel for
    for (fil = 0; fil < n; ++fil) {
        float mean_val = mean_arr[fil];
        int chan, y, x, f_y, f_x;   // c_pack
        // channel index
        for (chan = 0; chan < c / 32; ++chan)
            //for (chan = 0; chan < l.c; chan += 32)
            //for (c_pack = 0; c_pack < 32; ++c_pack)
            // input - y
            for (y = 0; y < h; ++y)
                // input - x
                for (x = 0; x < w; ++x)
                {
                    int const output_index = fil*w*h + y*w + x;
                    float sum = 0;

                    // filter - y
                    for (f_y = 0; f_y < size; ++f_y)
                    {
                        int input_y = y + f_y - pad;
                        // filter - x
                        for (f_x = 0; f_x < size; ++f_x)
                        {
                            int input_x = x + f_x - pad;
                            if (input_y < 0 || input_x < 0 || input_y >= h || input_x >= w) continue;

                            // normal
                            //float input = state.input[(chan + c_pack)*l.w*l.h + input_y*l.w + input_x];
                            //float weight = l.weights[fil*l.c*l.size*l.size + (chan + c_pack)*l.size*l.size + f_y*l.size + f_x];

                            // packed
                            //float input = re_packed_input[chan*l.w*l.h + (input_y*l.w + input_x) * 32 + c_pack];
                            //float weight = l.weights[fil*l.c*l.size*l.size + chan*l.size*l.size + (f_y*l.size + f_x) * 32 + c_pack];
                            //sum += input * weight;

                            //float input = re_packed_input[chan*l.w*l.h + (input_y*l.w + input_x) * 32 + c_pack];
                            //float weight = l.weights[fil*l.c*l.size*l.size + chan*l.size*l.size + (f_y*l.size + f_x) * 32 + c_pack];
                            //uint32_t bit1 = input > 0;
                            //uint32_t bit2 = weight > 0;
                            //uint32_t count = (~(bit1 ^ bit2)) & 1;
                            //float result = (2 * (float)count - 1) * mean_val;
                            //printf("\n mul = %f, bit1 = %d, bit2 = %d, count = %d, mean = %f, result = %f  ", input*weight, bit1, bit2, count, mean_val, result);
                            //sum += result;

                            uint32_t input = ((uint32_t *)packed_input)[chan*w*h + input_y*w + input_x];
                            //uint32_t weight = ((uint32_t *)l.align_bit_weights)[fil*l.c*l.size*l.size/32 + chan*l.size*l.size + f_y*l.size + f_x];
                            uint32_t weight = ((uint32_t *)packed_weights)[fil*new_lda / 32 + chan*size*size + f_y*size + f_x];

                            uint32_t xnor_result = ~(input ^ weight);
                            int32_t count = popcnt_32(xnor_result); // mandatory Signed int
                            sum += (2 * count - 32) * mean_val;
                        }
                    }
                    // l.output[filters][width][height] +=
                    //        state.input[channels][width][height] *
                    //        l.weights[filters][channels][filter_width][filter_height];
                    output[output_index] += sum;
                }
    }
}
void quantized_activate_array_cpu_custom(unsigned char *x, const int n, const ACTIVATION a,int quantization_layer_zeropoint)
{
    int i;
    if (a == LINEAR)
    {
    }
    else if (a == LEAKY)
    {
        for (i = 0; i < n; ++i) {
            x[i] = (x[i]>0) ? x[i] : .1*x[i];
        }
    }
    else {
        for (i = 0; i < n; ++i) {
            x[i] = quantized_activate(x[i], a, quantization_layer_zeropoint);
        }
    }
}

void gemm_nt(int M, int N, int K, float ALPHA,
        float *A, int lda,
        float *B, int ldb,
        float *C, int ldc)
{
    int i,j,k;
    for(i = 0; i < M; ++i){
        for(j = 0; j < N; ++j){
            PUT_IN_REGISTER float sum = 0;
            for(k = 0; k < K; ++k){
                sum += ALPHA*A[i*lda+k]*B[j*ldb + k];
            }
            C[i*ldc+j] += sum;
        }
    }
}

void gemm_tn(int M, int N, int K, float ALPHA,
        float *A, int lda,
        float *B, int ldb,
        float *C, int ldc)
{
    int i,j,k;
    for(i = 0; i < M; ++i){
        for(k = 0; k < K; ++k){
            PUT_IN_REGISTER float A_PART = ALPHA * A[k * lda + i];
            for(j = 0; j < N; ++j){
                C[i*ldc+j] += A_PART*B[k*ldb+j];
            }
        }
    }
}

void gemm_tt(int M, int N, int K, float ALPHA,
        float *A, int lda,
        float *B, int ldb,
        float *C, int ldc)
{
    int i,j,k;
    for(i = 0; i < M; ++i){
        for(j = 0; j < N; ++j){
            PUT_IN_REGISTER float sum = 0;
            for(k = 0; k < K; ++k){
                sum += ALPHA*A[i+k*lda]*B[k+j*ldb];
            }
            C[i*ldc+j] += sum;
        }
    }
}
void quantized_gemm_nn(int M, int N, int K, float ALPHA,
    signed char *A, int lda,
    unsigned char *B, int ldb,
    int *C, int ldc)
{
    int i, j, k;
    for (i = 0; i < M; ++i) {
        for (k = 0; k < K; ++k) {
            PUT_IN_REGISTER signed char A_PART = (signed char)ALPHA * A[i * lda + k];
            for (j = 0; j < N; ++j) {

                C[i*ldc + j] += (int)A_PART*B[k*ldb + j];
                // if((j==0)&&(k==0)){
                //     printf("A[%d]:%d\n",i * lda + k,A[i * lda + k]);
                //     printf("B[%d]:%d\n",k*ldb + j,B[k*ldb + j]);
                //     printf("C[%d]:%d\n",i*ldc + j,C[i*ldc + j]);
                // }
            }

        }
    }
}
void quantized_gemm_nn_int(int M, int N, int K, int ALPHA,
    signed char *A, int lda,
    unsigned char *B, int ldb,
    int *C, int ldc)
{
    int i, j, k;
    for (i = 0; i < M; ++i) {
        for (k = 0; k < K; ++k) {
            PUT_IN_REGISTER signed char A_PART = A[i * lda + k];
            for (j = 0; j < N; ++j) {

                C[i*ldc + j] += (int)A_PART*B[k*ldb + j];
                // if((j==0)&&(k==0)){
                //     printf("A[%d]:%d\n",i * lda + k,A[i * lda + k]);
                //     printf("B[%d]:%d\n",k*ldb + j,B[k*ldb + j]);
                //     printf("C[%d]:%d\n",i*ldc + j,C[i*ldc + j]);
                // }
            }

        }
    }
}
void quantized_gemm_nt(int M, int N, int K, float ALPHA,
        signed char *A, int lda,
        unsigned char *B, int ldb,
        int *C, int ldc)
{
    int i,j,k;
    for(i = 0; i < M; ++i){
        for(j = 0; j < N; ++j){
            PUT_IN_REGISTER int sum = 0;
            for(k = 0; k < K; ++k){
                //sum += (int)ALPHA*B[i*lda+k]*A[j*ldb + k];  // hujang ???? B[i*ldb+k]*A[j*lda + k]
                sum += (int)ALPHA*A[i*lda+k]*B[j*ldb+k];  // hujang ???? B[i*ldb+k]*A[j*lda + k]
               // printf("A:%d\n",A[i*lda+k]);
               // printf("B:%d\n",B[j*ldb + k]);
               // printf("add:%d\n",ALPHA*A[i*lda+k]*B[j*ldb + k]);
               // printf("SUM:%d\n",sum);
            }

            C[i*ldc+j] += sum;
            //printf("C[%d]:%d\n",i*ldc+j,C[i*ldc+j]);
        }
    }
}
void gemm_cpu(int TA, int TB, int M, int N, int K, float ALPHA,
        float *A, int lda,
        float *B, int ldb,
        float BETA,
        float *C, int ldc)
{
    //printf("cpu: %d %d %d %d %d %f %d %d %f %d\n",TA, TB, M, N, K, ALPHA, lda, ldb, BETA, ldc);
    if (BETA != 1){
        int i, j;
        for(i = 0; i < M; ++i){
            for(j = 0; j < N; ++j){
                C[i*ldc + j] *= BETA;
            }
        }
    }

    is_avx();   // initialize static variable
    if (is_fma_avx2() && !TA && !TB) {
        gemm_nn_fast(M, N, K, ALPHA, A, lda, B, ldb, C, ldc);
    }
    else {
#if 0
        int t;
        #pragma omp parallel for
        for (t = 0; t < M; ++t) {
            if (!TA && !TB)
                gemm_nn(1, N, K, ALPHA, A + t*lda, lda, B, ldb, C + t*ldc, ldc);
            else if (TA && !TB)
                gemm_tn(1, N, K, ALPHA, A + t, lda, B, ldb, C + t*ldc, ldc);
            else if (!TA && TB)
                gemm_nt(1, N, K, ALPHA, A + t*lda, lda, B, ldb, C + t*ldc, ldc);
            else
                gemm_tt(1, N, K, ALPHA, A + t, lda, B, ldb, C + t*ldc, ldc);
        }
#else
        if (!TA && !TB)
          gemm_nn(M, N, K, ALPHA, A, lda, B, ldb, C, ldc);
        else if (TA && !TB)
          gemm_tn(M, N, K, ALPHA, A, lda, B, ldb, C, ldc);
        else if (!TA && TB)
          gemm_nt(M, N, K, ALPHA, A, lda, B, ldb, C, ldc);
        else
          gemm_tt(M, N, K, ALPHA, A, lda, B, ldb, C, ldc);
#endif
    }
}
void quantized_gemm(int TA, int TB, int M, int N, int K, float ALPHA,
        signed char *A, int lda,
        unsigned char *B, int ldb,
        float BETA,
        int *C, int ldc)
{
   // printf("cpu: %d %d %d %d %d %f %d %d %f %d\n",TA, TB, M, N, K, ALPHA, lda, ldb, BETA, ldc);
    if (BETA != 1){
        int i, j;
        for(i = 0; i < M; ++i){
            for(j = 0; j < N; ++j){
                C[i*ldc + j] *= BETA;
            }
        }
    }

    // is_avx();   // initialize static variable
    // if (is_fma_avx2() && !TA && !TB) {
    //     gemm_nn_fast(M, N, K, ALPHA, A, lda, B, ldb, C, ldc);
    // }
    else {
#if 1
    //printf("###################### quantized_gemm: TA[%d] TB[%d]\n", TA, TB);
        int t;
        #pragma omp parallel for
        for (t = 0; t < M; ++t) {
            if (!TA && !TB)
                //M=64 N=196 K=800,ALPHA =1 lda=800,ldb=196 BETA=1 ldc=196
                quantized_gemm_nn(1, N, K, ALPHA, A + t*lda, lda, B, ldb, C + t*ldc, ldc);
            else if (TA && !TB)
                gemm_tn(1, N, K, ALPHA, A + t, lda, B, ldb, C + t*ldc, ldc);
            else if (!TA && TB)
                quantized_gemm_nt(1, N, K, ALPHA, A + t*lda, lda, B, ldb, C + t*ldc, ldc);
            else
                gemm_tt(1, N, K, ALPHA, A + t, lda, B, ldb, C + t*ldc, ldc);
        }
#else

    //quantized_gemm_nt(M, N, K, ALPHA, A, lda, B, ldb, C, ldc);

    //printf("######### quantized_gemm: TA[%d] TB[%d] M[%d] N[%d] K[%d]\n", TA, TB, M, N, K);
    //printf("######### quantized_gemm: lda[%d] ldb[%d] ldc[%d]\n", lda, ldb, ldc);
    int i;
    for(i = 0; i < 20; i++)
      printf("0x%02x ", A[i]);
    printf("\n");
    for(i = 0; i < 20; i++)
      printf("0x%02x ", B[i]);
    printf("\n");
    printf("A: 0x%08x\n", A);
    printf("B: 0x%08x\n", B);
    ErvpMatrixInfo matrix_a;
    ErvpMatrixInfo matrix_b;
    ErvpMatrixInfo matrix_c;
    //int *pact_C = malloc(sizeof(int32_t)*M*N);

    matrix_a.datatype = MATRIX_DATATYPE_SINT08;
    matrix_a.addr = (void *)A;
    matrix_a.num_row = M;
    matrix_a.num_col = K;
    matrix_a.stride = sizeof(int8_t)*K;

    matrix_b.datatype = MATRIX_DATATYPE_UINT08;
    matrix_b.addr = (void *)B;
    matrix_b.num_row = K;
    matrix_b.num_col = N;
    matrix_b.stride = sizeof(uint8_t)*N;

    matrix_c.datatype = MATRIX_DATATYPE_SINT32;
    //matrix_c.addr = (void *)pact_C;
    matrix_c.addr = (void *)C;
    matrix_c.num_row = M;
    matrix_c.num_col = N;
    matrix_c.stride = sizeof(int32_t)*N;

    matrix_mult_opt(&matrix_a, &matrix_b, &matrix_c);
#if 0
    for(i = 0; i < M*N; i++)
      printf("[%d/ %d] ", C[i], pact_C[i]);
    printf("\n");
#endif

#endif
    }
}
void quantized_gemm_int(int TA, int TB, int M, int N, int K, int ALPHA,
        signed char *A, int lda,
        unsigned char *B, int ldb,
        int BETA,
        int *C, int ldc)
{
   // printf("cpu: %d %d %d %d %d %f %d %d %f %d\n",TA, TB, M, N, K, ALPHA, lda, ldb, BETA, ldc);
    // if (BETA != 1){
    //     int i, j;
    //     for(i = 0; i < M; ++i){
    //         for(j = 0; j < N; ++j){
    //             C[i*ldc + j] *= BETA;
    //         }
    //     }
    // }

    // is_avx();   // initialize static variable
    // if (is_fma_avx2() && !TA && !TB) {
    //     gemm_nn_fast(M, N, K, ALPHA, A, lda, B, ldb, C, ldc);
    // }

#if 1

    int t;
    #pragma omp parallel for
    for (t = 0; t < M; ++t) {
        if (!TA && !TB)
            //M=64 N=196 K=800,ALPHA =1 lda=800,ldb=196 BETA=1 ldc=196
            quantized_gemm_nn_int(1, N, K, ALPHA, A + t*lda, lda, B, ldb, C + t*ldc, ldc);
    }

#else

    ErvpMatrixInfo matrix_a;
    ErvpMatrixInfo matrix_b;
    ErvpMatrixInfo matrix_c;

    matrix_a.datatype = MATRIX_DATATYPE_SINT08;
    matrix_a.addr = (void *)A;
    matrix_a.num_row = M;
    matrix_a.num_col = K;
    matrix_a.stride = sizeof(int8_t)*lda;

    matrix_b.datatype = MATRIX_DATATYPE_UINT08;
    matrix_b.addr = (void *)B;
    matrix_b.num_row = K;
    matrix_b.num_col = N;
    matrix_b.stride = sizeof(uint8_t)*ldb;

    matrix_c.datatype = MATRIX_DATATYPE_SINT32;
    matrix_c.addr = (void *)C;
    matrix_c.num_row = M;
    matrix_c.num_col = N;
    matrix_c.stride = sizeof(int32_t)*ldc;

    matrix_mult_opt(&matrix_a, &matrix_b, &matrix_c);

#endif
}

void quantized_input_accumulate(int M, int N, int K, // out_channels out_w*out*h  input_channel*kernel_w*kernel_h
    unsigned char *im2col_input, //input 
    int *input_acc) // output
{
    int i, j, k;
    for (i = 0; i < M; ++i) {
        for (k = 0; k < K; ++k) {
            for (j = 0; j < N; ++j) {

                input_acc[i*N + j] += (int)im2col_input[k*N + j];
            }
        }
    }
}

void quantized_weight_accumulate(int N, int K, //kernel_w*kernel_h  input_channel  
    int channel_num,
    signed char *quantized_weight, //input 
    int *quantized_weight_acc) // output
{
    int j, k;
    quantized_weight_acc[channel_num]=0;
    for (k = 0; k < K; ++k) {
        for (j = 0; j < N; ++j) {

            quantized_weight_acc[channel_num] += (int)quantized_weight[channel_num*N*K + k*N + j];
        }
    }
}

void fixed_nomalization(float input_scale, float* kernel_scale, float layer_scale,int out_channel,unsigned int *M_0, int *right_shift){
    int i;
    for(i=0; i<out_channel; i++){
        float M= input_scale*kernel_scale[i]/layer_scale;
        if (M==0) {
            right_shift[i]=32;
            M_0[i] =0;
        }
        else{
            float M_left_shfted=M;
            int shift = 0;
            assert(M>=0);
            assert(M<=1);
            while(M_left_shfted<0.5){
                M_left_shfted *=2;
                shift++;
            }    
          //  printf("M_left_shfted:%f\n",M_left_shfted);
            M_0[i]= (unsigned int)round(M_left_shfted*(float)((unsigned int)(1<<31)));
            if(M_0[i]==0){
                right_shift[i]=32;
            }
            else{
                right_shift[i]=31+shift ;          
            }

        }
   // printf("M_0[%d]=%d",i,M_0[i]);
    }
}
void bias_scale(float* bias , float input_scale,float *kernel_scale, int out_channel, int* bias_int32){
    int i;
    for (i =0; i<out_channel; i++){
        bias_int32[i] = (int)round(bias[i]/(input_scale*kernel_scale[i]));
    }
}

void conv_output_quantization(signed char *quantized_weight, unsigned char *im2col_input, int *mm_output,
                        int input_channel, int out_channel,
                        int out_h,int out_w,
                        int kernel_w,int kernel_h,
                        int input_zeropoint, int *kernel_zeropoint,
                        int *M0_int32, unsigned char* right_shift, 
                        int layer_zeropoint,
                        unsigned char *quantized_output_uint8, int *biases_int32) //output_int64_version
{  
   int i, j; 
    int kernel_size = kernel_w*kernel_h;
    int output_channel_size= out_h*out_w;
    int N =kernel_size*input_channel;
    int * quantized_input_acc = (int*)calloc(output_channel_size*out_channel,sizeof(int));
    int * quantized_weight_acc = (int*)calloc(out_channel,sizeof(int));
    int kernel_nonzero=0;
    for ( i=0; i<out_channel; i++){ //kernel zeropoint check
        if(kernel_zeropoint[i]) {
            kernel_nonzero =1;
            break;
        }
    }
    if (kernel_nonzero){
        quantized_input_accumulate(out_channel,output_channel_size,input_channel*kernel_w*kernel_h,im2col_input,quantized_input_acc);
    }
    else{
        for( i=0; i<output_channel_size*out_channel; i++) quantized_input_acc[i]=0;
    }
    int64_t round_mask=1;
    for(i=0;i<out_channel;i++){
        if (input_zeropoint) quantized_weight_accumulate(kernel_size, input_channel, i, quantized_weight, quantized_weight_acc); // output    
        else quantized_weight_acc[i] =0;
        int64_t temp_output=0;
        for(j =0; j<output_channel_size; j++){   
        if(kernel_nonzero || input_zeropoint) temp_output= (int64_t)(N*input_zeropoint*kernel_zeropoint[i]) -(int64_t)(input_zeropoint*quantized_weight_acc[i])- (int64_t)(quantized_input_acc[i*output_channel_size+j]*kernel_zeropoint[i])+(int64_t)(mm_output[i*output_channel_size+j])+(int64_t)(biases_int32[i]); 
            else temp_output = (int64_t)(mm_output[i*output_channel_size+j]+biases_int32[i]); 

            //int64 multiply
            temp_output*=(int64_t)M0_int32[i];
            //left shift
            if ( temp_output &(round_mask<<(right_shift[i]-1))) temp_output=(temp_output>>right_shift[i])+1;   
            else temp_output=(temp_output>>right_shift[i]);               
            //clamp
            if(temp_output>=255) quantized_output_uint8[i*output_channel_size+j]= 255;
            else if(temp_output<=0) quantized_output_uint8[i*output_channel_size+j]= (unsigned char)(layer_zeropoint);  
            else quantized_output_uint8[i*output_channel_size+j]=(unsigned char)(temp_output)+(unsigned char)(layer_zeropoint);


        }
    }
}



void connect_output_quantization(signed char *quantized_weight, unsigned char *input, int *mm_output,
                        int input_channel, int out_channel,
                       // int out_h,int out_w,
                       // int kernel_w,int kernel_h,
                        int input_zeropoint, int *kernel_zeropoint,
                        int *M0_int32, unsigned char* right_shift, 
                        int layer_zeropoint,
                        unsigned char *quantized_output_uint8, int *biases_int32)
{   
    int N =input_channel;
    int i;
    int64_t round_mask=1;
    unsigned int quantized_input_acc = 0;
    int * quantized_weight_acc = (int*)calloc(out_channel,sizeof(int));
    int kernel_nonzero=0;
    for ( i=0; i<out_channel; i++){ //kernel zeropoint check
        if(kernel_zeropoint[i]) {
            kernel_nonzero =1;
            break;
        }
    }
    if (kernel_nonzero) {
        for(i=0; i<input_channel; i++) quantized_input_acc+=(unsigned int) input[i];   
    }
    for(i=0;i<out_channel;i++){
        int64_t temp_output=0;
        
        if (input_zeropoint) quantized_weight_accumulate(1, input_channel, i, quantized_weight, quantized_weight_acc); // output 
        else quantized_weight_acc[i] =0;
        
        if(kernel_nonzero || input_zeropoint) temp_output= (int64_t)(N*input_zeropoint*kernel_zeropoint[i]) -(int64_t)(input_zeropoint*quantized_weight_acc[i])- (int64_t)(quantized_input_acc*kernel_zeropoint[i])+(int64_t)(mm_output[i])+(int64_t)(biases_int32[i]); 
        else temp_output = (int64_t)(mm_output[i])+(int64_t)(biases_int32[i]); 
        temp_output*=(int64_t)M0_int32[i];
        //left shift
        if ( temp_output &(round_mask<<(right_shift[i]-1))) temp_output=(temp_output>>right_shift[i]) +1;   
        else temp_output=(temp_output>>right_shift[i]) ;  
        //clamp             
        if(temp_output>=255) quantized_output_uint8[i]= 255;
        else if(temp_output<=0) quantized_output_uint8[i]= (unsigned char)(layer_zeropoint);  
        else quantized_output_uint8[i]=(unsigned char)(temp_output)+(unsigned char)(layer_zeropoint);


        }
}

void forward_quantized_maxpool_layer_avx(unsigned char *src, unsigned char *dst, int *indexes, int size, int w, int h, int out_w, int out_h, int c,
    int pad, int stride, int batch)
{
    printf("q_maxpool_avx init\n");

    int b, k;
    const int w_offset = -pad / 2;
    const int h_offset = -pad / 2;

    for (b = 0; b < batch; ++b) {
        #pragma omp parallel for
        for (k = 0; k < c; ++k) {
            int i, j, m, n;
            for (i = 0; i < out_h; ++i) {
                for (j = 0; j < out_w; ++j) {
                    int out_index = j + out_w*(i + out_h*(k + c*b));
                    unsigned char uint8_min= 0;
                    int max_i = -1;
                    for (n = 0; n < size; ++n) {
                        for (m = 0; m < size; ++m) {
                            int cur_h = h_offset + i*stride + n;
                            int cur_w = w_offset + j*stride + m;
                            int index = cur_w + w*(cur_h + h*(k + b*c));
                            int valid = (cur_h >= 0 && cur_h < h &&
                                cur_w >= 0 && cur_w < w);
                            float val = (valid != 0) ? src[index] : 0;
                            max_i = (val > uint8_min) ? index : max_i;
                            uint8_min = (val > uint8_min) ? val : uint8_min;
                        }
                    }
                    dst[out_index] = uint8_min;
                    if (indexes) indexes[out_index] = max_i;
                }
            }
        }
    }       
}
