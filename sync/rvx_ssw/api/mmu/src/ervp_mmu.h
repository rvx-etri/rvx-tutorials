#ifndef __ERVP_MMU_H__
#define __ERVP_MMU_H__

int quantize_to_page_size(int memory_size, int num_page);
int set_page_table(int memory_size, unsigned int base_addr, unsigned int target_addr);
void print_mmu(int page_id);

#endif
