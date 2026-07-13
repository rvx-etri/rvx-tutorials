#include "ervp_malloc.h"
#include "ervp_printf.h"
#include "ervp_profiling.h"

#include "connected_layer.h"
#include "batchnorm_layer.h"
#include "convolutional_layer.h"
#include "utils.h"
#include "blas.h"
#include "gemm.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

connected_layer make_connected_layer(int batch, int steps, int inputs, int outputs, ACTIVATION activation, int batch_normalize)
{
    int total_batch = batch*steps;
    int i;
    connected_layer l = { (LAYER_TYPE)0 };
    l.type = CONNECTED;

    l.inputs = inputs;
    l.outputs = outputs;
    l.batch= batch;
    l.batch_normalize = batch_normalize;
    l.h = 1;
    l.w = 1;
    l.c = inputs;
    l.out_h = 1;
    l.out_w = 1;
    l.out_c = outputs;
    l.n = l.out_c;
    l.size = 1;
    l.stride = 1;
    l.pad = 0;
    l.activation = activation;
    l.learning_rate_scale = 1;

    l.output = (float*)calloc(total_batch * outputs, sizeof(float));
    l.delta = (float*)calloc(total_batch * outputs, sizeof(float));

    l.weight_updates = (float*)calloc(inputs * outputs, sizeof(float));
    l.bias_updates = (float*)calloc(outputs, sizeof(float));

    l.weights = (float*)calloc(outputs * inputs, sizeof(float));
    l.biases = (float*)calloc(outputs, sizeof(float));

    l.forward = forward_connected_layer;
#ifdef USE_BACKWARD_LAYER
    l.backward = backward_connected_layer;
#else
    l.backward = 0;
#endif
#ifdef USE_UPDATE_LAYER
    l.update = update_connected_layer;
#else
    l.update = 0;
#endif

    //float scale = 1./sqrt(inputs);
    //float scale = sqrt(2.f/inputs);
    //for(i = 0; i < outputs*inputs; ++i){
    //    l.weights[i] = scale*rand_uniform(-1, 1);
    //}

    for(i = 0; i < outputs; ++i){
        l.biases[i] = 0;
    }

    if(batch_normalize){
        l.scales = (float*)calloc(outputs, sizeof(float));
        l.scale_updates = (float*)calloc(outputs, sizeof(float));
        for(i = 0; i < outputs; ++i){
            l.scales[i] = 1;
        }

        l.mean = (float*)calloc(outputs, sizeof(float));
        l.mean_delta = (float*)calloc(outputs, sizeof(float));
        l.variance = (float*)calloc(outputs, sizeof(float));
        l.variance_delta = (float*)calloc(outputs, sizeof(float));

        l.rolling_mean = (float*)calloc(outputs, sizeof(float));
        l.rolling_variance = (float*)calloc(outputs, sizeof(float));

        l.x = (float*)calloc(total_batch * outputs, sizeof(float));
        l.x_norm = (float*)calloc(total_batch * outputs, sizeof(float));
    }

    printf("connected                            %4d  ->  %4d\n", inputs, outputs);
    return l;
}
connected_layer make_quantized_connected_layer(int batch, int steps, int inputs, int outputs, ACTIVATION activation, int batch_normalize, int quantization_type)
{
    int total_batch = batch*steps;
    int i;
    connected_layer l = { (LAYER_TYPE)0 };
    l.type = CONNECTED;
    l.quantization_type=quantization_type;
    l.inputs = inputs;
    l.outputs = outputs;
    l.batch= batch;
    l.batch_normalize = batch_normalize;
    l.h = 1;
    l.w = 1;
    l.c = inputs;
    l.out_h = 1;
    l.out_w = 1;
    l.out_c = outputs;
    l.n = l.out_c;
    l.size = 1;
    l.stride = 1;
    l.pad = 0;
    l.activation = activation;
    l.learning_rate_scale = 1;
    if(l.quantization_type){

        l.quantized_output = (int*)calloc(total_batch * outputs, sizeof(int));        
        l.quantized_output_uint8 = (unsigned char*)calloc(total_batch * outputs, sizeof(char));        
        l.quantized_weights = (signed char*)calloc(outputs * inputs, sizeof(char));
        l.M0_int32 = (int*)calloc(outputs, sizeof(int));   
        l.right_shift = (unsigned char*)calloc(outputs, sizeof(unsigned char));   
        l.quantization_per_channel_zeropoint = (int*)calloc(outputs,sizeof(int)); 
        l.biases_int32 = (int*)calloc(outputs, sizeof(int));   
    }
    else{   

        l.weights = (float*)calloc(outputs * inputs, sizeof(float));
        l.biases = (float*)calloc(outputs, sizeof(float));

        for(i = 0; i < outputs; ++i){
            l.biases[i] = 0;
        }  
    }
    l.output = (float*)calloc(total_batch * outputs, sizeof(float));     
    l.delta = (float*)calloc(total_batch * outputs, sizeof(float));

    l.weight_updates = (float*)calloc(inputs * outputs, sizeof(float));
    l.bias_updates = (float*)calloc(outputs, sizeof(float));

    // l.weights = (float*)calloc(outputs * inputs, sizeof(float));

    l.forward = forward_connected_layer;
#ifdef USE_BACKWARD_LAYER
    l.backward = backward_connected_layer;
#else
    l.backward = 0;
#endif
#ifdef USE_UPDATE_LAYER
    l.update = update_connected_layer;
#else
    l.update = 0;
#endif

    //float scale = 1./sqrt(inputs);
    //float scale = sqrt(2.f/inputs);
    //for(i = 0; i < outputs*inputs; ++i){
    //    l.weights[i] = scale*rand_uniform(-1, 1);
    //}



    if(batch_normalize){
        l.scales = (float*)calloc(outputs, sizeof(float));
        l.scale_updates = (float*)calloc(outputs, sizeof(float));
        for(i = 0; i < outputs; ++i){
            l.scales[i] = 1;
        }

        l.mean = (float*)calloc(outputs, sizeof(float));
        l.mean_delta = (float*)calloc(outputs, sizeof(float));
        l.variance = (float*)calloc(outputs, sizeof(float));
        l.variance_delta = (float*)calloc(outputs, sizeof(float));

        l.rolling_mean = (float*)calloc(outputs, sizeof(float));
        l.rolling_variance = (float*)calloc(outputs, sizeof(float));

        l.x = (float*)calloc(total_batch * outputs, sizeof(float));
        l.x_norm = (float*)calloc(total_batch * outputs, sizeof(float));
    }

    printf("connected                            %4d  ->  %4d\n", inputs, outputs);
    return l;
}
void update_connected_layer(connected_layer l, int batch, float learning_rate, float momentum, float decay)
{
    axpy_cpu(l.outputs, learning_rate/batch, l.bias_updates, 1, l.biases, 1);
    scal_cpu(l.outputs, momentum, l.bias_updates, 1);

    if(l.batch_normalize){
        axpy_cpu(l.outputs, learning_rate/batch, l.scale_updates, 1, l.scales, 1);
        scal_cpu(l.outputs, momentum, l.scale_updates, 1);
    }

    axpy_cpu(l.inputs*l.outputs, -decay*batch, l.weights, 1, l.weight_updates, 1);
    axpy_cpu(l.inputs*l.outputs, learning_rate/batch, l.weight_updates, 1, l.weights, 1);
    scal_cpu(l.inputs*l.outputs, momentum, l.weight_updates, 1);
}

