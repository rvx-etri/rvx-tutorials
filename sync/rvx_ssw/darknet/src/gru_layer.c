#include "ervp_malloc.h"
#include "ervp_printf.h"

#include "gru_layer.h"
#include "connected_layer.h"
#include "utils.h"
#include "blas.h"
#include "gemm.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static void increment_layer(layer *l, int steps)
{
    int num = l->outputs*l->batch*steps;
    l->output += num;
    l->delta += num;
    l->x += num;
    l->x_norm += num;
}

layer make_gru_layer(int batch, int inputs, int outputs, int steps, int batch_normalize)
{
    printf("GRU Layer: %d inputs, %d outputs\n", inputs, outputs);
    batch = batch / steps;
    layer l = { (LAYER_TYPE)0 };
    l.batch = batch;
    l.type = GRU;
    l.steps = steps;
    l.inputs = inputs;

    l.input_z_layer = (layer*)malloc(sizeof(layer));
    printf("\t\t");
    *(l.input_z_layer) = make_connected_layer(batch, steps, inputs, outputs, LINEAR, batch_normalize);
    l.input_z_layer->batch = batch;

    l.state_z_layer = (layer*)malloc(sizeof(layer));
    printf("\t\t");
    *(l.state_z_layer) = make_connected_layer(batch, steps, outputs, outputs, LINEAR, batch_normalize);
    l.state_z_layer->batch = batch;



    l.input_r_layer = (layer*)malloc(sizeof(layer));
    printf("\t\t");
    *(l.input_r_layer) = make_connected_layer(batch, steps, inputs, outputs, LINEAR, batch_normalize);
    l.input_r_layer->batch = batch;

    l.state_r_layer = (layer*)malloc(sizeof(layer));
    printf("\t\t");
    *(l.state_r_layer) = make_connected_layer(batch, steps, outputs, outputs, LINEAR, batch_normalize);
    l.state_r_layer->batch = batch;



    l.input_h_layer = (layer*)malloc(sizeof(layer));
    printf("\t\t");
    *(l.input_h_layer) = make_connected_layer(batch, steps, inputs, outputs, LINEAR, batch_normalize);
    l.input_h_layer->batch = batch;

    l.state_h_layer = (layer*)malloc(sizeof(layer));
    printf("\t\t");
    *(l.state_h_layer) = make_connected_layer(batch, steps, outputs, outputs, LINEAR, batch_normalize);
    l.state_h_layer->batch = batch;

    l.batch_normalize = batch_normalize;


    l.outputs = outputs;
    l.output = (float*)calloc(outputs * batch * steps, sizeof(float));
    l.delta = (float*)calloc(outputs * batch * steps, sizeof(float));
    l.state = (float*)calloc(outputs * batch, sizeof(float));
    l.prev_state = (float*)calloc(outputs * batch, sizeof(float));
    l.forgot_state = (float*)calloc(outputs * batch, sizeof(float));
    l.forgot_delta = (float*)calloc(outputs * batch, sizeof(float));

    l.r_cpu = (float*)calloc(outputs * batch, sizeof(float));
    l.z_cpu = (float*)calloc(outputs * batch, sizeof(float));
    l.h_cpu = (float*)calloc(outputs * batch, sizeof(float));

    l.forward = forward_gru_layer;
#ifdef USE_BACKWARD_LAYER
    l.backward = backward_gru_layer;
#else
    l.backward = 0;
#endif
#ifdef USE_UPDATE_LAYER
    l.update = update_gru_layer;
#else
    l.update = 0;
#endif

    return l;
}

void update_gru_layer(layer l, int batch, float learning_rate, float momentum, float decay)
{
    update_connected_layer(*(l.input_layer), batch, learning_rate, momentum, decay);
    update_connected_layer(*(l.self_layer), batch, learning_rate, momentum, decay);
    update_connected_layer(*(l.output_layer), batch, learning_rate, momentum, decay);
}

void forward_gru_layer(layer l, network_state state)
{
    network_state s = {0};
    s.train = state.train;
    s.workspace = state.workspace;
    int i;
    layer input_z_layer = *(l.input_z_layer);
    layer input_r_layer = *(l.input_r_layer);
    layer input_h_layer = *(l.input_h_layer);

    layer state_z_layer = *(l.state_z_layer);
    layer state_r_layer = *(l.state_r_layer);
    layer state_h_layer = *(l.state_h_layer);

    fill_cpu(l.outputs * l.batch * l.steps, 0, input_z_layer.delta, 1);
    fill_cpu(l.outputs * l.batch * l.steps, 0, input_r_layer.delta, 1);
    fill_cpu(l.outputs * l.batch * l.steps, 0, input_h_layer.delta, 1);

    fill_cpu(l.outputs * l.batch * l.steps, 0, state_z_layer.delta, 1);
    fill_cpu(l.outputs * l.batch * l.steps, 0, state_r_layer.delta, 1);
    fill_cpu(l.outputs * l.batch * l.steps, 0, state_h_layer.delta, 1);
    if(state.train) {
        fill_cpu(l.outputs * l.batch * l.steps, 0, l.delta, 1);
        copy_cpu(l.outputs*l.batch, l.state, 1, l.prev_state, 1);
    }

    for (i = 0; i < l.steps; ++i) {
        s.input = l.state;
        forward_connected_layer(state_z_layer, s);
        forward_connected_layer(state_r_layer, s);

        s.input = state.input;
        forward_connected_layer(input_z_layer, s);
        forward_connected_layer(input_r_layer, s);
        forward_connected_layer(input_h_layer, s);


        copy_cpu(l.outputs*l.batch, input_z_layer.output, 1, l.z_cpu, 1);
        axpy_cpu(l.outputs*l.batch, 1, state_z_layer.output, 1, l.z_cpu, 1);

        copy_cpu(l.outputs*l.batch, input_r_layer.output, 1, l.r_cpu, 1);
        axpy_cpu(l.outputs*l.batch, 1, state_r_layer.output, 1, l.r_cpu, 1);

        activate_array(l.z_cpu, l.outputs*l.batch, LOGISTIC);
        activate_array(l.r_cpu, l.outputs*l.batch, LOGISTIC);

        copy_cpu(l.outputs*l.batch, l.state, 1, l.forgot_state, 1);
        mul_cpu(l.outputs*l.batch, l.r_cpu, 1, l.forgot_state, 1);

        s.input = l.forgot_state;
        forward_connected_layer(state_h_layer, s);

        copy_cpu(l.outputs*l.batch, input_h_layer.output, 1, l.h_cpu, 1);
        axpy_cpu(l.outputs*l.batch, 1, state_h_layer.output, 1, l.h_cpu, 1);

        #ifdef USET
        activate_array(l.h_cpu, l.outputs*l.batch, TANH);
        #else
        activate_array(l.h_cpu, l.outputs*l.batch, LOGISTIC);
        #endif

        weighted_sum_cpu(l.state, l.h_cpu, l.z_cpu, l.outputs*l.batch, l.output);

        copy_cpu(l.outputs*l.batch, l.output, 1, l.state, 1);

        state.input += l.inputs*l.batch;
        l.output += l.outputs*l.batch;
        increment_layer(&input_z_layer, 1);
        increment_layer(&input_r_layer, 1);
        increment_layer(&input_h_layer, 1);

        increment_layer(&state_z_layer, 1);
        increment_layer(&state_r_layer, 1);
        increment_layer(&state_h_layer, 1);
    }
}

void backward_gru_layer(layer l, network_state state)
{
}
