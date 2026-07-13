#include "ervp_malloc.h"
#include "ervp_profiling.h"

#include "convolutional_layer.h"
#include "utils.h"
#include "batchnorm_layer.h"
#include "im2col.h"
#include "col2im.h"
#include "blas.h"
#include "gemm.h"
#include <stdio.h>
#include <time.h>

#ifdef AI2
#include "xnor_layer.h"
#endif

#ifdef __cplusplus
#define PUT_IN_REGISTER
#else
#define PUT_IN_REGISTER register
#endif

#ifndef AI2
#define AI2 0
void forward_xnor_layer(layer l, network_state state);
#endif

void swap_binary(convolutional_layer *l)
{
    float *swap = l->weights;
    l->weights = l->binary_weights;
    l->binary_weights = swap;
}

void binarize_weights(float *weights, int n, int size, float *binary)
{
    int i, f;
    for(f = 0; f < n; ++f){
        float mean = 0;
        for(i = 0; i < size; ++i){
            mean += fabs(weights[f*size + i]);
        }
        mean = mean / size;
        for(i = 0; i < size; ++i){
            binary[f*size + i] = (weights[f*size + i] > 0) ? mean: -mean;
        }
    }
}

void binarize_cpu(float *input, int n, float *binary)
{
    int i;
    for(i = 0; i < n; ++i){
        binary[i] = (input[i] > 0) ? 1 : -1;
    }
}

void binarize_input(float *input, int n, int size, float *binary)
{
    int i, s;
    for(s = 0; s < size; ++s){
        float mean = 0;
        for(i = 0; i < n; ++i){
            mean += fabs(input[i*size + s]);
        }
        mean = mean / n;
        for(i = 0; i < n; ++i){
            binary[i*size + s] = (input[i*size + s] > 0) ? mean : -mean;
        }
    }
}

int convolutional_out_height(convolutional_layer l)
{
    return (l.h + 2*l.pad - l.size) / l.stride + 1;
}

int convolutional_out_width(convolutional_layer l)
{
    return (l.w + 2*l.pad - l.size) / l.stride + 1;
}

image get_convolutional_image(convolutional_layer l)
{
    int h,w,c;
    h = convolutional_out_height(l);
    w = convolutional_out_width(l);
    c = l.n;
    return float_to_image(w,h,c,l.output);
}

image get_convolutional_delta(convolutional_layer l)
{
    int h,w,c;
    h = convolutional_out_height(l);
    w = convolutional_out_width(l);
    c = l.n;
    return float_to_image(w,h,c,l.delta);
}

size_t get_workspace_size32(layer l){
    if (l.xnor) {
        size_t re_packed_input_size = l.c * l.w * l.h * sizeof(float);
        size_t workspace_size = (size_t)l.bit_align*l.size*l.size*l.c * sizeof(float);
        if (workspace_size < re_packed_input_size) workspace_size = re_packed_input_size;
        return workspace_size;
    }
    if(l.quantization_type) return (size_t)l.out_h*l.out_w*l.size*l.size*(l.c / l.groups)*sizeof(char);
    else return (size_t)l.out_h*l.out_w*l.size*l.size*(l.c / l.groups)*sizeof(float);
}

