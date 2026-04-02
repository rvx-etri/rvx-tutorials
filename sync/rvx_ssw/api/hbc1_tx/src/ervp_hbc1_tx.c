#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_hbc1_tx.h"
#include "ervp_platform_controller_api.h"
#include "ervp_malloc.h"

void hbc_tx_reset()
{
	REG32(MMAP_HTX_MODULATOR_SW_RESET) = 1;
}

unsigned int hbc_tx_get_version()
{
	return REG32(MMAP_HTX_MODULATOR_VERSION);
}

void hbc_tx_print_version()
{
	printf("\n%d", hbc_tx_get_version());
}

void hbc_tx_transfer_frame_buffer()
{
	REG32(MMAP_HTX_MODULATOR_TRANSFER_FRAME) = 1;
}

void hbc_tx_set_inverted_output(unsigned int value)
{
	REG32(MMAP_HTX_MODULATOR_CONFIG) = value;
}

unsigned int hbc_tx_get_number_of_vacant_buffers()
{
	return REG32(MMAP_HTX_MODULATOR_NUM_VACANT_BUFFERS);
}

unsigned int hbc_tx_get_status()
{
	return REG32(MMAP_HTX_MODULATOR_STATUS);
}

void hbc_tx_clear_frame()
{
	REG32(MMAP_HTX_FRAME_CLEAR) = 1;
}

void hbc_tx_write_frame_buffer(const struct HbcFrame* frame)
{
	int i;
	int num_data_in_4bytes;
	unsigned int* addr;
	union HbcHeader header;

	num_data_in_4bytes = ((frame->num_byte-1) >> 2) + 1;
	while(hbc_tx_get_number_of_vacant_buffers()<num_data_in_4bytes);
	for(i=0; i < num_data_in_4bytes; i++)
	{
		addr = (unsigned int*)(&(frame->data[i<<2]));
		REG32(MMAP_HTX_FRAME_DATA_INPUT) = *addr;
	}

	header.value = 0;	
	header.br.rate = frame->rate;
	header.br.mode_type = frame->mode_type;
	header.br.frame_type = frame->frame_type;
	header.br.data_type = frame->data_type;
	header.br.src_addr = frame->src_addr;
	header.br.dst_addr = frame->dst_addr;
	header.br.length = frame->num_byte;
	while(hbc_tx_get_status()!=HTX_MODULATOR_STATUS_IDLE);
	REG32(MMAP_HTX_FRAME_HEADER) = header.value;
}

void init_hbc_frame_struct(struct HbcFrame* frame)
{
	frame->num_byte = 0;
	frame->rate = 0;
	frame->mode_type = 0;
	frame->frame_type = 0;
	frame->data_type = 0;
	frame->src_addr = 0;
	frame->dst_addr = 0;
}

void set_hbc_frame_struct_data(struct HbcFrame* frame, unsigned char data)
{
	frame->data[frame->num_byte++] = data;
}

static struct HbcFrame test_frame;

static inline void hbc_tx_init_test_frame()
{
	init_hbc_frame_struct(&test_frame);
	test_frame.rate = HBC_HEADER_1MPS;
	test_frame.mode_type = HBC_HEADER_MODE_NORMAL;
	test_frame.frame_type = HBC_HEADER_FRAME_NORMAL;
	test_frame.data_type = HBC_HEADER_TYPE_BROADCAST;
	test_frame.src_addr = 0xf;
	test_frame.dst_addr = 0x5;
}

void hbc_tx_isr_test()
{
	const int MAX_NUM_DATA = 16;
	static int num = 0;
	if(num==(MAX_NUM_DATA))
		num = 0;
	if(num==0)
		hbc_tx_init_test_frame();
	num++;
	set_hbc_frame_struct_data(&test_frame, MAX_NUM_DATA-num);
	hbc_tx_write_frame_buffer(&test_frame);
	hbc_tx_transfer_frame_buffer();
}
