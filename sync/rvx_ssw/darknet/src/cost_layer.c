#include "ervp_malloc.h"
#include "ervp_printf.h"

#include "cost_layer.h"
#include "utils.h"
#include "blas.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

COST_TYPE get_cost_type(char *s)
{
    if (strcmp(s, "sse")==0) return SSE;
    if (strcmp(s, "masked")==0) return MASKED;
    if (strcmp(s, "smooth")==0) return SMOOTH;
    printf("Couldn't find cost type %s, going with SSE\n", s);
    return SSE;
}

char *get_cost_string(COST_TYPE a)
{
    switch(a){
        case SSE:
            return "sse";
        case MASKED:
            return "masked";
        case SMOOTH:
            return "smooth";
		default:
			return "sse";
    }
}

cost_layer make_cost_layer(int batch, int inputs, COST_TYPE cost_type, float scale)
{
    printf("cost                                           %4d\n",  inputs);
    cost_layer l = { (LAYER_TYPE)0 };
    l.type = COST;

    l.scale = scale;
    l.batch = batch;
    l.inputs = inputs;
    l.outputs = inputs;
    l.cost_type = cost_type;
    l.delta = (float*)calloc(inputs * batch, sizeof(float));
    l.output = (float*)calloc(inputs * batch, sizeof(float));
    l.cost = (float*)calloc(1, sizeof(float));

    l.forward = forward_cost_layer;
#ifdef USE_BACKWARD_LAYER
    l.backward = backward_cost_layer;
#else
    l.backward = 0;
#endif
    return l;
}

void resize_cost_layer(cost_layer *l, int inputs)
{
    l->inputs = inputs;
    l->outputs = inputs;
    l->delta = (float*)realloc(l->delta, inputs * l->batch * sizeof(float));
    l->output = (float*)realloc(l->output, inputs * l->batch * sizeof(float));
}

void forward_cost_layer(cost_layer l, network_state state)
{
    if (!state.truth) return;
    if(l.cost_type == MASKED){
        int i;
        for(i = 0; i < l.batch*l.inputs; ++i){
            if(state.truth[i] == SECRET_NUM) state.input[i] = SECRET_NUM;
        }
    }
    if(l.cost_type == SMOOTH){
        smooth_l1_cpu(l.batch*l.inputs, state.input, state.truth, l.delta, l.output);
    } else {
        l2_cpu(l.batch*l.inputs, state.input, state.truth, l.delta, l.output);
    }
    l.cost[0] = sum_array(l.output, l.batch*l.inputs);
}

void backward_cost_layer(const cost_layer l, network_state state)
{
    axpy_cpu(l.batch*l.inputs, l.scale, l.delta, 1, state.delta, 1);
}
