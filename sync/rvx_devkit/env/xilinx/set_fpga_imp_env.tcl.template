###########################
# DO NOT MODIFY THE BELOW #
###########################

#set IMP_FPGA_XDC_FILE and/or IMP_FPGA_PINMAP_FILE in USER_FPGA_IMP_ENV_FILE for customization
set USER_FPGA_IMP_ENV_FILE ${PLATFORM_DIR}/user/env/set_fpga_imp_env.tcl
if {[file exist ${USER_FPGA_IMP_ENV_FILE}]} {
	source ${USER_FPGA_IMP_ENV_FILE}
}

if {[file exist ${IMP_FPGA_XDC_FILE}]} {
	source ${IMP_FPGA_XDC_FILE}
} else {
  lappend ucf_list {*}[glob -nocomplain ${FPGA_DEPENDENT_DIR}/xdc/*.xdc]
}

if {[file exist ${IMP_FPGA_PINMAP_FILE}]} {
	source ${IMP_FPGA_PINMAP_FILE}
} else {
  lappend ucf_list {*}[glob -nocomplain ${FPGA_DEPENDENT_DIR}/pinmap/*.xdc]
}
