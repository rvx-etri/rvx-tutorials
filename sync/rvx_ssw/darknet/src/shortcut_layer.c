#include "ervp_malloc.h"
#include "ervp_assert.h"

#include "shortcut_layer.h"
#include "blas.h"
#include <stdio.h>

layer make_shortcut_layer(int batch, int index, int w, int h, int c, int w2, int h2, int c2)
{
    printf("Shortcut Layer: %d\n", index);
    layer l = { (LAYER_TYPE)0 };
    l.type = SHORTCUT;
    l.batch = batch;
    l.w = w2;
    l.h = h2;
    l.c = c2;
    l.out_w = w;
    l.out_h = h;
    l.out_c = c;
    l.outputs = w*h*c;
    l.inputs = l.outputs;

    l.index = index;
    l.nweights = 0;
    l.delta = (float*)calloc(l.outputs * batch, sizeof(float));
    l.output = (float*)calloc(l.outputs * batch, sizeof(float));

    l.forward = forward_shortcut_layer;
#ifdef USE_BACKWARD_LAYER
    l.backward = backward_shortcut_layer;
#else
    l.backward = 0;
#endif
    return l;
}
layer make_quantized_shortcut_layer(int batch, int index, int w, int h, int c, int w2, int h2, int c2,int quantization_type)   //q_chceck
{
    printf("Shortcut Layer: %d\n", index);
    
    layer l = { (LAYER_TYPE)0 };
    l.quantization_type=quantization_type;
    l.type = SHORTCUT;
    l.batch = batch;
    l.w = w2; 
    l.h = h2;
    l.c = c2;
    l.out_w = w;
    l.out_h = h;
    l.out_c = c;
    printf("l.w:%d l.h:%d l.c:%d \nl.out_w:%d l.out_h:%d l.out_c:%d\n",l.w,l.h,l.c,l.out_w,l.out_h,l.out_c);
    l.outputs = w*h*c;
    l.inputs = l.outputs;
    l.index = index;
    l.nweights = 0;
    if(l.quantization_type){
        l.quantized_output = (int*)calloc(batch*l.outputs, sizeof(int));
        l.quantized_output_uint8 = (unsigned char*)calloc(l.outputs * batch, sizeof(char));
        if (w != w2 || h != h2 || c != c2){
            l.nweights = c*c2;
            l.quantized_weights = (signed char*)calloc(l.nweights, sizeof(char));
            l.M0_int32 = (int*)calloc(c, sizeof(int));   
            l.right_shift = (unsigned char*)calloc(c, sizeof(unsigned char));   
            l.biases_int32 = (int*)calloc(c, sizeof(int));
            l.quantization_per_channel_zeropoint = (int*)calloc(c,sizeof(int)); 
        }
    }
    // else{
        // 
// 
    // }
    l.output = (float*)calloc(l.outputs * batch, sizeof(float));
    l.delta = (float*)calloc(l.outputs * batch, sizeof(float));

    l.forward = forward_shortcut_layer;
#ifdef USE_BACKWARD_LAYER
    l.backward = backward_shortcut_layer;
#else
    l.backward = 0;
#endif
    return l;
}
void resize_shortcut_layer(layer *l, int w, int h)
{
    //assert(l->w == l->out_w);
    //assert(l->h == l->out_h);
    l->w = l->out_w = w;
    l->h = l->out_h = h;
    l->outputs = w*h*l->out_c;
    l->inputs = l->outputs;
    l->delta = (float*)realloc(l->delta, l->outputs * l->batch * sizeof(float));
    l->output = (float*)realloc(l->output, l->outputs * l->batch * sizeof(float));


}

