// Page 4: https://arxiv.org/abs/1506.04214v2
// Page 3: https://arxiv.org/pdf/1705.06368v3.pdf
// https://wikimedia.org/api/rest_v1/media/math/render/svg/1edbece2559479959fe829e9c6657efb380debe7

#include "ervp_malloc.h"
#include "ervp_printf.h"

#include "conv_lstm_layer.h"
#include "connected_layer.h"
#include "convolutional_layer.h"
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


layer make_conv_lstm_layer(int batch, int h, int w, int c, int output_filters, int groups, int steps, int size, int stride, int pad, ACTIVATION activation, int batch_normalize, int peephole, int xnor)
{
    printf("CONV_LSTM Layer: %d x %d x %d image, %d filters\n", h, w, c, output_filters);
    /*
    batch = batch / steps;
    layer l = { (LAYER_TYPE)0 };
    l.batch = batch;
    l.type = LSTM;
    l.steps = steps;
    l.inputs = inputs;
    l.out_w = 1;
    l.out_h = 1;
    l.out_c = outputs;
    */
    batch = batch / steps;
    layer l = { (LAYER_TYPE)0 };
    l.batch = batch;
    l.type = CONV_LSTM;
    l.steps = steps;
    l.size = size;
    l.stride = stride;
    l.pad = pad;
    l.h = h;
    l.w = w;
    l.c = c;
    l.groups = groups;
    l.out_c = output_filters;
    l.inputs = h * w * c;
    l.xnor = xnor;
    l.peephole = peephole;

    // U
    l.uf = (layer*)calloc(1, sizeof(layer));
    *(l.uf) = make_convolutional_layer(batch, steps, h, w, c, output_filters, groups, size, stride, pad, activation, batch_normalize, 0, xnor, 0, 0, 0);
    l.uf->batch = batch;
    if (l.workspace_size < l.uf->workspace_size) l.workspace_size = l.uf->workspace_size;

    l.ui = (layer*)calloc(1, sizeof(layer));
    *(l.ui) = make_convolutional_layer(batch, steps, h, w, c, output_filters, groups, size, stride, pad, activation, batch_normalize, 0, xnor, 0, 0, 0);
    l.ui->batch = batch;
    if (l.workspace_size < l.ui->workspace_size) l.workspace_size = l.ui->workspace_size;

    l.ug = (layer*)calloc(1, sizeof(layer));
    *(l.ug) = make_convolutional_layer(batch, steps, h, w, c, output_filters, groups, size, stride, pad, activation, batch_normalize, 0, xnor, 0, 0, 0);
    l.ug->batch = batch;
    if (l.workspace_size < l.ug->workspace_size) l.workspace_size = l.ug->workspace_size;

    l.uo = (layer*)calloc(1, sizeof(layer));
    *(l.uo) = make_convolutional_layer(batch, steps, h, w, c, output_filters, groups, size, stride, pad, activation, batch_normalize, 0, xnor, 0, 0, 0);
    l.uo->batch = batch;
    if (l.workspace_size < l.uo->workspace_size) l.workspace_size = l.uo->workspace_size;


    // W
    l.wf = (layer*)calloc(1, sizeof(layer));
    *(l.wf) = make_convolutional_layer(batch, steps, h, w, output_filters, output_filters, groups, size, stride, pad, activation, batch_normalize, 0, xnor, 0, 0, 0);
    l.wf->batch = batch;
    if (l.workspace_size < l.wf->workspace_size) l.workspace_size = l.wf->workspace_size;

    l.wi = (layer*)calloc(1, sizeof(layer));
    *(l.wi) = make_convolutional_layer(batch, steps, h, w, output_filters, output_filters, groups, size, stride, pad, activation, batch_normalize, 0, xnor, 0, 0, 0);
    l.wi->batch = batch;
    if (l.workspace_size < l.wi->workspace_size) l.workspace_size = l.wi->workspace_size;

    l.wg = (layer*)calloc(1, sizeof(layer));
    *(l.wg) = make_convolutional_layer(batch, steps, h, w, output_filters, output_filters, groups, size, stride, pad, activation, batch_normalize, 0, xnor, 0, 0, 0);
    l.wg->batch = batch;
    if (l.workspace_size < l.wg->workspace_size) l.workspace_size = l.wg->workspace_size;

    l.wo = (layer*)calloc(1, sizeof(layer));
    *(l.wo) = make_convolutional_layer(batch, steps, h, w, output_filters, output_filters, groups, size, stride, pad, activation, batch_normalize, 0, xnor, 0, 0, 0);
    l.wo->batch = batch;
    if (l.workspace_size < l.wo->workspace_size) l.workspace_size = l.wo->workspace_size;


    // V
    l.vf = (layer*)calloc(1, sizeof(layer));
    if (l.peephole) {
        *(l.vf) = make_convolutional_layer(batch, steps, h, w, output_filters, output_filters, groups, size, stride, pad, activation, batch_normalize, 0, xnor, 0, 0, 0);
        l.vf->batch = batch;
        if (l.workspace_size < l.vf->workspace_size) l.workspace_size = l.vf->workspace_size;
    }

    l.vi = (layer*)calloc(1, sizeof(layer));
    if (l.peephole) {
        *(l.vi) = make_convolutional_layer(batch, steps, h, w, output_filters, output_filters, groups, size, stride, pad, activation, batch_normalize, 0, xnor, 0, 0, 0);
        l.vi->batch = batch;
        if (l.workspace_size < l.vi->workspace_size) l.workspace_size = l.vi->workspace_size;
    }

    l.vo = (layer*)calloc(1, sizeof(layer));
    if (l.peephole) {
        *(l.vo) = make_convolutional_layer(batch, steps, h, w, output_filters, output_filters, groups, size, stride, pad, activation, batch_normalize, 0, xnor, 0, 0, 0);
        l.vo->batch = batch;
        if (l.workspace_size < l.vo->workspace_size) l.workspace_size = l.vo->workspace_size;
    }


    l.batch_normalize = batch_normalize;

    l.out_h = l.wo->out_h;
    l.out_w = l.wo->out_w;
    l.outputs = l.wo->outputs;
    int outputs = l.outputs;
    l.inputs = w*h*c;

    assert(l.wo->outputs == l.uo->outputs);

    l.output = (float*)calloc(outputs * batch * steps, sizeof(float));
    //l.state = (float*)calloc(outputs * batch, sizeof(float));

    l.forward = forward_conv_lstm_layer;
#ifdef USE_UPDATE_LAYER
    l.update = update_conv_lstm_layer;
#else
    l.update = 0;
#endif
#ifdef USE_BACKWARD_LAYER
    l.backward = backward_conv_lstm_layer;
#else
    l.backward = 0;
#endif

    l.prev_state_cpu =  (float*)calloc(batch*outputs, sizeof(float));
    l.prev_cell_cpu =   (float*)calloc(batch*outputs, sizeof(float));
    l.cell_cpu =        (float*)calloc(batch*outputs*steps, sizeof(float));

    l.f_cpu =           (float*)calloc(batch*outputs, sizeof(float));
    l.i_cpu =           (float*)calloc(batch*outputs, sizeof(float));
    l.g_cpu =           (float*)calloc(batch*outputs, sizeof(float));
    l.o_cpu =           (float*)calloc(batch*outputs, sizeof(float));
    l.c_cpu =           (float*)calloc(batch*outputs, sizeof(float));
    l.stored_c_cpu = (float*)calloc(batch*outputs, sizeof(float));
    l.h_cpu =           (float*)calloc(batch*outputs, sizeof(float));
    l.stored_h_cpu = (float*)calloc(batch*outputs, sizeof(float));
    l.temp_cpu =        (float*)calloc(batch*outputs, sizeof(float));
    l.temp2_cpu =       (float*)calloc(batch*outputs, sizeof(float));
    l.temp3_cpu =       (float*)calloc(batch*outputs, sizeof(float));
    l.dc_cpu =          (float*)calloc(batch*outputs, sizeof(float));
    l.dh_cpu =          (float*)calloc(batch*outputs, sizeof(float));

    l.bflops = l.uf->bflops + l.ui->bflops + l.ug->bflops + l.uo->bflops +
        l.wf->bflops + l.wi->bflops + l.wg->bflops + l.wo->bflops +
        l.vf->bflops + l.vi->bflops + l.vo->bflops;

    if(l.peephole) l.bflops += 12 * l.outputs*l.batch / 1000000000.;
    else l.bflops += 9 * l.outputs*l.batch / 1000000000.;

    return l;
}

