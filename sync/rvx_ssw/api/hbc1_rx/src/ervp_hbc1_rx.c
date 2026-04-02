#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_hbc1_rx.h"
#include "ervp_platform_controller_api.h"
#include "ervp_malloc.h"

union HbcRxConfig
{
	unsigned int value;
	struct {
		unsigned int corr_threshold : 16;
		unsigned int my_addr : 4;
		unsigned int store_head_crc : 1;
		unsigned int store_data_crc : 1;
		unsigned int store_not_my_addr : 1;
		unsigned int inverted_input : 1;
		unsigned int use_filtered_pn : 1;
		unsigned int enable_interrupt : 1;
	} br;
};

void hbc_rx_reset()
{
	REG32(MMAP_HRX_DEMODULATOR_SW_RESET) = 1;
}

unsigned int hbc_rx_get_version()
{
	return REG32(MMAP_HRX_DEMODULATOR_VERSION);
}

void hbc_rx_print_version()
{
	printf("\n%d", hbc_rx_get_version());
}

void hbc_rx_enable(int enable)
{
	REG32(MMAP_HRX_DEMODULATOR_ENABLE) = enable;
}

void hbc_rx_set_corr_threshold(unsigned int threshold)
{
	union HbcRxConfig config;
	config.value = REG32(MMAP_HRX_DEMODULATOR_CONFIG);
	config.br.corr_threshold = threshold;
	REG32(MMAP_HRX_DEMODULATOR_CONFIG) = config.value;
}

void hbc_rx_set_my_addr(unsigned int my_addr)
{
	union HbcRxConfig config;
	config.value = REG32(MMAP_HRX_DEMODULATOR_CONFIG);
	config.br.my_addr = my_addr;
	REG32(MMAP_HRX_DEMODULATOR_CONFIG) = config.value;
}

void hbc_rx_store_head_crc(unsigned int value)
{
	union HbcRxConfig config;
	config.value = REG32(MMAP_HRX_DEMODULATOR_CONFIG);
	config.br.store_head_crc = value;
	REG32(MMAP_HRX_DEMODULATOR_CONFIG) = config.value;
}

void hbc_rx_store_data_crc(unsigned int value)
{
	union HbcRxConfig config;
	config.value = REG32(MMAP_HRX_DEMODULATOR_CONFIG);
	config.br.store_data_crc = value;
	REG32(MMAP_HRX_DEMODULATOR_CONFIG) = config.value;
}

void hbc_rx_store_not_my_addr(unsigned int value)
{
	union HbcRxConfig config;
	config.value = REG32(MMAP_HRX_DEMODULATOR_CONFIG);
	config.br.store_not_my_addr = value;
	REG32(MMAP_HRX_DEMODULATOR_CONFIG) = config.value;
}

void hbc_rx_set_inverted_input(unsigned int value)
{
	union HbcRxConfig config;
	config.value = REG32(MMAP_HRX_DEMODULATOR_CONFIG);
	config.br.inverted_input = value;
	REG32(MMAP_HRX_DEMODULATOR_CONFIG) = config.value;
}

void hbc_rx_use_filtered_pn(unsigned int value)
{
	union HbcRxConfig config;
	config.value = REG32(MMAP_HRX_DEMODULATOR_CONFIG);
	config.br.use_filtered_pn = value;
	REG32(MMAP_HRX_DEMODULATOR_CONFIG) = config.value;
}

void hbc_rx_enable_interrupt(unsigned int value)
{
	union HbcRxConfig config;
	config.value = REG32(MMAP_HRX_DEMODULATOR_CONFIG);
	config.br.enable_interrupt = value;
	REG32(MMAP_HRX_DEMODULATOR_CONFIG) = config.value;
}

unsigned int hbc_rx_get_frame_buffer_status()
{
	return REG32(MMAP_HRX_FRAME_STATUS);
}

unsigned int hbc_rx_get_frame_buffer_corr()
{
	return REG32(MMAP_HRX_FRAME_CORR);
}

void hbc_rx_clear_frame_buffer()
{
	REG32(MMAP_HRX_FRAME_CLEAR) = 1;
}

void hbc_rx_read_frame_buffer(struct HbcFrame* frame)
{
	int i;
	int num_data_in_4bytes;
	unsigned int* addr;
	union HbcHeader header;
	header.value = REG32(MMAP_HRX_FRAME_HEADER);
	frame->rate = header.br.rate;
	frame->mode_type = header.br.mode_type;
	frame->frame_type = header.br.frame_type;
	frame->data_type = header.br.data_type;
	frame->src_addr = header.br.src_addr;
	frame->dst_addr = header.br.dst_addr;
	frame->num_byte = header.br.length;
	
	num_data_in_4bytes = ((frame->num_byte-1) >> 2) + 1;
	for(i=0; i < num_data_in_4bytes; i++)
	{
		addr = (unsigned int*)(&(frame->data[i<<2]));
		*addr = REG32(MMAP_HRX_FRAME_DATA_OUTPUT);
	}
	hbc_rx_clear_frame_buffer();
}

void set_afe_config(const union AfeConfig afe_config)
{
	REG32(MMAP_HRX_AFE_CONFIG) = afe_config.value;
}

void hbc_rx_isr_default()
{
	struct HbcFrame receive_frame;
	int status = hbc_rx_get_frame_buffer_status();
	printf("\n%x", status);
	if(status&HRX_FRAME_STATUS_BIT_VALID)
	{
		printf("\n%x", hbc_rx_get_frame_buffer_corr());
		hbc_rx_read_frame_buffer(&receive_frame);
		hbc_print_frame_struct(&receive_frame);
	}
}
