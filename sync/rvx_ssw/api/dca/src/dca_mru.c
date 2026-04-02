#include "ervp_mmiox1.h"
#include "ervp_printf.h"
#include "ervp_caching.h"
#include "ervp_delay.h"

#include "dca_matrix_info.h"
#include "dca_mru.h"

typedef struct
{
	dca_matrix_info_t mi;
	dca_matrix_info_t mo;
	uint32_t opcode;
} dca_mru_inst_t;

void dca_mru_hwinfo_elaborate(dca_mru_hwpara_t *hwpara, dca_mru_hwinfo_t *hwinfo)
{
	static int id_to_issue = 0;
	// hwinfo->num_col = hwpara->matrix_size_para % 10000;
	// hwinfo->num_row = (hwpara->matrix_size_para / 10000) % 10000;
	hwinfo->id = id_to_issue++;
}

ervp_hwtask_busy_fx_t dca_mru_copy_part(ervp_mop_mapping_t *mop_mapping, const dca_mru_hwinfo_t *const hwinfo, const ErvpMatrixInfo *mi_info, ErvpMatrixInfo *mo_info, int num_row, int num_col, unsigned int option_value)
{
	ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
	if (mop_option_has_postprocess(option_value))
	{
		matrix_copy_part_sw(mi_info, mo_info, num_row, num_col, option_value);
	}
	else
	{
		// printf_function();
		_matrix_check_copy_part(mi_info, mo_info, num_row, num_col, option_value);

		dca_mru_inst_t inst;
		inst.opcode = DCA_MRU_COPY;
		dca_matrix_info_generate(mi_info, &(inst.mi));
		dca_matrix_info_generate(mo_info, &(inst.mo));
		inst.mi.br.num_row_m1 = num_row - 1;
		inst.mi.br.num_col_m1 = num_col - 1;
		inst.mo.br.num_row_m1 = num_row - 1;
		inst.mo.br.num_col_m1 = num_col - 1;
		cache_flush_smart(2, mi_info->addr, mo_info->addr);

		// mmiox1_inst_wait_vacanct(hwinfo->mmiox_info);
		mmiox1_inst_push(hwinfo->mmiox_info, &inst, 1, 0);
		hwtask_busy_fx = dca_mru_busy_fx(hwinfo);
	}
	return hwtask_busy_fx;
}

ervp_hwtask_busy_fx_t dca_mru_transpose_part(ervp_mop_mapping_t *mop_mapping, const dca_mru_hwinfo_t *const hwinfo, const ErvpMatrixInfo *mi_info, ErvpMatrixInfo *mo_info, int num_row, int num_col, unsigned int option_value)
{
	ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
	if (mop_option_has_postprocess(option_value))
	{
		matrix_transpose_part_sw(mi_info, mo_info, num_row, num_col, option_value);
	}
	else
	{
		// printf_function();
		_matrix_check_transpose_part(mi_info, mo_info, num_row, num_col, option_value);

		dca_mru_inst_t inst;
		inst.opcode = DCA_MRU_TRANSPOSE;
		dca_matrix_info_generate(mi_info, &(inst.mi));
		dca_matrix_info_generate(mo_info, &(inst.mo));
		inst.mi.br.num_row_m1 = num_row - 1;
		inst.mi.br.num_col_m1 = num_col - 1;
		inst.mo.br.num_row_m1 = num_col - 1;
		inst.mo.br.num_col_m1 = num_row - 1;
		cache_flush_smart(2, mi_info->addr, mo_info->addr);

		// mmiox1_inst_wait_vacanct(hwinfo->mmiox_info);
		mmiox1_inst_push(hwinfo->mmiox_info, &inst, 1, 0);
		hwtask_busy_fx = dca_mru_busy_fx(hwinfo);
	}
	return hwtask_busy_fx;
}

ervp_hwtask_busy_fx_t dca_mru_fill_fixed(ervp_mop_mapping_t *mop_mapping, const dca_mru_hwinfo_t *const hwinfo, ErvpMatrixInfo *result, int32_t value)
{
	// printf_function();

	dca_mru_inst_t inst;
	inst.opcode = DCA_MRU_FILL;
	dca_matrix_info_init(&(inst.mi));
	dca_matrix_info_generate(result, &(inst.mo));

	cache_flush_smart(1, result->addr);
	// mmiox1_inst_wait_vacanct(hwinfo->mmiox_info);
	mmiox1_input_push(hwinfo->mmiox_info, &value, 1);
	mmiox1_inst_push(hwinfo->mmiox_info, &inst, 1, 0);
	return dca_mru_busy_fx(hwinfo);
}