void update_conv_lstm_layer(layer l, int batch, float learning_rate, float momentum, float decay)
{
    if (l.peephole) {
        update_convolutional_layer(*(l.vf), batch, learning_rate, momentum, decay);
        update_convolutional_layer(*(l.vi), batch, learning_rate, momentum, decay);
        update_convolutional_layer(*(l.vo), batch, learning_rate, momentum, decay);
    }
    update_convolutional_layer(*(l.wf), batch, learning_rate, momentum, decay);
    update_convolutional_layer(*(l.wi), batch, learning_rate, momentum, decay);
    update_convolutional_layer(*(l.wg), batch, learning_rate, momentum, decay);
    update_convolutional_layer(*(l.wo), batch, learning_rate, momentum, decay);
    update_convolutional_layer(*(l.uf), batch, learning_rate, momentum, decay);
    update_convolutional_layer(*(l.ui), batch, learning_rate, momentum, decay);
    update_convolutional_layer(*(l.ug), batch, learning_rate, momentum, decay);
    update_convolutional_layer(*(l.uo), batch, learning_rate, momentum, decay);
}

void resize_conv_lstm_layer(layer *l, int w, int h)
{
    if (l->peephole) {
        resize_convolutional_layer(l->vf, w, h);
        if (l->workspace_size < l->vf->workspace_size) l->workspace_size = l->vf->workspace_size;

        resize_convolutional_layer(l->vi, w, h);
        if (l->workspace_size < l->vi->workspace_size) l->workspace_size = l->vi->workspace_size;

        resize_convolutional_layer(l->vo, w, h);
        if (l->workspace_size < l->vo->workspace_size) l->workspace_size = l->vo->workspace_size;
    }

    resize_convolutional_layer(l->wf, w, h);
    if (l->workspace_size < l->wf->workspace_size) l->workspace_size = l->wf->workspace_size;

    resize_convolutional_layer(l->wi, w, h);
    if (l->workspace_size < l->wi->workspace_size) l->workspace_size = l->wi->workspace_size;

    resize_convolutional_layer(l->wg, w, h);
    if (l->workspace_size < l->wg->workspace_size) l->workspace_size = l->wg->workspace_size;

    resize_convolutional_layer(l->wo, w, h);
    if (l->workspace_size < l->wo->workspace_size) l->workspace_size = l->wo->workspace_size;


    resize_convolutional_layer(l->uf, w, h);
    if (l->workspace_size < l->uf->workspace_size) l->workspace_size = l->uf->workspace_size;

    resize_convolutional_layer(l->ui, w, h);
    if (l->workspace_size < l->ui->workspace_size) l->workspace_size = l->ui->workspace_size;

    resize_convolutional_layer(l->ug, w, h);
    if (l->workspace_size < l->ug->workspace_size) l->workspace_size = l->ug->workspace_size;

    resize_convolutional_layer(l->uo, w, h);
    if (l->workspace_size < l->uo->workspace_size) l->workspace_size = l->uo->workspace_size;

    l->w = w;
    l->h = h;
    l->out_h = l->wo->out_h;
    l->out_w = l->wo->out_w;
    l->outputs = l->wo->outputs;
    int outputs = l->outputs;
    l->inputs = w*h*l->c;
    int steps = l->steps;
    int batch = l->batch;

    assert(l->wo->outputs == l->uo->outputs);

    l->output = (float*)realloc(l->output, outputs * batch * steps * sizeof(float));
    //l->state = (float*)realloc(l->state, outputs * batch * sizeof(float));

    l->prev_state_cpu = (float*)realloc(l->prev_state_cpu, batch*outputs * sizeof(float));
    l->prev_cell_cpu = (float*)realloc(l->prev_cell_cpu, batch*outputs * sizeof(float));
    l->cell_cpu = (float*)realloc(l->cell_cpu, batch*outputs*steps * sizeof(float));

    l->f_cpu = (float*)realloc(l->f_cpu, batch*outputs * sizeof(float));
    l->i_cpu = (float*)realloc(l->i_cpu, batch*outputs * sizeof(float));
    l->g_cpu = (float*)realloc(l->g_cpu, batch*outputs * sizeof(float));
    l->o_cpu = (float*)realloc(l->o_cpu, batch*outputs * sizeof(float));
    l->c_cpu = (float*)realloc(l->c_cpu, batch*outputs * sizeof(float));
    l->h_cpu = (float*)realloc(l->h_cpu, batch*outputs * sizeof(float));
    l->temp_cpu = (float*)realloc(l->temp_cpu, batch*outputs * sizeof(float));
    l->temp2_cpu = (float*)realloc(l->temp2_cpu, batch*outputs * sizeof(float));
    l->temp3_cpu = (float*)realloc(l->temp3_cpu, batch*outputs * sizeof(float));
    l->dc_cpu = (float*)realloc(l->dc_cpu, batch*outputs * sizeof(float));
    l->dh_cpu = (float*)realloc(l->dh_cpu, batch*outputs * sizeof(float));
    l->stored_c_cpu = (float*)realloc(l->stored_c_cpu, batch*outputs * sizeof(float));
    l->stored_h_cpu = (float*)realloc(l->stored_h_cpu, batch*outputs * sizeof(float));
}

