#ifndef CONNECTED_LAYER_H
#define CONNECTED_LAYER_H

#include "activations.h"
#include "layer.h"
#include "network.h"

typedef layer connected_layer;

#ifdef __cplusplus
extern "C" {
#endif
connected_layer make_connected_layer(int batch, int steps, int inputs, int outputs, ACTIVATION activation, int batch_normalize);
connected_layer make_quantized_connected_layer(int batch, int steps, int inputs, int outputs, ACTIVATION activation, int batch_normalize,int quantization_type);

void forward_connected_layer(connected_layer layer, network_state state);
void backward_connected_layer(connected_layer layer, network_state state);
void update_connected_layer(connected_layer layer, int batch, float learning_rate, float momentum, float decay);
void denormalize_connected_layer(layer l);
void statistics_connected_layer(layer l);

#ifdef __cplusplus
}
#endif

#endif
