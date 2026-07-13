#include "ervp_malloc.h"

#include "normalization_layer.h"
#include "blas.h"
#include <stdio.h>

layer make_normalization_layer(int batch, int w, int h, int c, int size, float alpha, float beta, float kappa)
{
    printf("Local Response Normalization Layer: %d x %d x %d image, %d size\n", w,h,c,size);
    layer layer = { (LAYER_TYPE)0 };
    layer.type = NORMALIZATION;
    layer.batch = batch;
    layer.h = layer.out_h = h;
    layer.w = layer.out_w = w;
    layer.c = layer.out_c = c;
    layer.kappa = kappa;
    layer.size = size;
    layer.alpha = alpha;
    layer.beta = beta;
    layer.output = (float*)calloc(h * w * c * batch, sizeof(float));
    layer.delta = (float*)calloc(h * w * c * batch, sizeof(float));
    layer.squared = (float*)calloc(h * w * c * batch, sizeof(float));
    layer.norms = (float*)calloc(h * w * c * batch, sizeof(float));
    layer.inputs = w*h*c;
    layer.outputs = layer.inputs;

    layer.forward = forward_normalization_layer;
#ifdef USE_BACKWARD_LAYER
    layer.backward = backward_normalization_layer;
#else
    layer.backward = 0;
#endif
    return layer;
}

void resize_normalization_layer(layer *layer, int w, int h)
{
    int c = layer->c;
    int batch = layer->batch;
    layer->h = h;
    layer->w = w;
    layer->out_h = h;
    layer->out_w = w;
    layer->inputs = w*h*c;
    layer->outputs = layer->inputs;
    layer->output = (float*)realloc(layer->output, h * w * c * batch * sizeof(float));
    layer->delta = (float*)realloc(layer->delta, h * w * c * batch * sizeof(float));
    layer->squared = (float*)realloc(layer->squared, h * w * c * batch * sizeof(float));
    layer->norms = (float*)realloc(layer->norms, h * w * c * batch * sizeof(float));
}

void forward_normalization_layer(const layer layer, network_state state)
{
    int k,b;
    int w = layer.w;
    int h = layer.h;
    int c = layer.c;
    scal_cpu(w*h*c*layer.batch, 0, layer.squared, 1);

    for(b = 0; b < layer.batch; ++b){
        float *squared = layer.squared + w*h*c*b;
        float *norms   = layer.norms + w*h*c*b;
        float *input   = state.input + w*h*c*b;
        pow_cpu(w*h*c, 2, input, 1, squared, 1);

        const_cpu(w*h, layer.kappa, norms, 1);
        for(k = 0; k < layer.size/2; ++k){
            axpy_cpu(w*h, layer.alpha, squared + w*h*k, 1, norms, 1);
        }

        for(k = 1; k < layer.c; ++k){
            copy_cpu(w*h, norms + w*h*(k-1), 1, norms + w*h*k, 1);
            int prev = k - ((layer.size-1)/2) - 1;
            int next = k + (layer.size/2);
            if(prev >= 0)      axpy_cpu(w*h, -layer.alpha, squared + w*h*prev, 1, norms + w*h*k, 1);
            if(next < layer.c) axpy_cpu(w*h,  layer.alpha, squared + w*h*next, 1, norms + w*h*k, 1);
        }
    }
    pow_cpu(w*h*c*layer.batch, -layer.beta, layer.norms, 1, layer.output, 1);
    mul_cpu(w*h*c*layer.batch, state.input, 1, layer.output, 1);
}

void backward_normalization_layer(const layer layer, network_state state)
{
    // TODO This is approximate ;-)
    // Also this should add in to delta instead of overwritting.

    int w = layer.w;
    int h = layer.h;
    int c = layer.c;
    pow_cpu(w*h*c*layer.batch, -layer.beta, layer.norms, 1, state.delta, 1);
    mul_cpu(w*h*c*layer.batch, layer.delta, 1, state.delta, 1);
}
