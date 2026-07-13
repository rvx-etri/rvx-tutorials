#include "ervp_malloc.h"
#include "ervp_assert.h"
#include "ervp_printf.h"

#include "softmax_layer.h"
#include "blas.h"
#include "utils.h"
#include "blas.h"

#include <float.h>
#include <math.h>
#include <stdio.h>

#define SECRET_NUM -1234

void softmax_tree(float *input, int batch, int inputs, float temp, tree *hierarchy, float *output)
{
	int b;
	for (b = 0; b < batch; ++b) {
		int i;
		int count = 0;
		for (i = 0; i < hierarchy->groups; ++i) {
			int group_size = hierarchy->group_size[i];
			softmax(input + b*inputs + count, group_size, temp, output + b*inputs + count, 1);
			count += group_size;
		}
	}
}

softmax_layer make_softmax_layer(int batch, int inputs, int groups)
{
    assert(inputs%groups == 0);
    printf("softmax                                        %4d\n",  inputs);
    softmax_layer l = { (LAYER_TYPE)0 };
    l.type = SOFTMAX;
    l.batch = batch;
    l.groups = groups;
    l.inputs = inputs;
    l.outputs = inputs;
    l.loss = (float*)calloc(inputs * batch, sizeof(float));
    l.output = (float*)calloc(inputs * batch, sizeof(float));
    l.delta = (float*)calloc(inputs * batch, sizeof(float));
    l.cost = (float*)calloc(1, sizeof(float));

    l.forward = forward_softmax_layer;
#ifdef USE_BACKWARD_LAYER
    l.backward = backward_softmax_layer;
#else
    l.backward = 0;
#endif
    return l;
}
softmax_layer make_quantized_softmax_layer(int batch, int inputs, int groups,int quantization_type)
{
    assert(inputs%groups == 0);
    printf("softmax                                        %4d\n",  inputs);
    softmax_layer l = { (LAYER_TYPE)0 };
    l.quantization_type=quantization_type;
    l.type = SOFTMAX;
    l.batch = batch;
    l.groups = groups;
    l.inputs = inputs;
    l.outputs = inputs;
    l.loss = (float*)calloc(inputs * batch, sizeof(float));
    l.output = (float*)calloc(inputs * batch, sizeof(float));
    l.delta = (float*)calloc(inputs * batch, sizeof(float));
    l.cost = (float*)calloc(1, sizeof(float));

    l.forward = forward_softmax_layer;
#ifdef USE_BACKWARD_LAYER
    l.backward = backward_softmax_layer;
#else
    l.backward = 0;
#endif
    return l;
}
void forward_softmax_layer(const softmax_layer l, network_state net)
{
    if(l.softmax_tree){
        int i;
        int count = 0;
        for (i = 0; i < l.softmax_tree->groups; ++i) {
            int group_size = l.softmax_tree->group_size[i];
            softmax_cpu(net.input + count, group_size, l.batch, l.inputs, 1, 0, 1, l.temperature, l.output + count);
            count += group_size;
        }
    } else {
        if(l.quantization_type){
            printf("q_softmax\n");
            quantized_softmax_cpu(net.quantized_input, l.inputs/l.groups, l.batch, l.inputs, l.groups, l.inputs/l.groups, 1, l.temperature, l.output);
            printf("q_softmax done\n");
        }
        else{
        softmax_cpu(net.input, l.inputs/l.groups, l.batch, l.inputs, l.groups, l.inputs/l.groups, 1, l.temperature, l.output);
        }
    }

    if(net.truth && !l.noloss){
        softmax_x_ent_cpu(l.batch*l.inputs, l.output, net.truth, l.delta, l.loss);
        l.cost[0] = sum_array(l.loss, l.batch*l.inputs);
    }
}

void backward_softmax_layer(const softmax_layer l, network_state net)
{
    axpy_cpu(l.inputs*l.batch, 1, l.delta, 1, net.delta, 1);
}