size_t get_convolutional_workspace_size(layer l) {
    size_t workspace_size = get_workspace_size32(l);
    return workspace_size;
}
convolutional_layer make_quantized_convolutional_layer(int batch, int steps, int h, int w, int c, int n, int groups, int size, int stride, int padding, ACTIVATION activation, int batch_normalize, int binary, int xnor, int adam, int use_bin_output, int index,int quantization_type)
{
    int total_batch = batch*steps;
    int i;
    convolutional_layer l = { (LAYER_TYPE)0 };
    l.type = CONVOLUTIONAL;

    if (xnor) groups = 1;   // disable groups for XNOR-net
    if (groups < 1) groups = 1;

    l.quantization_type=quantization_type;
    l.index = index;
    l.h = h;
    l.w = w;
    l.c = c;
    l.groups = groups;
    l.n = n;
    l.binary = binary;
    l.xnor = xnor;
    l.use_bin_output = use_bin_output;
    l.batch = batch;
    l.steps = steps;
    l.stride = stride;
    l.size = size;
    l.pad = padding;
    l.batch_normalize = batch_normalize;
    l.learning_rate_scale = 1;
    l.nweights = (c / groups) * n * size * size;
    if(l.quantization_type){
        l.quantized_weights = (signed char*)calloc(l.nweights, sizeof(char));
        l.quantization_per_channel_zeropoint = (int*)calloc(n,sizeof(int)); 
        l.M0_int32 = (int*)calloc(n, sizeof(int));   
        l.right_shift = (unsigned char*)calloc(n, sizeof(unsigned char));  
        l.biases_int32 = (int*)calloc(n, sizeof(int));   
    }
    else{
        l.weights = (float*)calloc(l.nweights, sizeof(float));
        l.biases = (float*)calloc(n, sizeof(float));
    }
    //l.weights = (float*)calloc(l.nweights, sizeof(float));
    l.weight_updates = (float*)calloc(l.nweights, sizeof(float));

    //l.biases = (float*)calloc(n, sizeof(float));
    l.bias_updates = (float*)calloc(n, sizeof(float));

    // float scale = 1./sqrt(size*size*c);
    //float scale = sqrt(2./(size*size*c/groups));
    //for(i = 0; i < l.nweights; ++i) l.weights[i] = scale*rand_uniform(-1, 1);   // rand_normal();
    int out_h = convolutional_out_height(l);
    int out_w = convolutional_out_width(l);
    l.out_h = out_h;
    l.out_w = out_w;
    l.out_c = n;
    l.outputs = l.out_h * l.out_w * l.out_c;
    l.inputs = l.w * l.h * l.c;
    l.activation = activation;
    if(l.quantization_type){
       l.quantized_output = (int*)calloc(total_batch*l.outputs, sizeof(int));  /// q_check remove
       l.quantized_output_uint8 = (unsigned char*)calloc(total_batch*l.outputs, sizeof(char));
       //l.output = (float*)calloc(total_batch*l.outputs, sizeof(float)); //q_check
    }
    // else{
        // 
// 
    // }
    l.output = (float*)calloc(total_batch*l.outputs, sizeof(float));

    l.delta  = (float*)calloc(total_batch*l.outputs, sizeof(float));

    l.forward = forward_convolutional_layer;
#ifdef USE_BACKWARD_LAYER
    l.backward = backward_convolutional_layer;
#else
    l.backward = 0;
#endif
#ifdef USE_UPDATE_LAYER
    l.update = update_convolutional_layer;
#else
    l.update = 0;
#endif
    if(binary){
        l.binary_weights = (float*)calloc(l.nweights, sizeof(float));
        l.cweights = (char*)calloc(l.nweights, sizeof(char));
        l.scales = (float*)calloc(n, sizeof(float));
    }
    if(xnor){
        l.binary_weights = (float*)calloc(l.nweights, sizeof(float));
        l.binary_input = (float*)calloc(l.inputs * l.batch, sizeof(float));

        int align = 32;// 8;
        int src_align = l.out_h*l.out_w;
        l.bit_align = src_align + (align - src_align % align);

        l.mean_arr = (float*)calloc(l.n, sizeof(float));

        const size_t new_c = l.c / 32;
        size_t in_re_packed_input_size = new_c * l.w * l.h + 1;
        l.bin_re_packed_input = (uint32_t*)calloc(in_re_packed_input_size, sizeof(uint32_t));

        l.lda_align = 256;  // AVX2
        int k = l.size*l.size*l.c;
        size_t k_aligned = k + (l.lda_align - k%l.lda_align);
        size_t t_bit_input_size = k_aligned * l.bit_align / 8;
        l.t_bit_input = (char*)calloc(t_bit_input_size, sizeof(char));
    }

    if(batch_normalize){
        l.scales = (float*)calloc(n, sizeof(float));
        l.scale_updates = (float*)calloc(n, sizeof(float));
        for(i = 0; i < n; ++i){
            l.scales[i] = 1;
        }

        l.mean = (float*)calloc(n, sizeof(float));
        l.variance = (float*)calloc(n, sizeof(float));

        l.mean_delta = (float*)calloc(n, sizeof(float));
        l.variance_delta = (float*)calloc(n, sizeof(float));

        l.rolling_mean = (float*)calloc(n, sizeof(float));
        l.rolling_variance = (float*)calloc(n, sizeof(float));
        l.x = (float*)calloc(total_batch * l.outputs, sizeof(float));
        l.x_norm = (float*)calloc(total_batch * l.outputs, sizeof(float));
    }
    if(adam){
        l.adam = 1;
        l.m = (float*)calloc(l.nweights, sizeof(float));
        l.v = (float*)calloc(l.nweights, sizeof(float));
        l.bias_m = (float*)calloc(n, sizeof(float));
        l.scale_m = (float*)calloc(n, sizeof(float));
        l.bias_v = (float*)calloc(n, sizeof(float));
        l.scale_v = (float*)calloc(n, sizeof(float));
    }

    l.workspace_size = get_convolutional_workspace_size(l);

    //printf("conv  %5d %2d x%2d /%2d  %4d x%4d x%4d   ->  %4d x%4d x%4d\n", n, size, size, stride, w, h, c, l.out_w, l.out_h, l.out_c);
    l.bflops = (2.0 * l.nweights * l.out_h*l.out_w) / 1000000000.;
    if (l.xnor && l.use_bin_output) printf("convXB");
    else if (l.xnor) printf("convX ");
    else printf("conv  ");
    printf("%5d %2d x%2d /%2d  %4d x%4d x%4d   ->  %4d x%4d x%4d %5.3f BF\n", n, size, size, stride, w, h, c, l.out_w, l.out_h, l.out_c, l.bflops);

    return l;
}
convolutional_layer make_convolutional_layer(int batch, int steps, int h, int w, int c, int n, int groups, int size, int stride, int padding, ACTIVATION activation, int batch_normalize, int binary, int xnor, int adam, int use_bin_output, int index)
{
    int total_batch = batch*steps;
    int i;
    convolutional_layer l = { (LAYER_TYPE)0 };
    l.type = CONVOLUTIONAL;

    if (xnor) groups = 1;   // disable groups for XNOR-net
    if (groups < 1) groups = 1;

    l.index = index;
    l.h = h;
    l.w = w;
    l.c = c;
    l.groups = groups;
    l.n = n;
    l.binary = binary;
    l.xnor = xnor;
    l.use_bin_output = use_bin_output;
    l.batch = batch;
    l.steps = steps;
    l.stride = stride;
    l.size = size;
    l.pad = padding;
    l.batch_normalize = batch_normalize;
    l.learning_rate_scale = 1;
    l.nweights = (c / groups) * n * size * size;

    l.weights = (float*)calloc(l.nweights, sizeof(float));
    l.weight_updates = (float*)calloc(l.nweights, sizeof(float));

    l.biases = (float*)calloc(n, sizeof(float));
    l.bias_updates = (float*)calloc(n, sizeof(float));

    // float scale = 1./sqrt(size*size*c);
    //float scale = sqrt(2./(size*size*c/groups));
    //for(i = 0; i < l.nweights; ++i) l.weights[i] = scale*rand_uniform(-1, 1);   // rand_normal();
    int out_h = convolutional_out_height(l);
    int out_w = convolutional_out_width(l);
    l.out_h = out_h;
    l.out_w = out_w;
    l.out_c = n;
    l.outputs = l.out_h * l.out_w * l.out_c;
    l.inputs = l.w * l.h * l.c;
    l.activation = activation;

    l.output = (float*)calloc(total_batch*l.outputs, sizeof(float));
    l.delta  = (float*)calloc(total_batch*l.outputs, sizeof(float));

    l.forward = forward_convolutional_layer;
#ifdef USE_BACKWARD_LAYER
    l.backward = backward_convolutional_layer;
#else
    l.backward = 0;
#endif
#ifdef USE_UPDATE_LAYER
    l.update = update_convolutional_layer;
#else
    l.update = 0;
#endif
    if(binary){
        l.binary_weights = (float*)calloc(l.nweights, sizeof(float));
        l.cweights = (char*)calloc(l.nweights, sizeof(char));
        l.scales = (float*)calloc(n, sizeof(float));
    }
    if(xnor){
        l.binary_weights = (float*)calloc(l.nweights, sizeof(float));
        l.binary_input = (float*)calloc(l.inputs * l.batch, sizeof(float));

        int align = 32;// 8;
        int src_align = l.out_h*l.out_w;
        l.bit_align = src_align + (align - src_align % align);

        l.mean_arr = (float*)calloc(l.n, sizeof(float));

        const size_t new_c = l.c / 32;
        size_t in_re_packed_input_size = new_c * l.w * l.h + 1;
        l.bin_re_packed_input = (uint32_t*)calloc(in_re_packed_input_size, sizeof(uint32_t));

        l.lda_align = 256;  // AVX2
        int k = l.size*l.size*l.c;
        size_t k_aligned = k + (l.lda_align - k%l.lda_align);
        size_t t_bit_input_size = k_aligned * l.bit_align / 8;
        l.t_bit_input = (char*)calloc(t_bit_input_size, sizeof(char));
    }

    if(batch_normalize){
        l.scales = (float*)calloc(n, sizeof(float));
        l.scale_updates = (float*)calloc(n, sizeof(float));
        for(i = 0; i < n; ++i){
            l.scales[i] = 1;
        }

        l.mean = (float*)calloc(n, sizeof(float));
        l.variance = (float*)calloc(n, sizeof(float));

        l.mean_delta = (float*)calloc(n, sizeof(float));
        l.variance_delta = (float*)calloc(n, sizeof(float));

        l.rolling_mean = (float*)calloc(n, sizeof(float));
        l.rolling_variance = (float*)calloc(n, sizeof(float));
        l.x = (float*)calloc(total_batch * l.outputs, sizeof(float));
        l.x_norm = (float*)calloc(total_batch * l.outputs, sizeof(float));
    }
    if(adam){
        l.adam = 1;
        l.m = (float*)calloc(l.nweights, sizeof(float));
        l.v = (float*)calloc(l.nweights, sizeof(float));
        l.bias_m = (float*)calloc(n, sizeof(float));
        l.scale_m = (float*)calloc(n, sizeof(float));
        l.bias_v = (float*)calloc(n, sizeof(float));
        l.scale_v = (float*)calloc(n, sizeof(float));
    }

    l.workspace_size = get_convolutional_workspace_size(l);

    //printf("conv  %5d %2d x%2d /%2d  %4d x%4d x%4d   ->  %4d x%4d x%4d\n", n, size, size, stride, w, h, c, l.out_w, l.out_h, l.out_c);
    l.bflops = (2.0 * l.nweights * l.out_h*l.out_w) / 1000000000.;
    if (l.xnor && l.use_bin_output) printf("convXB");
    else if (l.xnor) printf("convX ");
    else printf("conv  ");
    printf("%5d %2d x%2d /%2d  %4d x%4d x%4d   ->  %4d x%4d x%4d %5.3f BF\n", n, size, size, stride, w, h, c, l.out_w, l.out_h, l.out_c, l.bflops);

    return l;
}

