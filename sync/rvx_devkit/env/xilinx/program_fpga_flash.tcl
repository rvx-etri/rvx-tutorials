## ****************************************************************************
## ****************************************************************************
## Copyright SoC Design Research Group, All rights reserved.    
## Electronics and Telecommunications Research Institute (ETRI)
##
## THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE 
## WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS 
## TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE 
## REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL 
## SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE, 
## IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE 
## COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.
## ****************************************************************************
## 2019-10-01
## Kyuseung Han (han@etri.re.kr)
## ****************************************************************************
## ****************************************************************************

set define_list {}

#################
## environment ##
#################

source ./set_path.tcl
source ./set_fpga.tcl
source ./set_design.tcl
source ${RVX_ENV}/xilinx/__set_env.tcl

source ./gen_mcs.tcl

open_hw_manager
connect_hw_server
current_hw_server [get_hw_servers]
disconnect_hw_server
connect_hw_server
current_hw_server [get_hw_servers]

open_hw_target [get_hw_targets]
current_hw_device [get_hw_devices]
refresh_hw_device -update_hw_probes false

source ./set_config_memory.tcl

if {[string equal ${FLASH_INTERFACE_TYPE} spix8]} {
	set_property PROGRAM.FILES [list ${IMP_RESULT_DIR}/${BIT_FILE_NAME}_primary.mcs ${IMP_RESULT_DIR}/${BIT_FILE_NAME}_secondary.mcs] [ get_property PROGRAM.HW_CFGMEM [current_hw_device]]
} else {
	set_property PROGRAM.FILES [list ${IMP_RESULT_DIR}/${BIT_FILE_NAME}.mcs] [ get_property PROGRAM.HW_CFGMEM [current_hw_device]]
}
set_property PROGRAM.UNUSED_PIN_TERMINATION {pull-none} [ get_property PROGRAM.HW_CFGMEM [current_hw_device]]
set_property PROGRAM.BLANK_CHECK  0 [ get_property PROGRAM.HW_CFGMEM [current_hw_device]]
set_property PROGRAM.ERASE  1 [ get_property PROGRAM.HW_CFGMEM [current_hw_device]]
set_property PROGRAM.CFG_PROGRAM  1 [ get_property PROGRAM.HW_CFGMEM [current_hw_device]]
set_property PROGRAM.VERIFY  1 [ get_property PROGRAM.HW_CFGMEM [current_hw_device]]
set_property PROGRAM.CHECKSUM  0 [ get_property PROGRAM.HW_CFGMEM [current_hw_device]]

if {![string equal [get_property PROGRAM.HW_CFGMEM_TYPE [current_hw_device]] [get_property MEM_TYPE [get_property CFGMEM_PART [get_property PROGRAM.HW_CFGMEM [current_hw_device]]]]] } {
	create_hw_bitstream -hw_device [current_hw_device] [get_property PROGRAM.HW_CFGMEM_BITFILE [current_hw_device]];
	program_hw_devices [current_hw_device];
}

program_hw_cfgmem -hw_cfgmem [get_property PROGRAM.HW_CFGMEM [current_hw_device]]
