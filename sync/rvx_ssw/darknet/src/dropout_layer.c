#include "ervp_malloc.h"
#include "ervp_printf.h"

#include "dropout_layer.h"
#include "utils.h"
#include <stdio.h>

dropout_layer make_dropout_layer(int batch, int inputs, float probability)
{
    dropout_layer l = { (LAYER_TYPE)0 };
    l.type = DROPOUT;
    l.probability = probability;
    l.inputs = inputs;
    l.outputs = inputs;
    l.batch = batch;
    l.rand = (float*)calloc(inputs * batch, sizeof(float));
    l.scale = 1./(1.-probability);
    l.forward = forward_dropout_layer;
#ifdef USE_BACKWARD_LAYER
    l.backward = backward_dropout_layer;
#else
    l.backward = 0;
#endif
    printf("dropout       p = %.2f               %4d  ->  %4d\n", probability, inputs, inputs);
    return l;
}

void resize_dropout_layer(dropout_layer *l, int inputs)
{
    l->rand = (float*)realloc(l->rand, l->inputs * l->batch * sizeof(float));
}

void forward_dropout_layer(dropout_layer l, network_state state)
{
    int i;
    if (!state.train) return;
    for(i = 0; i < l.batch * l.inputs; ++i){
        float r = rand_uniform(0, 1);
        l.rand[i] = r;
        if(r < l.probability) state.input[i] = 0;
        else state.input[i] *= l.scale;
    }
}

void backward_dropout_layer(dropout_layer l, network_state state)
{
    int i;
    if(!state.delta) return;
    for(i = 0; i < l.batch * l.inputs; ++i){
        float r = l.rand[i];
        if(r < l.probability) state.delta[i] = 0;
        else state.delta[i] *= l.scale;
    }
}
