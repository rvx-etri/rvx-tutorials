#ifndef __STARC_SUPPORT_H__
#define __STARC_SUPPORT_H__

#include "ervp_matrix.h"
#include "ervp_math.h"
#include "starc.h"

const ErvpMatrixInfo *starc_convert_weight(const starc_hwinfo_t *hwinfo, const ErvpMatrixInfo *weight_info);
unsigned int starc_analyze_num_spike_allowed(const starc_hwinfo_t *hwinfo, const ErvpMatrixInfo *weight_info, const ervp_starc_option_t *options);
uint32_t *starc_convert_threshold(const starc_hwinfo_t *hwinfo, const ervp_starc_option_t *options);

starc_supply_t *starc_supply_generate(const starc_hwinfo_t *hwinfo, const ErvpMatrixInfo *weight_info, const ervp_starc_option_t *options);
void starc_supply_free(starc_supply_t *supply);

#endif