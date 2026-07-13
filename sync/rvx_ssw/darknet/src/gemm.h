#ifndef GEMM_H
#define GEMM_H
#include "activations.h"
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

void convolution_2d(int w, int h, int ksize, int n, int c, int pad, int stride,
    float *weights, float *input, float *output, float *mean);

static inline void set_bit(unsigned char *const dst, size_t index) {
    size_t dst_i = index / 8;
    int dst_shift = index % 8;
    dst[dst_i] |= 1 << dst_shift;
    //dst[dst_i] |= 1 << (8 - dst_shift);
}

static inline unsigned char get_bit(unsigned char const*const src, size_t index) {
    size_t src_i = index / 8;
    int src_shift = index % 8;
    unsigned char val = (src[src_i] & (1 << src_shift)) > 0;
    //unsigned char val = (src[src_i] & (1 << (8 - src_shift))) > 0;
    return val;
}

int is_avx();
int is_fma_avx2();

void float_to_bit(float *src, unsigned char *dst, size_t size);

void transpose_block_SSE4x4(float *A, float *B, const int n, const int m,
    const int lda, const int ldb, const int block_size);

void transpose_bin(uint32_t *A, uint32_t *B, const int n, const int m,
    const int lda, const int ldb, const int block_size);

void gemm_nn_custom_bin_mean_transposed(int M, int N, int K, float ALPHA_UNUSED,
    unsigned char *A, int lda,
    unsigned char *B, int ldb,
    float *C, int ldc, float *mean_arr);

void im2col_cpu_custom(float* data_im,
    int channels, int height, int width,
    int ksize, int stride, int pad, float* data_col);

void im2col_cpu_custom_align(float* data_im,
    int channels, int height, int width,
    int ksize, int stride, int pad, float* data_col, int bit_align);

void im2col_cpu_custom_bin(float* data_im,
    int channels, int height, int width,
    int ksize, int stride, int pad, float* data_col, int bit_align);

void im2col_cpu_custom_transpose(float* data_im,
    int channels, int height, int width,
    int ksize, int stride, int pad, float* data_col, int ldb_align);

void activate_array_cpu_custom(float *x, const int n, const ACTIVATION a);

void transpose_32x32_bits_reversed_diagonale(uint32_t *A, uint32_t *B, int m, int n);

void gemm_bin(int M, int N, int K, float ALPHA,
        char  *A, int lda,
        float *B, int ldb,
        float *C, int ldc);

void repack_input(float *input, float *re_packed_input, int w, int h, int c);

void convolution_repacked(uint32_t *packed_input, uint32_t *packed_weights, float *output,
    int w, int h, int c, int n, int size, int pad, int new_lda, float *mean_arr);

void gemm_nn_bin_32bit_packed(int M, int N, int K, float ALPHA,
    uint32_t *A, int lda,
    uint32_t *B, int ldb,
    float *C, int ldc, float *mean_arr);

void transpose_uint32(uint32_t *src, uint32_t *dst, int src_h, int src_w, int src_align, int dst_align);

void gemm_nn_bin_transposed_32bit_packed(int M, int N, int K, float ALPHA,
    uint32_t *A, int lda,
    uint32_t *B, int ldb,
    float *C, int ldc, float *mean_arr);


void forward_maxpool_layer_avx(float *src, float *dst, int *indexes, int size, int w, int h, int out_w, int out_h, int c,
    int pad, int stride, int batch);
void forward_quantized_maxpool_layer_avx(unsigned char *src, unsigned char *dst, int *indexes, int size, int w, int h, int out_w, int out_h, int c,
    int pad, int stride, int batch);

void gemm(int TA, int TB, int M, int N, int K, float ALPHA,
                    float *A, int lda,
                    float *B, int ldb,
                    float BETA,
                    float *C, int ldc);

void gemm_cpu(int TA, int TB, int M, int N, int K, float ALPHA,
        float *A, int lda,
        float *B, int ldb,
        float BETA,
        float *C, int ldc);
void quantized_gemm(int TA, int TB, int M, int N, int K, float ALPHA,
                    signed char *A, int lda,
                    unsigned char *B, int ldb,
                    float BETA,
                    int *C, int ldc);
void quantized_gemm_int(int TA, int TB, int M, int N, int K, int ALPHA,
                    signed char *A, int lda,
                    unsigned char *B, int ldb,
                    int BETA,
                    int *C, int ldc);
void fixed_nomalization(float input_scale, float* kernel_scale,float layer_scale,int out_channel,unsigned int *M_0, int *right_shift);
void bias_scale(float* bias, float input_scale,float *kernel_scale, int out_channel, int* bias_int32);
void conv_output_quantization(signed char *quantized_weight, unsigned char *im2col_input, int *mm_output,
                        int input_channel, int out_channel,
                        int out_h,int out_w,
                        int kernel_w,int kernel_h,
                        int input_zeropoint, int *kernel_zeropoint,
                        int *M0_int32, unsigned char* right_shift, 
                        int layer_zeropoint,
                        unsigned char *quantized_output_uint8, int *biases_int32); //output_int64_version
void connect_output_quantization(signed char *quantized_weight,unsigned char *input,  int *mm_output,
                        int input_channel, int out_channel,
                       // int out_h,int out_w,
                       // int kernel_w,int kernel_h,
                        int input_zeropoint, int *kernel_zeropoint,
                        int *M0_int32, unsigned char* right_shift, 
                        int layer_zeropoint,
                        unsigned char *quantized_output_uint8, int *biases_int32);
void quantized_input_accumulate(int M, int N, int K, // out_channels out_w*out*h  input_channel*kernel_w*kernel_h
                                unsigned char *im2col_input, //input 
                                int *input_acc); // output
void quantized_weight_accumulate(int N, int K, //kernel_w*kernel_h  input_channel  
                                int channel_num,
                                signed char *quantized_weight, //input 
                                int *quantized_weight_acc); // output
void quantized_activate_array_cpu_custom(unsigned char *x, const int n, const ACTIVATION a, int quantization_layer_zeropoint);


#ifdef __cplusplus
}
#endif
#endif
