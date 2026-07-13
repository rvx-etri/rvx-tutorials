#include "ervp_malloc.h"

#include "reorg_old_layer.h"
#include "blas.h"
#include <stdio.h>


layer make_reorg_old_layer(int batch, int w, int h, int c, int stride, int reverse)
{
    layer l = { (LAYER_TYPE)0 };
    l.type = REORG_OLD;
    l.batch = batch;
    l.stride = stride;
    l.h = h;
    l.w = w;
    l.c = c;
    if(reverse){
        l.out_w = w*stride;
        l.out_h = h*stride;
        l.out_c = c/(stride*stride);
    }else{
        l.out_w = w/stride;
        l.out_h = h/stride;
        l.out_c = c*(stride*stride);
    }
    l.reverse = reverse;
    printf("reorg_old              /%2d  %4d x%4d x%4d   ->  %4d x%4d x%4d\n",  stride, w, h, c, l.out_w, l.out_h, l.out_c);
    l.outputs = l.out_h * l.out_w * l.out_c;
    l.inputs = h*w*c;
    int output_size = l.out_h * l.out_w * l.out_c * batch;
    l.output = (float*)calloc(output_size, sizeof(float));
    l.delta = (float*)calloc(output_size, sizeof(float));

    l.forward = forward_reorg_old_layer;
#ifdef USE_BACKWARD_LAYER
    l.backward = backward_reorg_old_layer;
#else
    l.backward = 0;
#endif
    return l;
}

void resize_reorg_old_layer(layer *l, int w, int h)
{
    int stride = l->stride;
    int c = l->c;

    l->h = h;
    l->w = w;

    if(l->reverse){
        l->out_w = w*stride;
        l->out_h = h*stride;
        l->out_c = c/(stride*stride);
    }else{
        l->out_w = w/stride;
        l->out_h = h/stride;
        l->out_c = c*(stride*stride);
    }

    l->outputs = l->out_h * l->out_w * l->out_c;
    l->inputs = l->outputs;
    int output_size = l->outputs * l->batch;

    l->output = (float*)realloc(l->output, output_size * sizeof(float));
    l->delta = (float*)realloc(l->delta, output_size * sizeof(float));
}

void forward_reorg_old_layer(const layer l, network_state state)
{
    if (l.reverse) {
        reorg_cpu(state.input, l.w, l.h, l.c, l.batch, l.stride, 1, l.output);
    }
    else {
        reorg_cpu(state.input, l.w, l.h, l.c, l.batch, l.stride, 0, l.output);
    }
}

void backward_reorg_old_layer(const layer l, network_state state)
{
    if (l.reverse) {
        reorg_cpu(l.delta, l.w, l.h, l.c, l.batch, l.stride, 0, state.delta);
    }
    else {
        reorg_cpu(l.delta, l.w, l.h, l.c, l.batch, l.stride, 1, state.delta);
    }
}
