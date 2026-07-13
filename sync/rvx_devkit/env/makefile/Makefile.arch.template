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

TEMPLATE_FILE = ${RVX_ENV}/makefile/Makefile.arch.template
include ${RVX_UTIL_HOME}/template_makefile.mh

export PLATFORM_DIR=$(call absolute_path,${CURDIR}/..)
PLATFORM_NAME = $(notdir ${PLATFORM_DIR})

ORIGINAL_FILE_LIST=./Makefile 
include ${RVX_UTIL_HOME}/distclean.mh

SYN_DIR = ${CURDIR}

XML_DIR = ${SYN_DIR}/xml
SW_SRC_DIR = ${SYN_DIR}/ssw/src
SW_ENV_DIR = ${SYN_DIR}/ssw/env
RTL_INCLUDE_DIR = ${SYN_DIR}/rtl/include
RTL_DIR = ${SYN_DIR}/rtl
RTL_SRC_DIR = ${SYN_DIR}/rtl/src
RTL_ENV_DIR = ${SYN_DIR}/rtl/env
IROM_DIR = ${SYN_DIR}/rtl/irom
TCL_DIR = ${SYN_DIR}/tcl
LOCAL_SSW_DIR = ${PLATFORM_DIR}/local_ssw

PLATFORM_XML = ${PLATFORM_DIR}/${PLATFORM_NAME}.xml
INFO_XML = ${XML_DIR}/platform_info.xml

IP_INSTANCE_INFO_XML = ${XML_DIR}/ip_instance_info.xml

MMIO_XML = ${XML_DIR}/member_mmio.xml
MMIO_MMAP_OFFSET_XML = ${XML_DIR}/member_mmio_memorymap_offset.xml
MMIO_MMAP_XML = ${XML_DIR}/member_mmio_memorymap.xml
MMIO_MMAP_C = ${SW_SRC_DIR}/member_mmio_memorymap.h
SYNTHESIZED_MMIO_XML = ${XML_DIR}/member_mmio_synthesized.xml

PACT_CONFIG_XML=${XML_DIR}/pact_config.xml
PACT_USER_NODE_XML=${XML_DIR}/pact_user_node.xml

MEMBER_ADC_XML=${XML_DIR}/member_adc.xml

var:
	@echo $(PLATFORM_NAME)

all: syn

clean: distclean

syn: syn_arch gen_c_info gen_sw_env gen_rtl_info gen_tcl_info gen_rtl_env gen_ip_instance_info gen_mmio gen_pact irom gen_adc