void denormalize_convolutional_layer(convolutional_layer l)
{
    int i, j;
    for(i = 0; i < l.n; ++i){
        float scale = l.scales[i]/sqrt(l.rolling_variance[i] + .00001);
        for(j = 0; j < l.nweights; ++j){
            l.weights[i*l.nweights + j] *= scale;
        }
        l.biases[i] -= l.rolling_mean[i] * scale;
        l.scales[i] = 1;
        l.rolling_mean[i] = 0;
        l.rolling_variance[i] = 1;
    }
}

void test_convolutional_layer()
{
    convolutional_layer l = make_convolutional_layer(1, 1, 5, 5, 3, 2, 1, 5, 2, 1, LEAKY, 1, 0, 0, 0, 0, 0);
    l.batch_normalize = 1;
    float data[] = {1,1,1,1,1,
        1,1,1,1,1,
        1,1,1,1,1,
        1,1,1,1,1,
        1,1,1,1,1,
        2,2,2,2,2,
        2,2,2,2,2,
        2,2,2,2,2,
        2,2,2,2,2,
        2,2,2,2,2,
        3,3,3,3,3,
        3,3,3,3,3,
        3,3,3,3,3,
        3,3,3,3,3,
        3,3,3,3,3};
    network_state state = {0};
    state.input = data;
    forward_convolutional_layer(l, state);
}

