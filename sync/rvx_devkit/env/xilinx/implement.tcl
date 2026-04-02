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

###################
## init variable ##
###################

set define_list {}
set verilog_module_list {}
set verilog_include_list {}
set vhdl_module_list {}
set vhdl_include_list {}
set presyn_list {}
set edf_list {}
set xci_list {}
set ucf_list {}

#################
## environment ##
#################

source ./set_path.tcl
source ./set_design.tcl
source ${RVX_ENV}/xilinx/__set_env.tcl

###############
## run check ##
###############

if {[file exist ${IMP_RESULT_DIR}/${BIT_FILE_NAME}.bit]} {
	puts "\[INFO\] Bit file already exists. Please clean if you want to rerun\n\n"
	exit 0
}

##################
## make project ##
##################

if {[file exist ${IMP_RESULT_DIR}/${PROJECT_NAME}.xpr]} {
	open_project ${IMP_RESULT_DIR}/${PROJECT_NAME}
} else {
	source ${RVX_ENV}/xilinx/__make_project.tcl
}

###############
## implement ##
###############

source ${RVX_ENV}/xilinx/__implement.tcl

####################
## close project  ##
####################

close_project
