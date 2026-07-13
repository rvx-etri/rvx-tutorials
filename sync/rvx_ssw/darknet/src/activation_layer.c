#include "ervp_malloc.h"
#include "ervp_printf.h"

#include "activation_layer.h"
#include "utils.h"
#include "blas.h"
#include "gemm.h"

#include <math.h>
#include <stdio.h>

#include "ervp_malloc.h"
#include "ervp_printf.h"

layer make_activation_layer(int batch, int inputs, ACTIVATION activation)
{
    layer l = { (LAYER_TYPE)0 };
    l.type = ACTIVE;

    l.inputs = inputs;
    l.outputs = inputs;
    l.batch=batch;

    l.output = (float*)calloc(batch * inputs, sizeof(float));
    l.delta = (float*)calloc(batch * inputs, sizeof(float));

    l.forward = forward_activation_layer;
#ifdef USE_BACKWARD_LAYER
    l.backward = backward_activation_layer;
#else
    l.backward = 0;
#endif

    l.activation = activation;
    printf("Activation Layer: %d inputs\n", inputs);
    return l;
}

void forward_activation_layer(layer l, network_state state)
{
    copy_cpu(l.outputs*l.batch, state.input, 1, l.output, 1);
    activate_array(l.output, l.outputs*l.batch, l.activation);
}

void backward_activation_layer(layer l, network_state state)
{
    gradient_array(l.output, l.outputs*l.batch, l.activation, l.delta);
    copy_cpu(l.outputs*l.batch, l.delta, 1, state.delta, 1);
}