void free_state_conv_lstm(layer l)
{
    int i;
    for (i = 0; i < l.outputs * l.batch; ++i) l.h_cpu[i] = 0;
    for (i = 0; i < l.outputs * l.batch; ++i) l.c_cpu[i] = 0;
}

void randomize_state_conv_lstm(layer l)
{
    int i;
    for (i = 0; i < l.outputs * l.batch; ++i) l.h_cpu[i] = rand_uniform(-1, 1);
    for (i = 0; i < l.outputs * l.batch; ++i) l.c_cpu[i] = rand_uniform(-1, 1);
}


void remember_state_conv_lstm(layer l)
{
    memcpy(l.stored_c_cpu, l.c_cpu, l.outputs * l.batch * sizeof(float));
    memcpy(l.stored_h_cpu, l.h_cpu, l.outputs * l.batch * sizeof(float));
}

void restore_state_conv_lstm(layer l)
{
    memcpy(l.c_cpu, l.stored_c_cpu, l.outputs * l.batch * sizeof(float));
    memcpy(l.h_cpu, l.stored_h_cpu, l.outputs * l.batch * sizeof(float));
}

void forward_conv_lstm_layer(layer l, network_state state)
{
    network_state s = { 0 };
    s.train = state.train;
    s.workspace = state.workspace;
    s.net = state.net;
    int i;
    layer vf = *(l.vf);
    layer vi = *(l.vi);
    layer vo = *(l.vo);

    layer wf = *(l.wf);
    layer wi = *(l.wi);
    layer wg = *(l.wg);
    layer wo = *(l.wo);

    layer uf = *(l.uf);
    layer ui = *(l.ui);
    layer ug = *(l.ug);
    layer uo = *(l.uo);

    if (state.train) {
        if (l.peephole) {
            fill_cpu(l.outputs * l.batch * l.steps, 0, vf.delta, 1);
            fill_cpu(l.outputs * l.batch * l.steps, 0, vi.delta, 1);
            fill_cpu(l.outputs * l.batch * l.steps, 0, vo.delta, 1);
        }

        fill_cpu(l.outputs * l.batch * l.steps, 0, wf.delta, 1);
        fill_cpu(l.outputs * l.batch * l.steps, 0, wi.delta, 1);
        fill_cpu(l.outputs * l.batch * l.steps, 0, wg.delta, 1);
        fill_cpu(l.outputs * l.batch * l.steps, 0, wo.delta, 1);

        fill_cpu(l.outputs * l.batch * l.steps, 0, uf.delta, 1);
        fill_cpu(l.outputs * l.batch * l.steps, 0, ui.delta, 1);
        fill_cpu(l.outputs * l.batch * l.steps, 0, ug.delta, 1);
        fill_cpu(l.outputs * l.batch * l.steps, 0, uo.delta, 1);

        fill_cpu(l.outputs * l.batch * l.steps, 0, l.delta, 1);
    }

    for (i = 0; i < l.steps; ++i)
    {
        if (l.peephole) {
            assert(l.outputs == vf.out_w * vf.out_h * vf.out_c);
            s.input = l.c_cpu;
            forward_convolutional_layer(vf, s);
            forward_convolutional_layer(vi, s);
            // vo below
        }

        assert(l.outputs == wf.out_w * wf.out_h * wf.out_c);
        assert(wf.c == l.out_c && wi.c == l.out_c && wg.c == l.out_c && wo.c == l.out_c);

        s.input = l.h_cpu;
        forward_convolutional_layer(wf, s);
        forward_convolutional_layer(wi, s);
        forward_convolutional_layer(wg, s);
        forward_convolutional_layer(wo, s);

        assert(l.inputs == uf.w * uf.h * uf.c);
        assert(uf.c == l.c && ui.c == l.c && ug.c == l.c && uo.c == l.c);

        s.input = state.input;
        forward_convolutional_layer(uf, s);
        forward_convolutional_layer(ui, s);
        forward_convolutional_layer(ug, s);
        forward_convolutional_layer(uo, s);

        // f = wf + uf + vf
        copy_cpu(l.outputs*l.batch, wf.output, 1, l.f_cpu, 1);
        axpy_cpu(l.outputs*l.batch, 1, uf.output, 1, l.f_cpu, 1);
        if (l.peephole) axpy_cpu(l.outputs*l.batch, 1, vf.output, 1, l.f_cpu, 1);

        // i = wi + ui + vi
        copy_cpu(l.outputs*l.batch, wi.output, 1, l.i_cpu, 1);
        axpy_cpu(l.outputs*l.batch, 1, ui.output, 1, l.i_cpu, 1);
        if (l.peephole) axpy_cpu(l.outputs*l.batch, 1, vi.output, 1, l.i_cpu, 1);

        // g = wg + ug
        copy_cpu(l.outputs*l.batch, wg.output, 1, l.g_cpu, 1);
        axpy_cpu(l.outputs*l.batch, 1, ug.output, 1, l.g_cpu, 1);

        activate_array(l.f_cpu, l.outputs*l.batch, LOGISTIC);
        activate_array(l.i_cpu, l.outputs*l.batch, LOGISTIC);
        activate_array(l.g_cpu, l.outputs*l.batch, TANH);

        // c = f*c + i*g
        copy_cpu(l.outputs*l.batch, l.i_cpu, 1, l.temp_cpu, 1);
        mul_cpu(l.outputs*l.batch, l.g_cpu, 1, l.temp_cpu, 1);
        mul_cpu(l.outputs*l.batch, l.f_cpu, 1, l.c_cpu, 1);
        axpy_cpu(l.outputs*l.batch, 1, l.temp_cpu, 1, l.c_cpu, 1);

        // o = wo + uo + vo(c_new)
        if (l.peephole) {
            s.input = l.c_cpu;
            forward_convolutional_layer(vo, s);
        }
        copy_cpu(l.outputs*l.batch, wo.output, 1, l.o_cpu, 1);
        axpy_cpu(l.outputs*l.batch, 1, uo.output, 1, l.o_cpu, 1);
        if (l.peephole) axpy_cpu(l.outputs*l.batch, 1, vo.output, 1, l.o_cpu, 1);
        activate_array(l.o_cpu, l.outputs*l.batch, LOGISTIC);

        // h = o * tanh(c)
        copy_cpu(l.outputs*l.batch, l.c_cpu, 1, l.h_cpu, 1);
        activate_array(l.h_cpu, l.outputs*l.batch, TANH);
        mul_cpu(l.outputs*l.batch, l.o_cpu, 1, l.h_cpu, 1);

        if (l.state_constrain) constrain_cpu(l.outputs*l.batch, l.state_constrain, l.c_cpu);
        fix_nan_and_inf_cpu(l.c_cpu, l.outputs*l.batch);
        fix_nan_and_inf_cpu(l.h_cpu, l.outputs*l.batch);

        copy_cpu(l.outputs*l.batch, l.c_cpu, 1, l.cell_cpu, 1);
        copy_cpu(l.outputs*l.batch, l.h_cpu, 1, l.output, 1);

        state.input += l.inputs*l.batch;
        l.output    += l.outputs*l.batch;
        l.cell_cpu      += l.outputs*l.batch;

        if (l.peephole) {
            increment_layer(&vf, 1);
            increment_layer(&vi, 1);
            increment_layer(&vo, 1);
        }

        increment_layer(&wf, 1);
        increment_layer(&wi, 1);
        increment_layer(&wg, 1);
        increment_layer(&wo, 1);

        increment_layer(&uf, 1);
        increment_layer(&ui, 1);
        increment_layer(&ug, 1);
        increment_layer(&uo, 1);
    }
}

