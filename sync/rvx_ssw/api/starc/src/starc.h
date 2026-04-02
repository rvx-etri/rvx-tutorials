#ifndef __STARC_H__
#define __STARC_H__

#include "ervp_matrix_op.h"
#include "ervp_mmiox1.h"
#include "starc_memorymap_offset.h"

typedef struct
{
	unsigned int bw_addr;
	unsigned int spikein_bw_data;
	unsigned int spikein_bw_tensor;
	unsigned int weight_bw_data;
	unsigned int weight_bw_tensor;
	unsigned int spikeout_bw_data;
	unsigned int spikeout_bw_tensor;
	unsigned int core_input_size;
	unsigned int core_output_size;
	unsigned int bw_weight;
	unsigned int bw_config;
	unsigned int bw_status;
	unsigned int bw_log;
	unsigned int bw_inst;
	unsigned int bw_input;
	unsigned int bw_output;
	unsigned int config_default_value; // invalid due to the limit of data type, but not used
	unsigned int log_fifo_depth;
	unsigned int inst_fifo_depth;
	unsigned int input_fifo_depth;
	unsigned int output_fifo_depth;
	unsigned int lsu_para;
	unsigned int capacitance;
	unsigned int bw_cap_counter;
} starc_hwpara_t;

typedef struct
{
	const ervp_mmiox1_hwinfo_t *mmiox_info;
	unsigned int core_input_size : 16;
	unsigned int core_output_size : 16;
	unsigned int bw_weight : 16;
	unsigned int capacitance : 16;
	unsigned int bw_cap_counter : 16;
	int id : 16;
} starc_hwinfo_t;

void starc_hwinfo_elaborate(starc_hwpara_t *hwpara, starc_hwinfo_t *hwinfo);
static inline void starc_print_info(const starc_hwinfo_t *hwinfo)
{
	mmiox1_print_info(hwinfo->mmiox_info);
}

static inline ervp_hwtask_busy_fx_t starc_busy_fx(const starc_hwinfo_t *const hwinfo)
{
	return hwinfo->mmiox_info->busy_fx;
}

static inline void starc_wait(const starc_hwinfo_t *const hwinfo)
{
	hwtask_wait_complete(starc_busy_fx(hwinfo));
}

typedef struct
{
	unsigned int num_step_merged_m1 : 16;
	unsigned int neuron_reset_type : 8;
	unsigned int opcode : 8;
	int threshold; // DO NOT change the type to unsigned
} ervp_starc_option_t;

typedef struct
{
	const ErvpMatrixInfo *converted_weight_info;
	unsigned int num_spike_allowed;
	uint32_t *formatted_threshold;
} starc_supply_t;

void _starc_request(const starc_hwinfo_t *hwinfo, const ervp_starc_option_t *options, const starc_supply_t *supply, const ErvpMatrixInfo *spikein_info, ErvpMatrixInfo *spikeout_info);
ervp_hwtask_busy_fx_t starc_start(ervp_mop_mapping_t *mop_mapping, const starc_hwinfo_t *hwinfo, const ervp_starc_option_t *options, const starc_supply_t *supply, const ErvpMatrixInfo *spikein_info, ErvpMatrixInfo *spikeout_info, unsigned int option_value);

#ifdef USE_NPX

#include "npx_struct.h"

void starc_forward_layer_block(const starc_hwinfo_t *hwinfo, npx_layer_block_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state);
void starc_forward_layer_block_allcases(const starc_hwinfo_t *hwinfo, npx_layer_block_t *layer, ervp_mop_mapping_t *mop_mapping, npx_layerio_state_t *state);

#endif

#endif