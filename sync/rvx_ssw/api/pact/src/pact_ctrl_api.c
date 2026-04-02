#include "ervp_printf.h"
#include "ervp_variable_allocation.h"
#include "pact_register_info.h"
#include "pact_ctrl_api.h"

const ErvpMatrixInfo invalid_matrix_info DATA_BSS;

typedef union {
	unsigned int binary_value;
	unsigned int move_or_not : 1;
	struct {
		unsigned int not_used : 1;
		unsigned int right_operand : 10;
		unsigned int left_operand : 20;
	} move_inst;
	struct {
		unsigned int not_used : 1;
	} not_move_inst;
} PactInst;

void generate_pact_access_config(const ErvpMatrixInfo* info, PactAccessConfig* config)
{
	config->addr = (unsigned int)info->addr;
	config->stride = matrix_get_stride(info);
	config->num_row_m1 = info->num_row - 1;
	config->num_col_m1 = info->num_col - 1;
	switch(info->datatype)
	{
		case MATRIX_DATATYPE_FLOAT32:
			config->is_float = 1;
			break;
		default:
			config->is_float = 0;
	}
	switch(info->datatype)
	{
		case MATRIX_DATATYPE_SINT08:
		case MATRIX_DATATYPE_SINT16:
		case MATRIX_DATATYPE_SINT32:
			config->is_signed = 1;
			break;
		default:
			config->is_signed = 0;
	}

	switch(info->datatype)
	{
		case MATRIX_DATATYPE_SINT08:
		case MATRIX_DATATYPE_UINT08:
			config->data_width = PACT_DATA_WIDTH_DATA08;
			break;
		case MATRIX_DATATYPE_SINT16:
		case MATRIX_DATATYPE_UINT16:
			config->data_width = PACT_DATA_WIDTH_DATA16;
			break;
		case MATRIX_DATATYPE_SINT32:
		case MATRIX_DATATYPE_FLOAT32:
			config->data_width = PACT_DATA_WIDTH_DATA32;
			break;
	}
}

static inline int get_empty_fifo()
{
  const int INST_MEM_DEPTH = 1<<BW_PACT_INST_MEM_INDEX;
  int windex, rindex, diff;
  windex = mmio_read_data(MMAP_PACT_INST_MEM_WINDEX);
  rindex = mmio_read_data(MMAP_PACT_INST_MEM_RINDEX);
  diff = rindex - windex;
  if(diff<=0)
    diff += INST_MEM_DEPTH;
  else
    diff -= INST_MEM_DEPTH;
  return diff;
}

void pact_insert_inst_list(const PactInstBinary* inst_list, int num_inst, int force)
{
  volatile unsigned int* data = (unsigned int*)inst_list;
  int num_remaining = num_inst;
  int i = 0;
  while(num_remaining>0)
  {
    int num_empty;
    if(force)
      num_empty = num_remaining;
    else
    {
      num_empty = get_empty_fifo();
      if(num_empty > num_remaining)
        num_empty = num_remaining;
    }
    for(int j=0; j<(num_empty<<1); j++)
      mmio_write_data(MMAP_PACT_INST_MEM_WDATA, data[i+j]);
    i += (num_empty<<1);
    num_remaining -= num_empty;
  }
}

void pact_insert_lsu0_info(const ErvpMatrixInfo* info, int force)
{
	PactAccessConfig config;
	unsigned int* p = (unsigned int*)&config;
	
	generate_pact_access_config(info, &config);
	for(int i=0; i<PACT_ACCESS_CONFIG_SIZE_4BYTE; i++)
	{
		PactInstBinary inst;
		inst = pact_inst_lsu0_info(p[i]);
		pact_insert_inst(inst, force);
	}
}

void pact_insert_lsu1_info(const ErvpMatrixInfo* info, int force)
{
	PactAccessConfig config;
	unsigned int* p = (unsigned int*)&config;
	
	generate_pact_access_config(info, &config);
	for(int i=0; i<PACT_ACCESS_CONFIG_SIZE_4BYTE; i++)
	{
		PactInstBinary inst;
		inst = pact_inst_lsu1_info(p[i]);
		pact_insert_inst(inst, force);
	}
}

void pact_init_fifo()
{
  unsigned int cmd = PACT_CMD_OP_INIT_RINDEX;
  pact_wait_until_finished();
  mmio_write_data(MMAP_PACT_INST_MEM_WINDEX, 0);
	mmfifo_write(MMAP_PACT_CMD, &cmd, 1);
}

void pact_start_fifo()
{
  unsigned int cmd = PACT_CMD_OP_START_FIFO;
	mmfifo_write(MMAP_PACT_CMD, &cmd, 1);
}

void pact_start_mem(int index, int size)
{
  unsigned int cmd = PACT_CMD_OP_START_MEM | PACT_CMD_OP_SET_RINDEX | (size<<BW_PACT_CMD_OP) | (index<<16);
	mmfifo_write(MMAP_PACT_CMD, &cmd, 1);
}

void pact_enable_interrupt()
{
	mmio_write_data(MMAP_PACT_ITR_ENABLE, 1);
}

void pact_disable_interrupt()
{
	mmio_write_data(MMAP_PACT_ITR_ENABLE, 0);
}

void pact_clear_interrupt()
{
	mmio_write_data(MMAP_PACT_CMD, PACT_CMD_OP_CLEAR_ITR);
}

void pact_print_inst_list(const PactInstBinary* inst_list, int num_inst)
{
	for(int i=0; i<num_inst; i++)
	{
		PactInst inst;
		inst.binary_value = inst_list[i];
		printf("\n%x", inst.binary_value);
		if(inst.move_or_not)
		{
			printf(" move");
			unsigned int reg_value;
			unsigned int reg_index;
			reg_value = inst.move_inst.right_operand;
			reg_index = 0;
			while(reg_value!=0)
			{
				if((reg_value&1)==1)
				{
					printf(" OR_%02d", reg_index);
				}
				reg_index++;
				reg_value >>= 1;
			}
			reg_value = inst.move_inst.left_operand;
			reg_index = 0;
			while(reg_value!=0)
			{
				if((reg_value&1)==1)
				{
					printf(" IR_%02d", reg_index);
				}
				reg_index++;
				reg_value >>= 1;
			}
		}
		else
		{
		}
	}
}
