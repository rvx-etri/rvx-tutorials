#ifndef __H_ERVP_HBC1_COMMON_H__
#define __H_ERVP_HBC1_COMMON_H__

#include "ervp_hbc1_common_memorymap_offset.h"

struct HbcFrame
{
	unsigned char data[100];
	int num_byte; // in bytes
	unsigned int rate;
	unsigned int mode_type;
	unsigned int frame_type;
	unsigned int data_type;
	unsigned int src_addr;
	unsigned int dst_addr;
};

union HbcHeader
{
	unsigned int value;
	struct {
		unsigned int length : 8;
		unsigned int dst_addr : 4;
		unsigned int src_addr : 4;
		unsigned int data_type : 2;
		unsigned int reserved : 1;
		unsigned int frame_type : 1;
		unsigned int mode_type : 1;
		unsigned int rate : 3;
	} br;
};

void hbc_print_frame_struct(const struct HbcFrame* frame);
int hbc_get_frame_max_length(unsigned int rate);
int hbc_check_frame_validity(const struct HbcFrame* frame);

#endif
