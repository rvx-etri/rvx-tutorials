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
## 2019-04 : Kyuseung Han (han@etri.re.kr)
## ****************************************************************************
## ****************************************************************************

###############
## synthesis ##
###############

source ${TOOL_OPTION_FILE}

if {[file exist ${MANUAL_SYN_IMPDIR_FILE}]} {
	puts "\[INFO\] Synthesize using manual options from imp dir"
	source ${MANUAL_SYN_IMPDIR_FILE}
} elseif {[file exist ${MANUAL_SYN_USER_FILE}]} {
	puts "\[INFO\] Synthesize using manual options from user dir"
	source ${MANUAL_SYN_USER_FILE}
} else {
	if [string equal ${SYN_DESIGN_OPTION} {}] {
		synth_design
	} else {
		eval [concat {synth_design} ${SYN_DESIGN_OPTION}]
	}
}

touch ${IMP_RESULT_DIR}/syn_top
write_checkpoint -force ${IMP_RESULT_DIR}/synthed

#########
## CDC ##
#########

# DO NOT MOVE THIS PART BEFORE synth_design

set clock_list [all_clocks]
puts [format "\[INFO\] clock list: %s" $clock_list]
foreach clock_a $clock_list {
	foreach clock_b $clock_list {
		if {$clock_a!=$clock_b} {
			set_false_path -from [get_clocks $clock_a] -to [get_clocks $clock_b]
		}
	}
}

touch ${IMP_RESULT_DIR}/set_cdc

#########
## pnr ##
#########

if {[file exist ${MANUAL_PNR_IMPDIR_FILE}]} {
	puts "\[INFO\] Place and route using manual options from imp dir"
	source ${MANUAL_PNR_IMPDIR_FILE}
} elseif {[file exist ${MANUAL_PNR_USER_FILE}]} {
	puts "\[INFO\] Place and route using manual options from user dir"
	source ${MANUAL_PNR_USER_FILE}
} else {
	puts "\[INFO\] Place and route quickly"
	if [string equal ${OPT_DESIGN_OPTION} {}] {
		opt_design
	} else {
		eval [concat {opt_design} ${OPT_DESIGN_OPTION}]
	}
	if [string equal ${PLACE_DESIGN_OPTION} {}] {
		place_design
	} else {
		eval [concat {place_design} ${PLACE_DESIGN_OPTION}]
	}
	if [string equal ${ROUTE_DESIGN_OPTION} {}] {
		route_design
	} else {
		eval [concat {route_design} ${ROUTE_DESIGN_OPTION}]
	}
}
touch ${IMP_RESULT_DIR}/route_top
write_checkpoint -force ${IMP_RESULT_DIR}/routed

report_route_status -file ${IMP_RESULT_DIR}/route_status.rpt
report_timing_summary -file ${IMP_RESULT_DIR}/route_timing_summary.rpt
report_timing -sort_by group -max_paths 100 -path_type summary -file ${IMP_RESULT_DIR}/route_timing_max.rpt
report_clock_utilization -file ${IMP_RESULT_DIR}/clock_util.rpt
report_utilization -file ${IMP_RESULT_DIR}/route_util.rpt
report_utilization -hierarchical  -file ${IMP_RESULT_DIR}/route_util_hier.rpt
report_power -file ${IMP_RESULT_DIR}/route_power.rpt
report_power -hier all -hierarchical_depth 10 -file ${IMP_RESULT_DIR}/route_power_hier.rpt
report_io -file ${IMP_RESULT_DIR}/route_io.rpt 
write_verilog -force ${IMP_RESULT_DIR}/${TOP_MODULE}.v

###############
## bitstream ##
###############

write_bitstream -force ${IMP_RESULT_DIR}/${BIT_FILE_NAME}.bit
#write_device_image -force -file ${IMP_RESULT_DIR}/${BIT_FILE_NAME}.pdi
if [string equal $IMP_MODE "debug"] {
	write_debug_probes -force ${IMP_RESULT_DIR}/${BIT_FILE_NAME}.ltx
}

touch ${IMP_RESULT_DIR}/gen_bit
