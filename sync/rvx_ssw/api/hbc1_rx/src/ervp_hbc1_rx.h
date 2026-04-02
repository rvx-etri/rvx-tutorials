#ifndef __H_ERVP_HBC1_RX_H__
#define __H_ERVP_HBC1_RX_H__

#include "ervp_hbc1_common.h"
#include "ervp_hbc1_rx_memorymap.h"

union AfeConfig
{
	unsigned int value;
	struct {
		unsigned int r_sel_cdrop33 : 3;
		unsigned int r_sel_cdrop12 : 3;
		unsigned int r_oe_test_op : 1;
		unsigned int r_pd_bias2 : 1;
		unsigned int r_pd_bias1 : 1;
		unsigned int r_pd_opa33 : 1;
		unsigned int r_pd_opa12 : 1;
		unsigned int r_cdr_in_mux : 1;
		unsigned int r_sel_amp_out : 1;
		unsigned int r_oe_test_amp : 1;
		unsigned int r_pd_comp : 1;
		unsigned int r_pd_amp2 : 1;
		unsigned int r_pd_amp1 : 1;
		unsigned int rs_pd_amp : 1;
		unsigned int rs_out_sel : 1;
		unsigned int rs_hys_sel : 2;
		unsigned int rs_fs_feed_sel : 1;
		unsigned int rs_hpf_sel : 2;
		unsigned int rs_cul_sel : 2;
	} br;
};

void hbc_rx_reset();
unsigned int hbc_rx_get_version();
void hbc_rx_print_version();
void hbc_rx_enable(int enable);
void hbc_rx_set_corr_threshold(unsigned int threshold);
void hbc_rx_set_my_addr(unsigned int my_addr);
void hbc_rx_store_head_crc(unsigned int value);
void hbc_rx_store_data_crc(unsigned int value);
void hbc_rx_store_not_my_addr(unsigned int value);
void hbc_rx_set_inverted_input(unsigned int value);
void hbc_rx_use_filtered_pn(unsigned int value);
void hbc_rx_enable_interrupt(unsigned int value);

unsigned int hbc_rx_get_frame_buffer_status();
unsigned int hbc_rx_get_frame_buffer_corr();
void hbc_rx_clear_frame_buffer();

void hbc_rx_read_frame_buffer(struct HbcFrame* frame);

void set_afe_config(const union AfeConfig afe_config);

void hbc_rx_isr_default();

#endif