void resize_convolutional_layer(convolutional_layer *l, int w, int h)
{
    int total_batch = l->batch*l->steps;
    int old_w = l->w;
    int old_h = l->h;
    l->w = w;
    l->h = h;
    int out_w = convolutional_out_width(*l);
    int out_h = convolutional_out_height(*l);

    l->out_w = out_w;
    l->out_h = out_h;

    l->outputs = l->out_h * l->out_w * l->out_c;
    l->inputs = l->w * l->h * l->c;

    l->output = (float*)realloc(l->output, total_batch * l->outputs * sizeof(float));
    l->delta = (float*)realloc(l->delta, total_batch * l->outputs * sizeof(float));
    if(l->batch_normalize){
        l->x = (float*)realloc(l->x, total_batch * l->outputs * sizeof(float));
        l->x_norm = (float*)realloc(l->x_norm, total_batch * l->outputs * sizeof(float));
    }

    if (l->xnor) {
        //l->binary_input = realloc(l->inputs*l->batch, sizeof(float));
    }

    l->workspace_size = get_convolutional_workspace_size(*l);
}

void add_bias(float *output, float *biases, int batch, int n, int size)
{
    int i,j,b;
    for(b = 0; b < batch; ++b){
        for(i = 0; i < n; ++i){
            for(j = 0; j < size; ++j){
                output[(b*n + i)*size + j] += biases[i];
            }
        }
    }
}

void scale_bias(float *output, float *scales, int batch, int n, int size)
{
    int i,j,b;
    for(b = 0; b < batch; ++b){
        for(i = 0; i < n; ++i){
            for(j = 0; j < size; ++j){
                output[(b*n + i)*size + j] *= scales[i];
            }
        }
    }
}

void backward_bias(float *bias_updates, float *delta, int batch, int n, int size)
{
    int i,b;
    for(b = 0; b < batch; ++b){
        for(i = 0; i < n; ++i){
            bias_updates[i] += sum_array(delta+size*(i+b*n), size);
        }
    }
}

void gemm_nn_custom(int M, int N, int K, float ALPHA,
    float *A, int lda,
    float *B, int ldb,
    float *C, int ldc)
{
    int i, j, k;
    for (i = 0; i < M; ++i) {
        for (k = 0; k < K; ++k) {
            PUT_IN_REGISTER float A_PART = ALPHA * A[i * lda + k];
            //printf("\n weight = %f \n", A_PART);
            for (j = 0; j < N; ++j) {
                C[i*ldc + j] += A_PART*B[k*ldb + j];
            }
        }
    }
}


