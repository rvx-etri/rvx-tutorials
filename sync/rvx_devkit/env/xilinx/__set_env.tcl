source ${RVX_ENV}/xilinx/util.tcl

## shared directory
set ENV_DIR ${RVX_ENV}/xilinx

## private directory
set CURRENT_DIR [pwd]
set IMP_RESULT_DIR ${CURRENT_DIR}/imp_result
set ILA_RESULT_DIR ${CURRENT_DIR}/ila

## define
set SYN_ENV_FILE ./set_fpga_syn_env.tcl
set SYN_COMMON_LIB_ENV_FILE ${RVX_HWLIB_HOME}/lib_fpga/env/set_syn_env.tcl
set SYN_FPGA_SRC_FILE ${FPGA_DEPENDENT_DIR}/env/set_fpga_src.tcl

set IMP_ENV_FILE ./set_fpga_imp_env.tcl
set IMP_FPGA_XDC_FILE ${FPGA_DEPENDENT_DIR}/env/set_fpga_xdc.tcl
set IMP_FPGA_PINMAP_FILE ${FPGA_DEPENDENT_DIR}/env/set_fpga_pinmap.tcl

set TOOL_OPTION_FILE ./set_tool_option.tcl

set MANUAL_SYN_IMPDIR_FILE ./syn_manually.tcl
set MANUAL_PNR_IMPDIR_FILE ./pnr_manually.tcl

set MANUAL_SYN_USER_FILE ${PLATFORM_DIR}/user/fpga/${FPGA_NAME}/syn_manually.tcl
set MANUAL_PNR_USER_FILE ${PLATFORM_DIR}/user/fpga/${FPGA_NAME}/pnr_manually.tcl
