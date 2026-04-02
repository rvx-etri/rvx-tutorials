#ifndef __PACT_CTRL_API_H__
#define __PACT_CTRL_API_H__

#include "ervp_matrix.h"
#include "ervp_mmio_util.h"
#include "pact_memorymap.h"
#include "pact_inst.h"

typedef struct {
	unsigned int addr;
	unsigned int stride : 16;
	unsigned int num_row_m1 : 8;
	unsigned int num_col_m1 : 8;
	unsigned int data_width : 2;
	unsigned int is_float : 1;
	unsigned int is_signed : 1;
} PactAccessConfig;

#define PACT_ACCESS_CONFIG_SIZE_4BYTE (((sizeof(PactAccessConfig)-1)>>2) + 1)

extern const ErvpMatrixInfo invalid_matrix_info;

void generate_pact_access_config(const ErvpMatrixInfo* info, PactAccessConfig* config);

void pact_insert_inst_list(const PactInstBinary* inst_list, int num_inst, int force);
static inline void pact_insert_inst(const PactInstBinary inst, int force){pact_insert_inst_list(&inst, 1, force);}
void pact_insert_lsu0_info(const ErvpMatrixInfo* info, int force);
void pact_insert_lsu1_info(const ErvpMatrixInfo* info, int force);

void pact_print_inst_list(const PactInstBinary* inst_list, int num_inst);

void pact_init_fifo();
void pact_start_fifo();
void pact_start_mem(int index, int size);
static inline void pact_start(){pact_start_fifo();};

static inline unsigned int pact_read_status()
{
	return mmio_read_data(MMAP_PACT_STATUS);
}
static inline void pact_wait_until_finished()
{
	while(pact_read_status()!=PACT_STATUS_IDLE);
}

void pact_enable_interrupt();
void pact_disable_interrupt();
void pact_clear_interrupt();

#endif