void get_mean_array(float *src, size_t size, size_t filters, float *mean_arr) {
    size_t i, counter;
    counter = 0;
    for (i = 0; i < size; i += size / filters) {
        mean_arr[counter++] = fabs(src[i]);
    }
}

/*
void float_to_bit(float *src, unsigned char *dst, size_t size) {

    size_t dst_size = size / 8 + 1;
    memset(dst, 0, dst_size);
    size_t i, dst_i, dst_shift;
    for (i = 0; i < size; ++i) {
        if (src[i] > 0) set_bit(dst, i);
    }
}
*/

void bit_to_float(unsigned char *src, float *dst, size_t size, size_t filters, float *mean_arr) {
    memset(dst, 0, size *sizeof(float));
    size_t i;

    for (i = 0; i < size; ++i) {
        float mean_val = 1;
        if(mean_arr != NULL) mean_val = fabs(mean_arr[i / (size / filters)]);
        if(get_bit(src, i)) dst[i] = mean_val;
        else dst[i] = -mean_val;
    }
}

void binary_align_weights(convolutional_layer *l)
{
    int m = l->n;   // (l->n / l->groups)
    int k = l->size*l->size*l->c;   // ->size*l->size*(l->c / l->groups)
    size_t new_lda = k + (l->lda_align - k % l->lda_align); // (k / 8 + 1) * 8;
    l->new_lda = new_lda;

    binarize_weights(l->weights, m, k, l->binary_weights);

    size_t align_weights_size = new_lda * m;
    l->align_bit_weights_size = align_weights_size / 8 + 1;
    float* align_weights = (float*)calloc(align_weights_size, sizeof(float));
    l->align_bit_weights = (char*)calloc(l->align_bit_weights_size, sizeof(char));

    size_t i, j;
    // align A without transpose
    for (i = 0; i < m; ++i) {
        for (j = 0; j < k; ++j) {
            align_weights[i*new_lda + j] = l->binary_weights[i*k + j];
        }
    }


    if (l->c % 32 == 0)
    //if (l->stride == 1 && l->pad == 1 && l->c % 32 == 0)
    {
        int fil, chan;
        const int items_per_filter = l->c * l->size * l->size;
        //const int dst_items_per_filter = new_lda;
        for (fil = 0; fil < l->n; ++fil)
        {
            for (chan = 0; chan < l->c; chan += 32)
            {
                const int items_per_channel = l->size*l->size;
                for (i = 0; i < items_per_channel; ++i)
                {
                    //uint32_t val = 0;
                    int c_pack;
                    for (c_pack = 0; c_pack < 32; ++c_pack) {
                        float src = l->binary_weights[fil*items_per_filter + (chan + c_pack)*items_per_channel + i];

                        //align_weights[fil*items_per_filter + chan*items_per_channel + i * 32 + c_pack] = src;

                        align_weights[fil*new_lda + chan*items_per_channel + i*32 + c_pack] = src;
                        //val |= (src << c);
                    }

                }
            }
        }

        //printf("\n l.index = %d \t aw[0] = %f, aw[1] = %f, aw[2] = %f, aw[3] = %f \n", l->index, align_weights[0], align_weights[1], align_weights[2], align_weights[3]);
        //memcpy(l->binary_weights, align_weights, (l->size * l->size * l->c * l->n) * sizeof(float));

        float_to_bit(align_weights, (unsigned char*)l->align_bit_weights, align_weights_size);

        get_mean_array(l->binary_weights, m*k, l->n, l->mean_arr);
        //get_mean_array(l->binary_weights, m*new_lda, l->n, l->mean_arr);
    }
    else {
        float_to_bit(align_weights, (unsigned char*)l->align_bit_weights, align_weights_size);

        get_mean_array(l->binary_weights, m*k, l->n, l->mean_arr);
    }

    //l->mean_arr = calloc(l->n, sizeof(float));

    //get_mean_array(align_weights, align_weights_size, l->n, l->mean_arr);

    //free(align_weights);
}

// binary transpose
size_t binary_transpose_align_input(int k, int n, float *b, char **t_bit_input, size_t ldb_align, int bit_align)
{
    size_t new_ldb = k + (ldb_align - k%ldb_align); // (k / 8 + 1) * 8;
    //printf("\n n = %d, bit_align = %d \n", n, bit_align);
    size_t t_intput_size = new_ldb * bit_align;// n;
    size_t t_bit_input_size = t_intput_size / 8;// +1;

    memset(*t_bit_input, 0, t_bit_input_size * sizeof(char));
    //int src_size = k * bit_align;

    // b - [bit_align, k] - [l.bit_align, l.size*l.size*l.c] = src_size
    // t_input - [bit_align, k] - [n', k]
    // t_bit_input - [new_ldb, n] - [k', n]

    //transpose_bin(t_input, *t_bit_input, k, n, bit_align, new_ldb, 8);
    transpose_bin((uint32_t*)b, (uint32_t*)*t_bit_input, k, n, bit_align, new_ldb, 8);

    return t_intput_size;
}


