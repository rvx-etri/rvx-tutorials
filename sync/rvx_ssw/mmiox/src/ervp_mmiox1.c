#include "ervp_mmiox1.h"
#include "ervp_round_int.h"

void mmiox1_hwinfo_elaborate(ervp_mmiox1_hwpara_t *hwpara, ervp_mmiox1_hwinfo_t *hwinfo)
{
  hwinfo->size_of_config = rshift_ru(hwpara->bw_config, 3);
  hwinfo->size_of_inst = rshift_ru(hwpara->bw_inst, 3);
  hwinfo->size_of_log = rshift_ru(hwpara->bw_log, 3);
  hwinfo->size_of_status = rshift_ru(hwpara->bw_status, 3);
  hwinfo->size_of_input = rshift_ru(hwpara->bw_input, 3);
  hwinfo->size_of_output = rshift_ru(hwpara->bw_output, 3);
  hwinfo->baseaddr = 0;
  hwinfo->busy_fx = NULL;
}

void mmiox1_print_info(const ervp_mmiox1_hwinfo_t* ip_info)
{
  printf("\n%x", ip_info->baseaddr);
}