void forward_connected_layer(connected_layer l, network_state state)
{
    int i;
    
    int m = l.batch;
    int k = l.inputs;
    int n = l.outputs;
    fill_cpu(l.outputs*l.batch, 0, l.output, 1);
    if(l.quantization_type){
        signed char *q_a = l.quantized_weights;
        unsigned char *q_b = state.quantized_input;

        int *q_c = l.quantized_output;
        for(i = 0 ; i<n; i++)q_c[i]=0;
        profiling_start("connected_gemm"); 
        //quantized_gemm(0,1,m,n,k,1,q_a,k,q_b,k,1,q_c,n);  
        printf("q_a: 0x%08x\n", q_a);
        printf("q_b: 0x%08x\n", q_b);
  
        quantized_gemm(0,1,n,m,k,1,q_a,k,q_b,k,1,q_c,m);    
        profiling_end("connected_gemm"); 
        profiling_start("connected_out_quan"); 
        connect_output_quantization(q_a,q_b,q_c,
        l.inputs,l.outputs,
        state.quantized_input_zeropoint,l.quantization_per_channel_zeropoint,
        l.M0_int32,l.right_shift,
        l.quantization_layer_zeropoint,
        l.quantized_output_uint8,l.biases_int32); 
        profiling_end("connected_out_quan"); 
    }
    else{
        
        float *a = state.input;
        float *b = l.weights;
        float *c = l.output;

        gemm(0,1,m,n,k,1,a,k,b,k,1,c,n);
    }
    if(l.batch_normalize){
        if(state.train){
            mean_cpu(l.output, l.batch, l.outputs, 1, l.mean);
            variance_cpu(l.output, l.mean, l.batch, l.outputs, 1, l.variance);

            scal_cpu(l.outputs, .95f, l.rolling_mean, 1);
            axpy_cpu(l.outputs, .05f, l.mean, 1, l.rolling_mean, 1);
            scal_cpu(l.outputs, .95f, l.rolling_variance, 1);
            axpy_cpu(l.outputs, .05f, l.variance, 1, l.rolling_variance, 1);

            copy_cpu(l.outputs*l.batch, l.output, 1, l.x, 1);
            normalize_cpu(l.output, l.mean, l.variance, l.batch, l.outputs, 1);
            copy_cpu(l.outputs*l.batch, l.output, 1, l.x_norm, 1);
        } else {
            normalize_cpu(l.output, l.rolling_mean, l.rolling_variance, l.batch, l.outputs, 1);
        }
        scale_bias(l.output, l.scales, l.batch, l.outputs, 1);
    }
   if(l.quantization_type){
        for(i = 0; i < l.batch; ++i){
            // printf("init quan axpy cpu\n"); 
            //quantized_axpy_cpu(l.outputs, 1, l.biases, 1, quantized_output_float + i*l.outputs, 1,l.quantization_layer_scale);

            //for(i=0; i<l.outputs ; i++) l.quantized_output_uint8[i]= q_rounding(quantized_output_float[i]);
            //free(quantized_output_float);     
            activate_quantized_array(l.quantized_output_uint8, l.outputs*l.batch, l.activation, l.quantization_layer_zeropoint);
        }        

    }
    else{
        for(i = 0; i < l.batch; ++i){
            axpy_cpu(l.outputs, 1, l.biases, 1, l.output + i*l.outputs, 1);
            activate_array(l.output, l.outputs*l.batch, l.activation);
        }                                               

    }
}