void forward_convolutional_layer(convolutional_layer l, network_state state)
{
    int out_h = convolutional_out_height(l);
    int out_w = convolutional_out_width(l);
    int i, j, iter;
    
    
    fill_cpu(l.outputs*l.batch, 0, l.output, 1);
    if (l.xnor && (!l.align_bit_weights || state.train)) {
        if (!l.align_bit_weights || state.train) {
            binarize_weights(l.weights, l.n, l.nweights, l.binary_weights);
            //printf("\n binarize_weights l.align_bit_weights = %p \n", l.align_bit_weights);
        }
        swap_binary(&l);
        binarize_cpu(state.input, l.c*l.h*l.w*l.batch, l.binary_input);
        state.input = l.binary_input;
    }

    int m = l.n / l.groups;
    int k = l.size*l.size*l.c / l.groups;
    int n = out_h*out_w;

    static int u = 0;
    u++;
    float *a;
    float *b;
    float *c;
    signed char *q_a;
    unsigned char *q_b;
    int *q_c;
    float *im;
    unsigned char *q_im;
    for(i = 0; i < l.batch; ++i){
        for (j = 0; j < l.groups; ++j) {
            if(state.quantization_type){
                q_a = l.quantized_weights +j*l.nweights / l.groups;

                q_b=(unsigned char*) calloc(l.out_h*l.out_w*l.size*l.size*l.c , sizeof(unsigned char)); 
                //q_b = (unsigned char*)state.workspace;
                q_c = l.quantized_output +(i*l.groups + j)*n*m;
          //      for(iter = 0 ; iter<l.outputs; iter++)q_c[iter]=0;
            }
            else{

                a = l.weights +j*l.nweights / l.groups;
                b = state.workspace;
                c = l.output +(i*l.groups + j)*n*m;

            }

            //gemm(0,0,m,n,k,1,a,k,b,n,1,c,n);
            //gemm_nn_custom(m, n, k, 1, a, k, b, n, c, n);
            if (l.xnor && l.align_bit_weights && !state.train)
            {
                memset(b, 0, l.bit_align*l.size*l.size*l.c * sizeof(float));

                if (l.c % 32 == 0)
                {
                    //printf(" l.index = %d - new XNOR \n", l.index);

                    int ldb_align = l.lda_align;
                    size_t new_ldb = k + (ldb_align - k%ldb_align); // (k / 8 + 1) * 8;
                    //size_t t_intput_size = new_ldb * l.bit_align;// n;
                    //size_t t_bit_input_size = t_intput_size / 8;// +1;

                    int re_packed_input_size = l.c * l.w * l.h;
                    memset(state.workspace, 0, re_packed_input_size * sizeof(float));

                    const size_t new_c = l.c / 32;
                    size_t in_re_packed_input_size = new_c * l.w * l.h + 1;
                    memset(l.bin_re_packed_input, 0, in_re_packed_input_size * sizeof(uint32_t));

                    //float *re_packed_input = calloc(l.c * l.w * l.h, sizeof(float));
                    //uint32_t *bin_re_packed_input = calloc(new_c * l.w * l.h + 1, sizeof(uint32_t));

                    // float32x4 by channel (as in cuDNN)
                    repack_input(state.input, state.workspace, l.w, l.h, l.c);

                    // 32 x floats -> 1 x uint32_t
                    float_to_bit(state.workspace, (unsigned char *)l.bin_re_packed_input, l.c * l.w * l.h);

                    //free(re_packed_input);

                    // slow - convolution the packed inputs and weights: float x 32 by channel (as in cuDNN)
                    //convolution_repacked((uint32_t *)bin_re_packed_input, (uint32_t *)l.align_bit_weights, l.output,
                    //    l.w, l.h, l.c, l.n, l.size, l.pad, l.new_lda, l.mean_arr);

                    // // then exit from if()


                    im2col_cpu_custom((float *)l.bin_re_packed_input, new_c, l.h, l.w, l.size, l.stride, l.pad, state.workspace);
                    //im2col_cpu((float *)bin_re_packed_input, new_c, l.h, l.w, l.size, l.stride, l.pad, b);

                    //free(bin_re_packed_input);

                    int new_k = l.size*l.size*l.c / 32;

                    // good for (l.c == 64)
                    //gemm_nn_bin_32bit_packed(m, n, new_k, 1,
                    //    l.align_bit_weights, l.new_lda/32,
                    //    b, n,
                    //    c, n, l.mean_arr);

    // // then exit from if()

                    transpose_uint32((uint32_t *)state.workspace, (uint32_t*)l.t_bit_input, new_k, n, n, new_ldb);

                    // the main GEMM function
                    gemm_nn_custom_bin_mean_transposed(m, n, k, 1, (unsigned char*)l.align_bit_weights, new_ldb, (unsigned char*)l.t_bit_input, new_ldb, c, n, l.mean_arr);

                    // // alternative GEMM
                    //gemm_nn_bin_transposed_32bit_packed(m, n, new_k, 1,
                    //    l.align_bit_weights, l.new_lda/32,
                    //    t_bit_input, new_ldb / 32,
                    //    c, n, l.mean_arr);

                    //free(t_bit_input);

                }
                else
                { // else (l.c % 32 != 0)

                    //--------------------------------------------------------
                    //printf(" l.index = %d - old XNOR \n", l.index);

                    //im2col_cpu_custom_align(state.input, l.c, l.h, l.w, l.size, l.stride, l.pad, b, l.bit_align);
                    im2col_cpu_custom_bin(state.input, l.c, l.h, l.w, l.size, l.stride, l.pad, state.workspace, l.bit_align);

                    //size_t output_size = l.outputs;
                    //float *count_output = calloc(output_size, sizeof(float));
                    //size_t bit_output_size = output_size / 8 + 1;
                    //char *bit_output = calloc(bit_output_size, sizeof(char));

                    //size_t intput_size = n * k; // (out_h*out_w) X (l.size*l.size*l.c) : after im2col()
                    //size_t bit_input_size = intput_size / 8 + 1;
                    //char *bit_input = calloc(bit_input_size, sizeof(char));

                    //size_t weights_size = k * m; //l.size*l.size*l.c*l.n;
                    //size_t bit_weights_size = weights_size / 8 + 1;

                    //char *bit_weights = calloc(bit_weights_size, sizeof(char));
                    //float *mean_arr = calloc(l.n, sizeof(float));

                    // transpose B from NxK to KxN (x-axis (ldb = l.size*l.size*l.c) - should be multiple of 8 bits)
                    {
                        //size_t ldb_align = 256; // 256 bit for AVX2
                        int ldb_align = l.lda_align;
                        size_t new_ldb = k + (ldb_align - k%ldb_align);
                        size_t t_intput_size = binary_transpose_align_input(k, n, state.workspace, &l.t_bit_input, ldb_align, l.bit_align);

                        // 5x times faster than gemm()-float32
                        gemm_nn_custom_bin_mean_transposed(m, n, k, 1, (unsigned char*)l.align_bit_weights, new_ldb, (unsigned char*)l.t_bit_input, new_ldb, c, n, l.mean_arr);

                        //gemm_nn_custom_bin_mean_transposed(m, n, k, 1, bit_weights, k, t_bit_input, new_ldb, c, n, mean_arr);

                        //free(t_input);
                        //free(t_bit_input);
                        //}
                    }

                }

                add_bias(l.output, l.biases, l.batch, l.n, out_h*out_w);

                //activate_array(l.output, m*n*l.batch, l.activation);
                activate_array_cpu_custom(l.output, m*n*l.batch, l.activation);
                return;

            } // end xnor 
            else {
                if(state.quantization_type){
                    q_im = state.quantized_input + (i*l.groups + j)*(l.c / l.groups)*l.h*l.w;
                    //for(iter=0; iter <=300; iter++) printf("q_im[%d]:%d\n",iter,q_im[iter]);
                    printf("init q_im2col\n");
                    profiling_start("conv_im2col");
                    quantized_im2col_cpu_ext(q_im,   // input
                        l.c / l.groups,     // input channels
                        l.h, l.w,           // input size (h, w)
                        l.size, l.size,     // kernel size (h, w)
                        l.pad * 1, l.pad * 1,       // padding (h, w)
                        l.stride, l.stride, // stride (h, w)
                        1, 1, // dilation (h, w)
                        q_b, state.quantized_input_zeropoint);                 // output
                    profiling_end("conv_im2col");
                    printf("im2col done\n");
                    profiling_start("conv_gemm");            
                    quantized_gemm_int(0, 0, m, n, k, 1, q_a, k, q_b, n, 1, q_c, n);
                    profiling_end("conv_gemm");
                    printf("q_gemm done\n");
                    printf("make q_out_int64 done\n");
                    profiling_start("conv_out_quan");  
                    conv_output_quantization(q_a,q_b,q_c,
                        l.c/l.groups,l.n/l.groups,
                        out_h,out_w,
                        l.size,l.size,
                        state.quantized_input_zeropoint,l.quantization_per_channel_zeropoint,
                        l.M0_int32,l.right_shift,
                        l.quantization_layer_zeropoint,
                        l.quantized_output_uint8,l.biases_int32);
                    profiling_end("conv_out_quan");
                }
                else{
                    //printf(" l.index = %d - FP32 \n", l.index);
                    im2col_cpu(state.input + (i*l.groups + j)*l.c / l.groups*l.h*l.w,
                        l.c / l.groups, l.h, l.w, l.size, l.stride, l.pad, b);

                    gemm(0, 0, m, n, k, 1, a, k, b, n, 1, c, n);
                    // bit-count to float
                    c += n*m;
                    state.input += l.c*l.h*l.w;
                }
            }

        }
    }

    if(l.batch_normalize){
        forward_batchnorm_layer(l, state);
    }
    if(l.quantization_type) {
        printf("init_quantized_activate\n");
        quantized_activate_array_cpu_custom(l.quantized_output_uint8, l.outputs*l.batch, l.activation,l.quantization_layer_zeropoint);
    }
    else {
        add_bias(l.output, l.biases, l.batch, l.n, out_h*out_w);
        //activate_array(l.output, m*n*l.batch, l.activation);
        activate_array_cpu_custom(l.output, l.outputs*l.batch, l.activation);
    }
    if(l.binary || l.xnor) swap_binary(&l);
}


