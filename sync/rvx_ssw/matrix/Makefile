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
# 2018-11-29
# Kyuseung Han (han@etri.re.kr)
# ****************************************************************************
# ****************************************************************************

ifndef RVX_ETRI_HOME
$(error source the source file in RVX_ETRI_HOME)
endif

HEADER_LIST=access
OP3_LIST=add sub ewmult mult scalar_mult mac conv 
OP2_LIST=transpose downsample

OP_LIST= ${OP3_LIST} ${OP2_LIST}

gen: generate

generate: generate_case

generate_header: $(addprefix generate_header., ${HEADER_LIST})

$(addprefix generate_header., ${HEADER_LIST}):generate_header.%:
	if [ -f ./template/ervp_matrix_$(*).h.template ] ;	\
	then \
		python3 ${RVX_UTIL_HOME}/configure_template.py -i ./template/ervp_matrix_$(*).h.template -o ./src/ervp_matrix_$(*)_float.h -c CASE_NAME=float DATA_TYPE=float DATA_TYPE_UPPER=FLOAT; \
		python3 ${RVX_UTIL_HOME}/configure_template.py -i ./template/ervp_matrix_$(*).h.template -o ./src/ervp_matrix_$(*)_fixed.h -c CASE_NAME=fixed DATA_TYPE=fixed DATA_TYPE_UPPER=FIXED; \
	fi

generate_case: $(addprefix generate_case., ${OP_LIST})

$(addprefix generate_case., ${OP_LIST}):generate_case.%:
	if [ -f ./template/ervp_matrix_$(*)_case.c.template ] ;	\
	then \
		python3 ${RVX_UTIL_HOME}/configure_template.py -i ./template/ervp_matrix_$(*)_case.c.template -o ./src/ervp_matrix_$(*)_float_sw.c -c OP_TYPE=$(*) CASE_NAME=float DATA_TYPE=float DATA_TYPE_UPPER=FLOAT; \
		python3 ${RVX_UTIL_HOME}/configure_template.py -i ./template/ervp_matrix_$(*)_case.c.template -o ./src/ervp_matrix_$(*)_fixed_sw.c -c OP_TYPE=$(*) CASE_NAME=fixed DATA_TYPE=int DATA_TYPE_UPPER=FIXED; \
	fi;

generate_op: $(addprefix generate_op., ${OP_LIST})