void backward_connected_layer(connected_layer l, network_state state)
{
    int i;
    gradient_array(l.output, l.outputs*l.batch, l.activation, l.delta);
    for(i = 0; i < l.batch; ++i){
        axpy_cpu(l.outputs, 1, l.delta + i*l.outputs, 1, l.bias_updates, 1);
    }
    if(l.batch_normalize){
        backward_scale_cpu(l.x_norm, l.delta, l.batch, l.outputs, 1, l.scale_updates);

        scale_bias(l.delta, l.scales, l.batch, l.outputs, 1);

        mean_delta_cpu(l.delta, l.variance, l.batch, l.outputs, 1, l.mean_delta);
        variance_delta_cpu(l.x, l.delta, l.mean, l.variance, l.batch, l.outputs, 1, l.variance_delta);
        normalize_delta_cpu(l.x, l.mean, l.variance, l.mean_delta, l.variance_delta, l.batch, l.outputs, 1, l.delta);
    }

    int m = l.outputs;
    int k = l.batch;
    int n = l.inputs;
    float *a = l.delta;
    float *b = state.input;
    float *c = l.weight_updates;
    gemm(1,0,m,n,k,1,a,m,b,n,1,c,n);

    m = l.batch;
    k = l.outputs;
    n = l.inputs;

    a = l.delta;
    b = l.weights;
    c = state.delta;

    if(c) gemm(0,0,m,n,k,1,a,k,b,n,1,c,n);
}


void denormalize_connected_layer(layer l)
{
    int i, j;
    for(i = 0; i < l.outputs; ++i){
        float scale = l.scales[i]/sqrt(l.rolling_variance[i] + .000001f);
        for(j = 0; j < l.inputs; ++j){
            l.weights[i*l.inputs + j] *= scale;
        }
        l.biases[i] -= l.rolling_mean[i] * scale;
        l.scales[i] = 1;
        l.rolling_mean[i] = 0;
        l.rolling_variance[i] = 1;
    }
}


void statistics_connected_layer(layer l)
{
    if(l.batch_normalize){
        printf("Scales ");
        print_statistics(l.scales, l.outputs);
        /*
        printf("Rolling Mean ");
        print_statistics(l.rolling_mean, l.outputs);
        printf("Rolling Variance ");
        print_statistics(l.rolling_variance, l.outputs);
        */
    }
    printf("Biases ");
    print_statistics(l.biases, l.outputs);
    printf("Weights ");
    print_statistics(l.weights, l.outputs);
}