void backward_convolutional_layer(convolutional_layer l, network_state state)
{
    int i, j;
    int m = l.n / l.groups;
    int n = l.size*l.size*l.c / l.groups;
    int k = l.out_w*l.out_h;

    gradient_array(l.output, l.outputs*l.batch, l.activation, l.delta);

    if (l.batch_normalize) {
        backward_batchnorm_layer(l, state);
    }
    else {
        backward_bias(l.bias_updates, l.delta, l.batch, l.n, k);
    }

    for (i = 0; i < l.batch; ++i) {
        for (j = 0; j < l.groups; ++j) {
            float *a = l.delta + (i*l.groups + j)*m*k;
            float *b = state.workspace;
            float *c = l.weight_updates + j*l.nweights / l.groups;

            float *im = state.input + (i*l.groups + j)*l.c / l.groups*l.h*l.w;

            im2col_cpu(im, l.c / l.groups, l.h, l.w,
                l.size, l.stride, l.pad, b);
            gemm(0, 1, m, n, k, 1, a, k, b, k, 1, c, n);

            if (state.delta) {
                a = l.weights + j*l.nweights / l.groups;
                b = l.delta + (i*l.groups + j)*m*k;
                c = state.workspace;

                gemm(1, 0, n, k, m, 1, a, n, b, k, 0, c, k);

                col2im_cpu(state.workspace, l.c / l.groups, l.h, l.w, l.size, l.stride,
                    l.pad, state.delta + (i*l.groups + j)*l.c / l.groups*l.h*l.w);
            }
        }
    }
}

