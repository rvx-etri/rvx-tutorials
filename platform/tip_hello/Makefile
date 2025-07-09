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

ifdef RVX_MINI_HOME
	include ${RVX_MINI_HOME}/rvx_config.mh
else
	include ${RVX_DEVKIT_HOME}/rvx_config.mh
endif

TEMPLATE_FILE = ${RVX_ENV}/makefile/Makefile.platform.template
include ${RVX_UTIL_HOME}/template_makefile.mh

PLATFORM_DIR = ${CURDIR}
PLATFORM_NAME = $(notdir ${PLATFORM_DIR})
RVX_WORKSPACE = $(call absolute_path,${PLATFORM_DIR}/..)

USER_DIR = ./user

APP_DIR = ./app

SIM_RTL_DIRNAME = sim_rtl
SIM_RTL_DIR = ${CURDIR}/${SIM_RTL_DIRNAME}

PLATFORM_XML_FILE=${PLATFORM_DIR}/${PLATFORM_NAME}.xml

-include ./each_platform.mh

ifndef TARGET_IMP_CLASS
ifdef FPGA_NAME
	TARGET_IMP_CLASS = ${FPGA_NAME}
endif
endif

ifndef TARGET_IMP_CLASS
ifdef DEFAULT_FPGA_NAME
	TARGET_IMP_CLASS = ${DEFAULT_FPGA_NAME}
endif
endif

ENGINE_WS=${PLATFORM_DIR}/..
ENGINE_TARGET=platform
ENGINE_USER_CMD_LIST=clean syn sim_rtl imp_fpga clean_all_imp app_list fpga_list
ENGINE_USER_CMD_ADV_LIST=user util freeze unfreeze
ENGINE_SYSTEM_CMD_LIST=testbench app_base system_app distclean syn_arch sim_vp sim_rtl_syn imp_fpga_dir gui imp_chip wifi_config aix_config
ENGINE_PARA= -p ${PLATFORM_NAME}
ifdef TARGET_IMP_CLASS
ENGINE_PARA+= -tic ${TARGET_IMP_CLASS}
endif

ifndef IMP_INSTANCE_DIR
ifdef IMP_INSTANCE_NAME
	IMP_INSTANCE_DIR = ${PLATFORM_DIR}/${IMP_INSTANCE_NAME}
endif
endif

ifdef IMP_INSTANCE_DIR
ENGINE_PARA+= -imp_dir ${IMP_INSTANCE_DIR}
endif

MAKEFILE_USER_CMD_LIST=test verify edit
MAKEFILE_USER_CMD_ADV_LIST=
include ${RVX_ENV}/engine/rvx_make2engine.mh

ifndef TEST_APP_NAME
	TEST_APP_NAME=hello
endif
ifndef VERIFY_APP_NAME
	VERIFY_APP_NAME=${TEST_APP_NAME}
endif

-include ${RVX_ENV}/makefile/platform.mh
-include ${RVX_ENV}/cloud/platform.mh

########
# fpga #
########

FPGA_LIST = $(shell ${PYTHON3_CMD} ${RVX_UTIL_HOME}/os_util.py dir_list ${ENGINE_WS}/../imp_class_info )

${FPGA_LIST}:
	@make imp_fpga TARGET_IMP_CLASS=$@

fpga: imp_fpga

########
# chip #
########

CHIP_LIST = $(shell ${PYTHON3_CMD} ${RVX_UTIL_HOME}/os_util.py dir_list ${ENGINE_WS}/../hwlib_chip )

${CHIP_LIST}:
	@make imp_chip TARGET_IMP_CLASS=$@

########
# test #
########

edit:
	@vim ${PLATFORM_XML_FILE}

test: test_rtl_app

verify: verify_rtl_app

.PHONY: imp_fpga sim_vp_cloud

