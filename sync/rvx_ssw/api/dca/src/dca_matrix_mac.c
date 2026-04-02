#include "ervp_mmiox1.h"
#include "ervp_printf.h"
#include "ervp_caching.h"
#include "ervp_delay.h"

#include "dca_matrix_info.h"
#include "dca_matrix_mac.h"

typedef struct
{
	dca_matrix_info_t ma;
	dca_matrix_info_t mb;
	dca_matrix_info_t mc;
	uint32_t opcode;
} dca_matrix_mac_inst_t;

static UNKNOWN_TYPE scalar_matrix_value;
static ErvpMatrixInfo scalar_matrix_info;

static void __attribute__((constructor)) construct_dca_matrix_mac()
{
	matrix_generate_info(MATRIX_DATATYPE_SINT32, 1, 1, &scalar_matrix_value, &scalar_matrix_info);
}

void dca_matrix_mac_hwinfo_elaborate(dca_matrix_mac_hwpara_t *hwpara, dca_matrix_mac_hwinfo_t *hwinfo)
{
	static int id_to_issue = 0;
	hwinfo->num_col = hwpara->matrix_size_para % 10000;
	hwinfo->num_row = (hwpara->matrix_size_para / 10000) % 10000;
	if (hwinfo->num_row == 0)
		hwinfo->num_row = hwinfo->num_col;
	hwinfo->id = id_to_issue++;
	hwinfo->mmiox_info = NULL;
}

static void _dca_matrix_mac_request(const dca_matrix_mac_hwinfo_t *const hwinfo, int opcode, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info)
{
	dca_matrix_mac_inst_t inst;

	inst.opcode = opcode;
	dca_matrix_info_generate(ma_info, &(inst.ma));
	dca_matrix_info_generate(mb_info, &(inst.mb));
	dca_matrix_info_generate(mc_info, &(inst.mc));

	mmiox1_inst_push(hwinfo->mmiox_info, &inst, 1, 0);
}

ervp_hwtask_busy_fx_t dca_matrix_mac_start(ervp_mop_mapping_t *mop_mapping, const dca_matrix_mac_hwinfo_t *const hwinfo, int opcode, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int option_value)
{
	ervp_mop_option_t mop_option;
	mop_option.value = option_value;

	ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
	if (mop_option_has_postprocess(option_value))
	{
		const int stride = mop_option.br.stride_m1 + 1;
		ErvpMatrixInfo *temp = matrix_alloc(MATRIX_DATATYPE_SINT32, mc_info->num_row * stride, mc_info->num_col * stride, NULL);
		cache_flush_smart(3, ma_info->addr, mb_info->addr, temp->addr);
		_dca_matrix_mac_request(hwinfo, opcode, ma_info, mb_info, temp);
		dca_matrix_mac_wait(hwinfo);
		hwtask_busy_fx = matrix_perform_postprocess_tf(mop_mapping, temp, mc_info, option_value);
		if (hwtask_busy_fx)
		{
			hwtask_wait_complete(hwtask_busy_fx);
			hwtask_busy_fx = NULL;
			matrix_free(temp);
		}
		else
			matrix_free(temp);
	}
	else
	{
		int opcode_with_acc = opcode;
		if (mop_option.br.acc)
		{
			opcode_with_acc &= (~DCA_MATRIX_MAC_OPCODE_INIT_ACC);
			opcode_with_acc |= DCA_MATRIX_MAC_OPCODE_LOAD_ACC;
		}
		cache_flush_smart(3, ma_info->addr, mb_info->addr, mc_info->addr);
		_dca_matrix_mac_request(hwinfo, opcode_with_acc, ma_info, mb_info, mc_info);
		hwtask_busy_fx = dca_matrix_mac_busy_fx(hwinfo);
	}
	return hwtask_busy_fx;
}

ervp_hwtask_busy_fx_t dca_matrix_scalar_mult_fixed(ervp_mop_mapping_t *mop_mapping, const dca_matrix_mac_hwinfo_t *const hwinfo, const ErvpMatrixInfo *ma_info, int scalar_value, ErvpMatrixInfo *mc_info, unsigned int option_value)
{
	scalar_matrix_value.value_signed = scalar_value;
	return dca_matrix_mac_start(mop_mapping, hwinfo, DCA_MAC_SCALAR_MULT, ma_info, &scalar_matrix_info, mc_info, option_value);
}