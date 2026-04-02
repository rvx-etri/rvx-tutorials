#include "ervp_hbc1_common.h"
#include "ervp_printf.h"

void hbc_print_frame_struct(const struct HbcFrame* frame)
{
	int i;
	int max;
	switch(frame->data_type)
	{
		case HBC_HEADER_TYPE_CONTROL:
			printf("\nmode: CONTROL");
			break;
		case HBC_HEADER_TYPE_DATA:
			printf("\nmode: DATE");
			break;
		case HBC_HEADER_TYPE_MULTICAST:
			printf("\nmode: MULTICAST");
			break;
		case HBC_HEADER_TYPE_BROADCAST:
			printf("\nmode: BROADCAST");
			break;
	}
	printf("\nnum_byte: %d", frame->num_byte);
	printf("\nsrc_addr: %x", frame->src_addr);
	printf("\ndst_addr: %x", frame->dst_addr);
	for(i=0; i<frame->num_byte; i++)
	{
		if((i&7)==0)
		{
			max = i+7;
			if(max > (frame->num_byte-1))
				max = frame->num_byte-1;
			printf("\ndata %2d ~ %2d:", i, max);
		}
		printf(" %2x", frame->data[i]);
	}
	printf("\n");
}

int hbc_get_frame_max_length(unsigned int rate)
{
	int length;
	switch(rate)
	{
		case HBC_HEADER_111KBPS:
			length = 10;
			break;
		case HBC_HEADER_200KBPS:
			length = 19;
			break;
		case HBC_HEADER_333KBPS:
			length = 32;
			break;
		case HBC_HEADER_1MPS:
			length = 99;
			break;
		case HBC_HEADER_89_9KBPS:
			length = 5;
			break;
		case HBC_HEADER_161_9KBPS:
			length = 11;
			break;
		case HBC_HEADER_269_8KBPS:
			length = 24;
			break;
		case HBC_HEADER_809_5KBPS:
			length = 75;
			break;
		default:
			length = 0;
			break;
	}
	return length;
}

int hbc_check_frame_validity(const struct HbcFrame* frame)
{
	int valid = 1;
	int length;
	length = hbc_get_frame_max_length(frame->rate);
	if(frame->num_byte > length)
		valid = 0;
	return valid;
}