void backward_conv_lstm_layer(layer l, network_state state)
{
    network_state s = { 0 };
    s.train = state.train;
    s.workspace = state.workspace;
    int i;
    layer vf = *(l.vf);
    layer vi = *(l.vi);
    layer vo = *(l.vo);

    layer wf = *(l.wf);
    layer wi = *(l.wi);
    layer wg = *(l.wg);
    layer wo = *(l.wo);

    layer uf = *(l.uf);
    layer ui = *(l.ui);
    layer ug = *(l.ug);
    layer uo = *(l.uo);

    if (l.peephole) {
        increment_layer(&vf, l.steps - 1);
        increment_layer(&vi, l.steps - 1);
        increment_layer(&vo, l.steps - 1);
    }

    increment_layer(&wf, l.steps - 1);
    increment_layer(&wi, l.steps - 1);
    increment_layer(&wg, l.steps - 1);
    increment_layer(&wo, l.steps - 1);

    increment_layer(&uf, l.steps - 1);
    increment_layer(&ui, l.steps - 1);
    increment_layer(&ug, l.steps - 1);
    increment_layer(&uo, l.steps - 1);

    state.input += l.inputs*l.batch*(l.steps - 1);
    if (state.delta) state.delta += l.inputs*l.batch*(l.steps - 1);

    l.output += l.outputs*l.batch*(l.steps - 1);
    l.cell_cpu += l.outputs*l.batch*(l.steps - 1);
    l.delta += l.outputs*l.batch*(l.steps - 1);

    for (i = l.steps - 1; i >= 0; --i) {
        if (i != 0) copy_cpu(l.outputs*l.batch, l.cell_cpu - l.outputs*l.batch, 1, l.prev_cell_cpu, 1);
        copy_cpu(l.outputs*l.batch, l.cell_cpu, 1, l.c_cpu, 1);
        if (i != 0) copy_cpu(l.outputs*l.batch, l.output - l.outputs*l.batch, 1, l.prev_state_cpu, 1);
        copy_cpu(l.outputs*l.batch, l.output, 1, l.h_cpu, 1);

        l.dh_cpu = (i == 0) ? 0 : l.delta - l.outputs*l.batch;

        // f = wf + uf + vf
        copy_cpu(l.outputs*l.batch, wf.output, 1, l.f_cpu, 1);
        axpy_cpu(l.outputs*l.batch, 1, uf.output, 1, l.f_cpu, 1);
        if (l.peephole) axpy_cpu(l.outputs*l.batch, 1, vf.output, 1, l.f_cpu, 1);

        // i = wi + ui + vi
        copy_cpu(l.outputs*l.batch, wi.output, 1, l.i_cpu, 1);
        axpy_cpu(l.outputs*l.batch, 1, ui.output, 1, l.i_cpu, 1);
        if (l.peephole) axpy_cpu(l.outputs*l.batch, 1, vi.output, 1, l.i_cpu, 1);

        // g = wg + ug
        copy_cpu(l.outputs*l.batch, wg.output, 1, l.g_cpu, 1);
        axpy_cpu(l.outputs*l.batch, 1, ug.output, 1, l.g_cpu, 1);

        // o = wo + uo + vo
        copy_cpu(l.outputs*l.batch, wo.output, 1, l.o_cpu, 1);
        axpy_cpu(l.outputs*l.batch, 1, uo.output, 1, l.o_cpu, 1);
        if (l.peephole) axpy_cpu(l.outputs*l.batch, 1, vo.output, 1, l.o_cpu, 1);

        activate_array(l.f_cpu, l.outputs*l.batch, LOGISTIC);
        activate_array(l.i_cpu, l.outputs*l.batch, LOGISTIC);
        activate_array(l.g_cpu, l.outputs*l.batch, TANH);
        activate_array(l.o_cpu, l.outputs*l.batch, LOGISTIC);

        copy_cpu(l.outputs*l.batch, l.delta, 1, l.temp3_cpu, 1);

        copy_cpu(l.outputs*l.batch, l.c_cpu, 1, l.temp_cpu, 1);
        activate_array(l.temp_cpu, l.outputs*l.batch, TANH);

        copy_cpu(l.outputs*l.batch, l.temp3_cpu, 1, l.temp2_cpu, 1);
        mul_cpu(l.outputs*l.batch, l.o_cpu, 1, l.temp2_cpu, 1);

        gradient_array(l.temp_cpu, l.outputs*l.batch, TANH, l.temp2_cpu);
        axpy_cpu(l.outputs*l.batch, 1, l.dc_cpu, 1, l.temp2_cpu, 1);
        // temp  = tanh(c)
        // temp2 = delta * o * grad_tanh(tahc(c))
        // temp3 = delta

        copy_cpu(l.outputs*l.batch, l.c_cpu, 1, l.temp_cpu, 1);
        activate_array(l.temp_cpu, l.outputs*l.batch, TANH);
        mul_cpu(l.outputs*l.batch, l.temp3_cpu, 1, l.temp_cpu, 1);
        gradient_array(l.o_cpu, l.outputs*l.batch, LOGISTIC, l.temp_cpu);
        // delta for o(w,u,v):       temp  = delta * tanh(c) * grad_logistic(o)
        // delta for c,f,i,g(w,u,v): temp2 = delta * o * grad_tanh(tahc(c)) + delta_c(???)
        // delta for output:         temp3 = delta

        // o
        // delta for O(w,u,v):     temp  = delta * tanh(c) * grad_logistic(o)
        if (l.peephole) {
            copy_cpu(l.outputs*l.batch, l.temp_cpu, 1, vo.delta, 1);
            s.input = l.cell_cpu;
            //s.delta = l.dc_cpu;
            backward_convolutional_layer(vo, s);
        }

        copy_cpu(l.outputs*l.batch, l.temp_cpu, 1, wo.delta, 1);
        s.input = l.prev_state_cpu;
        //s.delta = l.dh_cpu;
        backward_convolutional_layer(wo, s);

        copy_cpu(l.outputs*l.batch, l.temp_cpu, 1, uo.delta, 1);
        s.input = state.input;
        s.delta = state.delta;
        backward_convolutional_layer(uo, s);

        // g
        copy_cpu(l.outputs*l.batch, l.temp2_cpu, 1, l.temp_cpu, 1);
        mul_cpu(l.outputs*l.batch, l.i_cpu, 1, l.temp_cpu, 1);
        gradient_array(l.g_cpu, l.outputs*l.batch, TANH, l.temp_cpu);
        // delta for c,f,i,g(w,u,v): temp2 = (delta * o * grad_tanh(tahc(c)) + delta_c(???)) * g * grad_logistic(i)

        copy_cpu(l.outputs*l.batch, l.temp_cpu, 1, wg.delta, 1);
        s.input = l.prev_state_cpu;
        //s.delta = l.dh_cpu;
        backward_convolutional_layer(wg, s);

        copy_cpu(l.outputs*l.batch, l.temp_cpu, 1, ug.delta, 1);
        s.input = state.input;
        s.delta = state.delta;
        backward_convolutional_layer(ug, s);

        // i
        copy_cpu(l.outputs*l.batch, l.temp2_cpu, 1, l.temp_cpu, 1);
        mul_cpu(l.outputs*l.batch, l.g_cpu, 1, l.temp_cpu, 1);
        gradient_array(l.i_cpu, l.outputs*l.batch, LOGISTIC, l.temp_cpu);
        // delta for c,f,i,g(w,u,v): temp2 = (delta * o * grad_tanh(tahc(c)) + delta_c(???)) * g * grad_logistic(i)

        if (l.peephole) {
            copy_cpu(l.outputs*l.batch, l.temp_cpu, 1, vi.delta, 1);
            s.input = l.prev_cell_cpu;
            //s.delta = l.dc_cpu;
            backward_convolutional_layer(vi, s);
        }

        copy_cpu(l.outputs*l.batch, l.temp_cpu, 1, wi.delta, 1);
        s.input = l.prev_state_cpu;
        //s.delta = l.dh_cpu;
        backward_convolutional_layer(wi, s);

        copy_cpu(l.outputs*l.batch, l.temp_cpu, 1, ui.delta, 1);
        s.input = state.input;
        s.delta = state.delta;
        backward_convolutional_layer(ui, s);

        // f
        copy_cpu(l.outputs*l.batch, l.temp2_cpu, 1, l.temp_cpu, 1);
        mul_cpu(l.outputs*l.batch, l.prev_cell_cpu, 1, l.temp_cpu, 1);
        gradient_array(l.f_cpu, l.outputs*l.batch, LOGISTIC, l.temp_cpu);
        // delta for c,f,i,g(w,u,v): temp2 = (delta * o * grad_tanh(tahc(c)) + delta_c(???)) * c * grad_logistic(f)

        if (l.peephole) {
            copy_cpu(l.outputs*l.batch, l.temp_cpu, 1, vf.delta, 1);
            s.input = l.prev_cell_cpu;
            //s.delta = l.dc_cpu;
            backward_convolutional_layer(vf, s);
        }

        copy_cpu(l.outputs*l.batch, l.temp_cpu, 1, wf.delta, 1);
        s.input = l.prev_state_cpu;
        //s.delta = l.dh_cpu;
        backward_convolutional_layer(wf, s);

        copy_cpu(l.outputs*l.batch, l.temp_cpu, 1, uf.delta, 1);
        s.input = state.input;
        s.delta = state.delta;
        backward_convolutional_layer(uf, s);

        copy_cpu(l.outputs*l.batch, l.temp2_cpu, 1, l.temp_cpu, 1);
        mul_cpu(l.outputs*l.batch, l.f_cpu, 1, l.temp_cpu, 1);
        copy_cpu(l.outputs*l.batch, l.temp_cpu, 1, l.dc_cpu, 1);

        state.input -= l.inputs*l.batch;
        if (state.delta) state.delta -= l.inputs*l.batch;
        l.output -= l.outputs*l.batch;
        l.cell_cpu -= l.outputs*l.batch;
        l.delta -= l.outputs*l.batch;

        if (l.peephole) {
            increment_layer(&vf, -1);
            increment_layer(&vi, -1);
            increment_layer(&vo, -1);
        }

        increment_layer(&wf, -1);
        increment_layer(&wi, -1);
        increment_layer(&wg, -1);
        increment_layer(&wo, -1);

        increment_layer(&uf, -1);
        increment_layer(&ui, -1);
        increment_layer(&ug, -1);
        increment_layer(&uo, -1);
    }
}