void update_convolutional_layer(convolutional_layer l, int batch, float learning_rate, float momentum, float decay)
{
    //int size = l.size*l.size*l.c*l.n;
    axpy_cpu(l.n, learning_rate / batch, l.bias_updates, 1, l.biases, 1);
    scal_cpu(l.n, momentum, l.bias_updates, 1);

    if (l.scales) {
        axpy_cpu(l.n, learning_rate / batch, l.scale_updates, 1, l.scales, 1);
        scal_cpu(l.n, momentum, l.scale_updates, 1);
    }

    axpy_cpu(l.nweights, -decay*batch, l.weights, 1, l.weight_updates, 1);
    axpy_cpu(l.nweights, learning_rate / batch, l.weight_updates, 1, l.weights, 1);
    scal_cpu(l.nweights, momentum, l.weight_updates, 1);
}



image get_convolutional_weight(convolutional_layer l, int i)
{
    int h = l.size;
    int w = l.size;
    int c = l.c / l.groups;
    return float_to_image(w, h, c, l.weights + i*h*w*c);
}

void rgbgr_weights(convolutional_layer l)
{
    int i;
    for (i = 0; i < l.n; ++i) {
        image im = get_convolutional_weight(l, i);
        if (im.c == 3) {
            rgbgr_image(im);
        }
    }
}

void rescale_weights(convolutional_layer l, float scale, float trans)
{
    int i;
    for (i = 0; i < l.n; ++i) {
        image im = get_convolutional_weight(l, i);
        if (im.c == 3) {
            scale_image(im, scale);
            float sum = sum_array(im.data, im.w*im.h*im.c);
            l.biases[i] += sum*trans;
        }
    }
}

image *get_weights(convolutional_layer l)
{
    image *weights = (image *)calloc(l.n, sizeof(image));
    int i;
    for (i = 0; i < l.n; ++i) {
        weights[i] = copy_image(get_convolutional_weight(l, i));
        normalize_image(weights[i]);
        /*
        char buff[256];
        sprintf(buff, "filter%d", i);
        save_image(weights[i], buff);
        */
    }
    //error("hey");
    return weights;
}

image *visualize_convolutional_layer(convolutional_layer l, char *window, image *prev_weights)
{
    image *single_weights = get_weights(l);
    show_images(single_weights, l.n, window);

    image delta = get_convolutional_image(l);
    image dc = collapse_image_layers(delta, 1);
    char buff[256];
    sprintf(buff, "%s: Output", window);
    //show_image(dc, buff);
    //save_image(dc, buff);
    free_image(dc);
    return single_weights;
}
