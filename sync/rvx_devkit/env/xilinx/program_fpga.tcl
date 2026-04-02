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
## 2019-04-15
## Kyuseung Han (han@etri.re.kr)
## ****************************************************************************
## ****************************************************************************

set define_list {}

#################
## environment ##
#################

source ./set_path.tcl
source ./set_design.tcl
source ${RVX_ENV}/xilinx/__set_env.tcl

#############
## program ##
#############

if [string equal $IMP_MODE "debug"] {
	start_gui
}

open_hw_manager
connect_hw_server
current_hw_server [get_hw_servers]
disconnect_hw_server
connect_hw_server
current_hw_server [get_hw_servers]

open_hw_target [get_hw_targets]
current_hw_device [get_hw_devices]
refresh_hw_device -update_hw_probes false

if [string equal $IMP_MODE "debug"] {
	set_property PROGRAM.FILE ${IMP_RESULT_DIR}/${BIT_FILE_NAME}.bit [current_hw_device]
	set_property PROBES.FILE ${IMP_RESULT_DIR}/${BIT_FILE_NAME}.ltx [current_hw_device]
	program_hw_devices
	refresh_hw_device
	display_hw_ila_data
} else {
	set_property PROGRAM.FILE ${IMP_RESULT_DIR}/${BIT_FILE_NAME}.bit [current_hw_device]
	program_hw_devices
	close_hw_target
	disconnect_hw_server
}
