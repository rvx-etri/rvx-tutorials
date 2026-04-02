#ifndef __H_ERVP_HBC1_TX_H__
#define __H_ERVP_HBC1_TX_H__

#include "ervp_hbc1_common.h"
#include "ervp_hbc1_tx_memorymap.h"

void hbc_tx_reset();
unsigned int hbc_tx_get_version();
void hbc_tx_print_version();
void hbc_tx_transfer_frame_buffer();
void hbc_tx_set_inverted_output(unsigned int value);
unsigned int hbc_tx_get_number_of_vacant_buffers();
unsigned int hbc_tx_get_status();

void hbc_tx_clear_frame();
void hbc_tx_write_frame_buffer(const struct HbcFrame* frame);

void init_hbc_frame_struct(struct HbcFrame* frame);
void set_hbc_frame_struct_data(struct HbcFrame* frame, unsigned char data);

void hbc_tx_isr_test();

#endif
