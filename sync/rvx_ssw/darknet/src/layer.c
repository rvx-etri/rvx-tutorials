#include "ervp_malloc.h"
#include "ervp_printf.h"

#include "layer.h"

void free_sublayer(layer *l)
{
    if (l) {
        free_layer(*l);
        free(l);
    }
}

void free_layer(layer l)
{
    if (l.type == CONV_LSTM) {
        if (l.peephole) {
            free_sublayer(l.vf);
            free_sublayer(l.vi);
            free_sublayer(l.vo);
        }
        else {
            free(l.vf);
            free(l.vi);
            free(l.vo);
        }
        free_sublayer(l.wf);
        free_sublayer(l.wi);
        free_sublayer(l.wg);
        free_sublayer(l.wo);
        free_sublayer(l.uf);
        free_sublayer(l.ui);
        free_sublayer(l.ug);
        free_sublayer(l.uo);
    }
    if (l.type == CRNN) {
        free_sublayer(l.input_layer);
        free_sublayer(l.self_layer);
        free_sublayer(l.output_layer);
        l.output = NULL;
        l.delta = NULL;
    }
    if (l.type == DROPOUT) {
        if (l.rand)           free(l.rand);
        return;
    }
    if (l.mask)               free(l.mask);
    if (l.cweights)           free(l.cweights);
    if (l.indexes)            free(l.indexes);
    if (l.input_layers)       free(l.input_layers);
    if (l.input_sizes)        free(l.input_sizes);
    if (l.map)                free(l.map);
    if (l.rand)               free(l.rand);
    if (l.cost)               free(l.cost);
    if (l.state)              free(l.state);
    if (l.prev_state)         free(l.prev_state);
    if (l.forgot_state)       free(l.forgot_state);
    if (l.forgot_delta)       free(l.forgot_delta);
    if (l.state_delta)        free(l.state_delta);
    if (l.concat)             free(l.concat);
    if (l.concat_delta)       free(l.concat_delta);
    if (l.binary_weights)     free(l.binary_weights);
    if (l.biases)             free(l.biases);
    if (l.bias_updates)       free(l.bias_updates);
    if (l.scales)             free(l.scales);
    if (l.scale_updates)      free(l.scale_updates);
    if (l.weights)            free(l.weights);
    if (l.weight_updates)     free(l.weight_updates);
    if (l.align_bit_weights)  free(l.align_bit_weights);
    if (l.mean_arr)           free(l.mean_arr);
    if (l.delta)              free(l.delta);
    if (l.output)             free(l.output);
    if (l.squared)            free(l.squared);
    if (l.norms)              free(l.norms);
    if (l.spatial_mean)       free(l.spatial_mean);
    if (l.mean)               free(l.mean);
    if (l.variance)           free(l.variance);
    if (l.mean_delta)         free(l.mean_delta);
    if (l.variance_delta)     free(l.variance_delta);
    if (l.rolling_mean)       free(l.rolling_mean);
    if (l.rolling_variance)   free(l.rolling_variance);
    if (l.x)                  free(l.x);
    if (l.x_norm)             free(l.x_norm);
    if (l.m)                  free(l.m);
    if (l.v)                  free(l.v);
    if (l.z_cpu)              free(l.z_cpu);
    if (l.r_cpu)              free(l.r_cpu);
    if (l.binary_input)       free(l.binary_input);
    if (l.bin_re_packed_input) free(l.bin_re_packed_input);
    if (l.t_bit_input)        free(l.t_bit_input);
    if (l.loss)               free(l.loss);

    // CONV-LSTM
    if (l.f_cpu)               free(l.f_cpu);
    if (l.i_cpu)               free(l.i_cpu);
    if (l.g_cpu)               free(l.g_cpu);
    if (l.o_cpu)               free(l.o_cpu);
    if (l.c_cpu)               free(l.c_cpu);
    if (l.h_cpu)               free(l.h_cpu);
    if (l.temp_cpu)            free(l.temp_cpu);
    if (l.temp2_cpu)           free(l.temp2_cpu);
    if (l.temp3_cpu)           free(l.temp3_cpu);
    if (l.dc_cpu)              free(l.dc_cpu);
    if (l.dh_cpu)              free(l.dh_cpu);
    if (l.prev_state_cpu)      free(l.prev_state_cpu);
    if (l.prev_cell_cpu)       free(l.prev_cell_cpu);
    if (l.stored_c_cpu)        free(l.stored_c_cpu);
    if (l.stored_h_cpu)        free(l.stored_h_cpu);
    if (l.cell_cpu)            free(l.cell_cpu);
}