syn_arch:
	-@rm -rf ${XML_DIR}
	@mkdir -p ${XML_DIR}
	-@rm -rf ${SW_SRC_DIR}
	@mkdir -p ${SW_SRC_DIR}
	-@rm -rf ${RTL_INCLUDE_DIR}
	@mkdir -p ${RTL_INCLUDE_DIR}
	-@rm -rf ${RTL_SRC_DIR}
	@mkdir -p ${RTL_SRC_DIR}
	-@rm -rf ${TCL_DIR}
	@mkdir -p ${TCL_DIR}
	@cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory syn_arch INPUT_XML=${PLATFORM_XML} OUTPUT_DIR=${XML_DIR}
	@mv ${XML_DIR}/*.v ${RTL_SRC_DIR}/
	@mv ${XML_DIR}/*.v? ${RTL_INCLUDE_DIR}/

gen_c_info:
	@cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory gen_info INPUT_XML=${INFO_XML} LANGAUGE_TYPE=c OUTPUT_DIR=${SW_SRC_DIR}

gen_rtl_info:
	@cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory gen_info INPUT_XML=${INFO_XML} LANGAUGE_TYPE=v OUTPUT_DIR=${RTL_INCLUDE_DIR}

gen_tcl_info:
	@cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory gen_info INPUT_XML=${INFO_XML} LANGAUGE_TYPE=t OUTPUT_DIR=${TCL_DIR}

gen_sw_env:
	-@rm -rf ${SW_ENV_DIR}
	@mkdir -p ${SW_ENV_DIR}
	@cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory gen_sw_env PLATFORM_DIR=${PLATFORM_DIR} INCLUDE_LIST="${SW_SRC_DIR}/memorymap_info.h ${SW_SRC_DIR}/hw_info.h ${SW_SRC_DIR}/ssw_info.h ${LOCAL_SSW_DIR}/ervp_wifi_info.h" OUTPUT_DIR=${SW_ENV_DIR}

gen_rtl_env:
	-@rm -rf ${RTL_ENV_DIR}
	@mkdir -p ${RTL_ENV_DIR}
	@cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory gen_rtl_sim_env INCLUDE_LIST="${RTL_INCLUDE_DIR}/hw_info.vh" OUTPUT_DIR=${RTL_ENV_DIR}
	@cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory gen_rtl_syn_env INCLUDE_LIST="${RTL_INCLUDE_DIR}/hw_info.vh" OUTPUT_DIR=${RTL_ENV_DIR}

irom:
	-@rm -rf ${IROM_DIR}
	@mkdir -p ${IROM_DIR}
	@cp ${RVX_ENV}/makefile/Makefile.irom.template ${IROM_DIR}/Makefile
	@cd ${IROM_DIR} && make --no-print-directory all

gen_ip_instance_info:
	@if [ -f ${IP_INSTANCE_INFO_XML} ]; \
	then \
		cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory gen_ip_instance_info INPUT_XML=${IP_INSTANCE_INFO_XML} OUTPUT_DIR=${SW_SRC_DIR}; \
	fi

gen_mmio:
	@if [ -f ${MMIO_XML} ]; \
	then \
		cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory gen_mmio INPUT_XML=${MMIO_XML} OUTPUT_DIR=${XML_DIR} OPERATION=s; \
		cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory gen_mmio INPUT_XML=${SYNTHESIZED_MMIO_XML} OUTPUT_DIR=${XML_DIR} OPERATION=rdm; \
		cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory gen_info INPUT_XML=${MMIO_MMAP_OFFSET_XML} LANGAUGE_TYPE=v OUTPUT_DIR=${RTL_INCLUDE_DIR}; \
		cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory gen_info INPUT_XML=${MMIO_MMAP_OFFSET_XML} LANGAUGE_TYPE=c OUTPUT_DIR=${SW_SRC_DIR}; \
		cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory gen_info INPUT_XML=${MMIO_MMAP_OFFSET_XML} LANGAUGE_TYPE=t OUTPUT_DIR=${TCL_DIR}; \
		cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory gen_info INPUT_XML=${MMIO_MMAP_XML} LANGAUGE_TYPE=c OUTPUT_DIR=${SW_SRC_DIR}; \
		${PYTHON3_CMD} ${RVX_UTIL_HOME}/configure_template.py -i ${MMIO_MMAP_C} -o ${MMIO_MMAP_C} -c MODULE_BASEADDR=I_MEMBER_MMIO_BASEADDR; \
		cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory gen_mmio INPUT_XML=${SYNTHESIZED_MMIO_XML} OUTPUT_DIR=${RTL_SRC_DIR} OPERATION=rv; \
		${PYTHON3_CMD} ${RVX_UTIL_HOME}/configure_template.py -i ${RTL_SRC_DIR}/member_mmio.v -o ${RTL_SRC_DIR}/member_mmio.v -c MODULE_NAME=MEMBER_MMIO MEMORYMAP_OFFSET_FILE=member_mmio_memorymap_offset.vh; \
	fi

gen_pact:
	@if [ -f ${PACT_CONFIG_XML} ]; \
	then \
		cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory gen_info INPUT_XML=${PACT_CONFIG_XML} LANGAUGE_TYPE=v OUTPUT_DIR=${RTL_INCLUDE_DIR} ; \
		cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory gen_info INPUT_XML=${PACT_CONFIG_XML} LANGAUGE_TYPE=c OUTPUT_DIR=${SW_SRC_DIR}; \
	fi
	@if [ -f ${PACT_CONFIG_XML} ]; \
	then \
		cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory generate_pact_component OP=lsu INPUT_XML=${PACT_CONFIG_XML} OUTPUT_DIR=${RTL_DIR}; \
		cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory generate_pact_component OP=core INPUT_XML=${PACT_CONFIG_XML} OUTPUT_DIR=${RTL_DIR}; \
	fi
	@if [ -f ${PACT_USER_NODE_XML} ]; \
	then \
		cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory generate_pact_component OP=user_node INPUT_XML=${PACT_USER_NODE_XML} OUTPUT_DIR=${RTL_DIR}; \
	fi

gen_adc:
	@if [ -f ${MEMBER_ADC_XML} ]; \
	then \
		cd ${RVX_SYNTHESIZER_HOME} && make --no-print-directory gen_adc ADC_XML_LIST=${XML_DIR}/member_adc_*.xml COMPONENT=rtl OUTPUT_DIR=${RTL_SRC_DIR}; \
	fi

.PHONY: syn
