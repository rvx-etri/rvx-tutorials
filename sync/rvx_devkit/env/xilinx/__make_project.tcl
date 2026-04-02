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
## 2019-04-18
## Kyuseung Han (han@etri.re.kr)
## ****************************************************************************
## ****************************************************************************

#####################
## create project  ##
#####################

if {[file exist ${IMP_RESULT_DIR}]} {
	file delete -force ${IMP_RESULT_DIR}
}
file mkdir ${IMP_RESULT_DIR}

create_project ${IMP_RESULT_DIR}/${PROJECT_NAME}

###############
## FPGA type ##
###############

source ./set_fpga.tcl
set_property part ${FPGA_PART} [current_project]

################
## add source ##
################

lappend define_list FPGA_IMP

source ${SYN_ENV_FILE}
source ${SYN_COMMON_LIB_ENV_FILE}

#################
## ILA include ##
#################

if [string equal $IMP_MODE "debug"] {
	puts "\[INFO\] ILA is used"
	lappend define_list USE_ILA
	lappend verilog_include_list ${ILA_RESULT_DIR}
} else {
	puts "\[INFO\] ILA is NOT used"
}

#################
## read source ##
#################

#puts $verilog_module_list
#puts $vhdl_module_list
#puts $edf_list
#puts $xci_list

foreach verilog_module $verilog_module_list {
	read_verilog $verilog_module
}

foreach vhdl_module $vhdl_module_list {
	read_vhdl -vhdl2008 $vhdl_module
}

foreach edf_file $edf_list {
	read_edif $edf_file
}

foreach xci_module $xci_list {
	read_ip $xci_module
}

foreach script [glob_wo_error ${PLATFORM_DIR}/user/fpga/common/xci/generate_*.tcl] {
	file copy -force $script ./xci
}

set include_list [concat $verilog_include_list $vhdl_include_list]
puts $include_list
set_property include_dirs $include_list [current_fileset]
set_property verilog_define $define_list [current_fileset]

touch ${IMP_RESULT_DIR}/read_source

################
## top module ##
################

puts [format "\[INFO\] TOP MODULE: %s" ${TOP_MODULE}]
set_property top ${TOP_MODULE} [current_fileset]

#############
## gen XCI ##
#############

foreach script [glob_wo_error ./xci/generate_*.tcl] {
	source $script
}

#############
## gen ILA ##
#############

set ILA_GEN_FILE ${ILA_RESULT_DIR}/generate_ila.tcl
if [string equal $IMP_MODE "debug"] {
	if {[file exist ${ILA_GEN_FILE}]} {
		source ${ILA_GEN_FILE}
	} else {
		puts "\[WARNING\] No ILA is generated"
	}
}

#############
## syn xci ##
#############

foreach xci_module [get_ips] {
	upgrade_ip -quiet $xci_module
	synth_ip -quiet $xci_module
}

touch ${IMP_RESULT_DIR}/syn_xci

###############
## elaborate ##
###############

synth_design -rtl

touch ${IMP_RESULT_DIR}/syn_rtl

#########################
## pinmap & constraint ##
#########################

source ${IMP_ENV_FILE}

foreach ucf_file $ucf_list {
	read_xdc $ucf_file
}

touch ${IMP_RESULT_DIR}/set_xdc

#####################
## generated_clock ##
#####################

set GENERATED_CLOCK_FILE ${CURRENT_DIR}/create_generated_clock.tcl
if {[file exist ${GENERATED_CLOCK_FILE}]} {
	source ${GENERATED_CLOCK_FILE}
	touch ${IMP_RESULT_DIR}/set_generated_clock
}

################
## false_path ##
################

set SET_FALSE_PATH_FILE ${CURRENT_DIR}/set_false_path.tcl
if {[file exist ${SET_FALSE_PATH_FILE}]} {
	source ${SET_FALSE_PATH_FILE}
	touch ${IMP_RESULT_DIR}/set_false_path
}
