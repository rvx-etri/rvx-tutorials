#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_bit_util.h"

#include "dca_matrix_info.h"
#include "starc.h"
#include "starc_support.h"

typedef struct
{
	dca_matrix_info_t spikein;
	dca_matrix_info_t weight;
	dca_matrix_info_t spikeout;
	unsigned int num_step_merged_m1 : 16;
	unsigned int num_spike_allowed : 15;
	unsigned int neuron_reset_type : 1;
	unsigned int opcode : 8;
} starc_inst_t;

void starc_hwinfo_elaborate(starc_hwpara_t *hwpara, starc_hwinfo_t *hwinfo)
{
	static int id_to_issue = 0;
	hwinfo->core_input_size = hwpara->core_input_size;
	hwinfo->core_output_size = hwpara->core_output_size;
	hwinfo->bw_weight = hwpara->bw_weight;
	hwinfo->capacitance = hwpara->capacitance;
	hwinfo->bw_cap_counter = hwpara->bw_cap_counter;
	hwinfo->id = id_to_issue++;
}

void _starc_request(const starc_hwinfo_t *hwinfo, const ervp_starc_option_t *options, const starc_supply_t *supply, const ErvpMatrixInfo *spikein_info, ErvpMatrixInfo *spikeout_info)
{
	assert(options != NULL);
	assert(spikein_info->is_binary == 1);
	spikeout_info->is_binary = 1;

	starc_inst_t inst;
	dca_matrix_info_generate(spikein_info, &(inst.spikein));
	dca_matrix_info_generate(supply->converted_weight_info, &(inst.weight));
	dca_matrix_info_generate(spikeout_info, &(inst.spikeout));
	inst.num_step_merged_m1 = options->num_step_merged_m1;
	inst.num_spike_allowed = supply->num_spike_allowed;
	inst.neuron_reset_type = options->neuron_reset_type;

	mmiox1_input_push(hwinfo->mmiox_info, supply->formatted_threshold, 1);
	mmiox1_inst_push(hwinfo->mmiox_info, &inst, 1, 0);
}

ervp_hwtask_busy_fx_t starc_start(ervp_mop_mapping_t *mop_mapping, const starc_hwinfo_t *hwinfo, const ervp_starc_option_t *options, const starc_supply_t *supply, const ErvpMatrixInfo *spikein_info, ErvpMatrixInfo *spikeout_info, unsigned int option_value)
{
	assert(spikein_info->num_col == supply->converted_weight_info->num_col);
	assert(spikein_info->num_row == spikeout_info->num_row);
	assert(spikeout_info->num_col == supply->converted_weight_info->num_row);

	ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
	cache_flush_smart(3, spikein_info->addr, supply->converted_weight_info->addr, spikeout_info->addr);
	_starc_request(hwinfo, options, supply, spikein_info, spikeout_info);
	hwtask_busy_fx = starc_busy_fx(hwinfo);
	return hwtask_busy_fx;
}

#ifdef USE_NPX

#include "npx_profiling.h"

void starc_forward_layer_block(const starc_hwinfo_t *hwinfo, npx_layer_block_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state)
{
	printf_function();
	NPX_PROFILING_START();

	assert(hwinfo != NULL);

	assert_pointer(2, layer, state);
	assert(state->input_tsseq != NULL);
	assert(state->input_tsseq->is_boundary);
	assert(state->output_tsseq == NULL);

	assert(layer->num_layer == 2);
	assert(layer->layer_compute_seq != NULL);
	assert(layer->layer_compute_seq[0]->layer_type == NPXL_LINEAR);
	assert(layer->layer_compute_seq[1]->layer_type == NPXL_LEAKY);

	npx_linear_layer_t *linear_layer = layer->layer_compute_seq[0]->layer;
	npx_leaky_layer_t *leaky_layer = layer->layer_compute_seq[1]->layer;

	assert(linear_layer->iodata.in_channels == linear_layer->iodata.out_channels);

	const int timesteps = state->input_tsseq->timesteps;
	int size[2];
	size[0] = 1;
	size[1] = linear_layer->out_features;
	state->output_tsseq = npx_output_tsseq_alloc(timesteps, state->input_tsseq->is_boundary, 1, MATRIX_DATATYPE_SINT08, 2, size);

	assert(leaky_layer->membrane_potential_scaled == 1);

	ervp_starc_option_t starc_options;

	starc_options.num_step_merged_m1 = 0;
	starc_options.neuron_reset_type = leaky_layer->is_hard_reset ? NEURON_RESET_TYPE_HARD : NEURON_RESET_TYPE_SOFT;
	starc_options.opcode = 0;
	starc_options.threshold = leaky_layer->threshold;

	ErvpMatrixInfo *weight_info = npx_tensor_to_matrix_info(linear_layer->weight_tensor, NULL);
	starc_supply_t *starc_supply = starc_supply_generate(hwinfo, weight_info, &starc_options);

	const NpxTensorInfo *const input_tensor = state->input_tsseq->sequence[0];
	const NpxTensorInfo *const output_tensor = state->output_tsseq->sequence[0];

	assert(input_tensor->size[0] == 1);
	assert(output_tensor->size[0] == 1);
	assert(linear_layer->iodata.out_channels == 1);

	ErvpMatrixInfo spikein_info, spikeout_info;
	matrix_generate_info(input_tensor->datatype, timesteps, input_tensor->size[1], input_tensor->addr, &spikein_info);
	spikein_info.is_binary = 1;
	matrix_generate_info(output_tensor->datatype, timesteps, output_tensor->size[1], output_tensor->addr, &spikeout_info);
	spikeout_info.is_binary = 1;

	ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
	hwtask_busy_fx = starc_start(mop_mapping, hwinfo, &starc_options, starc_supply, &spikein_info, &spikeout_info, 0);
	
	matrix_free(weight_info);
	starc_supply_free(starc_supply);
	hwtask_wait_complete(hwtask_busy_fx);
	NPX_PROFILING_END();
}

void starc_forward_layer_block_allcases(const starc_hwinfo_t *hwinfo, npx_layer_block_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state)
{
	assert(hwinfo != NULL);
	assert(layer->num_layer == 2);
	assert(layer->layer_compute_seq != NULL);
	assert(layer->layer_compute_seq[0]->layer_type == NPXL_LINEAR);
	assert(layer->layer_compute_seq[1]->layer_type == NPXL_LEAKY);

	if (state->input_tsseq->sequence[0]->is_binary == 1)
		starc_forward_layer_block(hwinfo, layer, mop_mapping, state);
	else
		npx_forward_layer_block_default(layer, mop_mapping, state);
}

#endif