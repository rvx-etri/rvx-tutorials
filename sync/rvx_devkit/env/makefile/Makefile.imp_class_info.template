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
## 2021-03-23
## Kyuseung Han (han@etri.re.kr)
## ****************************************************************************
## ****************************************************************************

ifndef RVX_ETRI_HOME
ifndef RVX_MINI_HOME
$(error source the rvx_setup script in RVX_ETRI_HOME or RVX_MINI_HOME)
endif
endif

ifdef RVX_MINI_HOME
	include ${RVX_MINI_HOME}/rvx_config.mh
else
	include ${RVX_DEVKIT_HOME}/rvx_config.mh
endif

ifdef RVX_MINI_HOME
	ORIGINAL_DIR=${RVX_MINI_HOME}/rvx_install/mini_git/imp_class_info
else
	ORIGINAL_DIR=${RVX_ETRI_HOME}/imp_class_info
endif

TEMPLATE_FILE = ${RVX_ENV}/makefile/Makefile.imp_class_info.template
include ${RVX_UTIL_HOME}/template_makefile.mh

ALL_FPGA_LIST = $(shell ${PYTHON3_CMD} ${RVX_UTIL_HOME}/os_util.py dir_list ${ORIGINAL_DIR} )
FPGA_LIST = $(shell ${PYTHON3_CMD} ${RVX_UTIL_HOME}/os_util.py dir_list ${CURDIR} )

list:
	@echo ${FPGA_LIST}

list_all:
	@echo ${ALL_FPGA_LIST}

reimport: $(addsuffix .reimport, $(FPGA_LIST))

$(addsuffix .reimport, $(FPGA_LIST)):%.reimport:
ifneq (${CURDIR},${ORIGINAL_DIR})
	@if [ -d ${ORIGINAL_DIR}/$(*) ] ;	\
	then \
		rm -rf $(*); \
		cp -r ${ORIGINAL_DIR}/$(*) .; \
		rm -rf $(*)/doc; \
	fi
else
	@echo "NOT allowed at the original directory"
endif

$(addsuffix .import, $(ALL_FPGA_LIST)):%.import:
ifneq (${CURDIR},${ORIGINAL_DIR})
	if [ -d ${ORIGINAL_DIR}/$(*) ] ;	\
	then \
		rm -rf $(*); \
		cp -r ${ORIGINAL_DIR}/$(*) .; \
		rm -rf $(*)/doc; \
	fi
else
	@echo "NOT allowed at the original directory"
endif

