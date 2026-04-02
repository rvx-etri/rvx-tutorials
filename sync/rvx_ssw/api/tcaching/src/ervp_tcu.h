#ifndef __ERVP_TCU_H__
#define __ERVP_TCU_H__

void tcu_set_region_valid(int index, unsigned int value);
unsigned int tcu_get_region_valid_list();
void tcu_clear_all_region();
void tcu_set_region_start(int index, unsigned int value);
unsigned int tcu_get_region_start(int index);
void tcu_set_region_last(int index, unsigned int value);
unsigned int tcu_get_region_last(int index);
void tcu_set_region_offset(int index, unsigned int value);
unsigned int tcu_get_region_offset(int index);

#endif
