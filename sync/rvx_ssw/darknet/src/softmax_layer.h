#ifndef SOFTMAX_LAYER_H
#define SOFTMAX_LAYER_H
#include "layer.h"
#include "network.h"

typedef layer softmax_layer;

#ifdef __cplusplus
extern "C" {
#endif
void softmax_array(float *input, int n, float temp, float *output);
softmax_layer make_softmax_layer(int batch, int inputs, int groups);
softmax_layer make_quantized_softmax_layer(int batch, int inputs, int groups,int quantization_type);
void forward_softmax_layer(const softmax_layer l, network_state state);
void backward_softmax_layer(const softmax_layer l, network_state state);

#ifdef __cplusplus
}
#endif
#endif