void forward_shortcut_layer(const layer l, network_state state)  // /w w2 h h2 c c2 
{
    int i;
    signed char *q_a;
    unsigned char *q_b;
    int *q_c;
    int m = l.out_c ;
    int k = 1*1*l.c ;
    int n = l.out_h*l.out_w;//q_check  
    int size = l.batch * l.out_w * l.out_h * l.out_c;
    if (l.w == l.out_w && l.h == l.out_h && l.c == l.out_c) {

        if (l.quantization_type){
            #pragma omp parallel for
            for(i = 0; i < size; ++i){
                //scale nomalize is required state  state.quantized_input_scale and state.net.layers[l.index].quantization_layer_scale
                //assert(state.quantized_input_scale == l.quantization_layer_scale && state.quantized_input_zeropoint == l.quantization_layer_zeropoint);
               // assert(state.quantized_input_scale == state.net.layers[l.index].quantization_layer_scale && state.quantized_input_zeropoint == state.net.layers[l.index].quantization_layer_zeropoint);
                l.quantized_output_uint8[i] = state.quantized_input[i] + state.net.layers[l.index].quantized_output_uint8[i];
            }
        }
        else{
            #pragma omp parallel for
            for(i = 0; i < size; ++i){
                    l.output[i] = state.input[i] + state.net.layers[l.index].output[i];
            }
        }
    }                                       
    else {
        if (l.quantization_type){        
             printf(" init shortcut covolution layer\n");
            q_a = l.quantized_weights;
            q_b = (unsigned char*)state.workspace;
            q_c = l.quantized_output;
            //for(i = 0 ; i<l.out_h*l.out_w*l.c; i++)q_b[i]=0;
            for(i = 0 ; i<l.outputs; i++)q_c[i]=0;
            
           // printf(" l.c :%d l.h :%d l.w :%d \n l.out_c:%d l.out_h:%d  l.out_w:%d \n",l.c , l.h , l.w , l.out_c, l.out_h,  l.out_w );
           // printf(" m:%d, n:%d, k:%d\n",m,n,k);
            unsigned char *quantized_output_uint8_to_be_added = (unsigned char *)calloc(size, sizeof(unsigned char));
            #pragma omp parallel for
            for(i = 0; i < size; ++i){
                quantized_output_uint8_to_be_added[i] = 0;
                }
           quantized_im2col_cpu_ext(state.net.layers[l.index].quantized_output_uint8,   // input
                l.c,     // input channels
                l.h, l.w,           // input size (h, w)
                1, 1,     // kernel size (h, w)
                0, 0,       // padding (h, w)
                2, 2, // stride (h, w)
                1, 1, // dilation (h, w)
                q_b,state.net.layers[l.index].quantization_layer_zeropoint);   // output
            //assert(state.quantized_input_scale == l.quantization_layer_scale && state.quantized_input_zeropoint == l.quantization_layer_zeropoint);
           // assert(state.quantized_input_scale == state.net.layers[l.index].quantization_layer_scale && state.quantized_input_zeropoint == state.net.layers[l.index].quantization_layer_zeropoint);
            // for(i = 0; i < 16; ++i){
            //      printf("shortcut conv q_a[%d]:%d\n",i,q_a[i]);            
            // }    
            // for(i = 0; i < 16; ++i){
            //      printf("shortcut conv q_b[%d]:%d\n",i,q_b[i]);            
            // }            
            quantized_gemm_int(0, 0, m, n, k, 1, q_a, k, q_b, n, 1, q_c, n);
            // for(i = 0; i < 16; ++i){
            //      printf("shortcut conv q_c[%d]:%d\n",i,q_c[i]);            
            // }
            conv_output_quantization(q_a,q_b,q_c,
                l.c,l.out_c, //input channels ,output channels
                l.out_h,l.out_w, // output h ,w
                1,1,
                l.quantization_layer_zeropoint,l.quantization_per_channel_zeropoint,
                l.M0_int32,l.right_shift,
                 state.quantized_input_zeropoint,
                 quantized_output_uint8_to_be_added,l.biases_int32);
            // for(i = 0; i < 16; ++i){
            //      printf("shortcut conv quantized_output_uint8_to_be_added[%d]:%d\n",i,quantized_output_uint8_to_be_added[i]);
            // }
            #pragma omp parallel for
            for(i = 0; i < size; ++i){ 
                l.quantized_output_uint8[i] = state.quantized_input[i] + quantized_output_uint8_to_be_added[i];
            }
            // for(i = 0; i < 16; ++i){
            //      printf("shortcut conv output[%d]:%d\n",i,l.quantized_output_uint8[i]);
            // }
        }
        else{
            copy_cpu(l.outputs*l.batch, state.input, 1, l.output, 1);
            shortcut_cpu(l.batch, l.w, l.h, l.c, state.net.layers[l.index].output, l.out_w, l.out_h, l.out_c, l.output);
        }
    }
    //activate_array(l.output, l.outputs*l.batch, l.activation);
}

void backward_shortcut_layer(const layer l, network_state state)
{
    gradient_array(l.output, l.outputs*l.batch, l.activation, l.delta);
    axpy_cpu(l.outputs*l.batch, 1, l.delta, 1, state.delta, 1);
    shortcut_cpu(l.batch, l.out_w, l.out_h, l.out_c, l.delta, l.w, l.h, l.c, state.net.layers[l.index].delta);
}
