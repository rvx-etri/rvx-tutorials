# ****************************************************************************
# ****************************************************************************
# Copyright SoC Design Research Group, All rights reserved.
# Electronics and Telecommunications Research Institute (ETRI)
# 
# THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE
# WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS
# TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE
# REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL
# SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE,
# IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE
# COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.
# ****************************************************************************
# 2019-04-03
# Kyuseung Han (han@etri.re.kr)
# ****************************************************************************
# ****************************************************************************

ifndef RVX_SHARED_HOME
	$(error source the rvs_setup script in RVX_SHARED_HOME)
endif

include ${RVX_SHARED_HOME}/rvx_python_config.mh
-include ${RVX_UTIL_HOME}/template_makefile.mh
-include ./env/cloud/engine.mh

TEMP_FILE=./temp.txt

init:
	@echo "Read README.md"

config: config_rvx_setup

config_rvx_setup:
	#@python3 ${RVX_UTIL_HOME}/allocate_gdb_port.py -ws ${RVX_SHARED_HOME} -u ${USER} -o ${TEMP_FILE}
	#@python3 ${RVX_UTIL_HOME}/configure_template.py -i ${RVX_UTIL_HOME}/rvx_devkit.source -o ./source -c RVX_SHARED_HOME=${RVX_SHARED_HOME} RVX_DEVKIT_HOME=$(CURDIR) RV_GDB_PORT=`cat ${TEMP_FILE}`
	#@python3 ${RVX_UTIL_HOME}/configure_template.py -i ./env/makefile/gdb_script.template -o ./env/makefile/gdb_script -c RV_GDB_PORT=`cat ${TEMP_FILE}`
	#-@rm -rf ${TEMP_FILE}
	${PYTHON3_CMD} ${RVX_UTIL_HOME}/configure_template.py -i ./rvx_setup.sh.template -o ./rvx_setup.sh -c RVX_SHARED_HOME=${RVX_SHARED_HOME} RVX_DEVKIT_HOME=$(CURDIR) RV_GDB_PORT=0

clean:
	-@rm -rf *.tar *.tar.gz *.tar.xz *.log *.txt .rvx_* *.log *.xz *.tgz
	-@rm -rf ./src
	-@cd platform && make clean	

.PHONY: init update config
