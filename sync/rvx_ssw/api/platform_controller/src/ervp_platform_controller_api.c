// ****************************************************************************
// ****************************************************************************
// Copyright SoC Design Research Group, All rights reserved.
// Electronics and Telecommunications Research Institute (ETRI)
// 
// THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE
// WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS
// TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE
// REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL
// SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE,
// IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE
// COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.
// ****************************************************************************
// 2017-07
// Kyuseung Han (han@etri.re.kr)
// ****************************************************************************
// ****************************************************************************

#include "platform_info.h"
#include "ervp_platform_controller_memorymap_offset.h"
#include "ervp_platform_controller_api.h"

int get_test_period_ms()
{
	int result;
	result = is_sim()? 1 : 1000;
	return result;
}
