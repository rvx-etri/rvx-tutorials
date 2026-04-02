# ****************************************************************************
# ****************************************************************************
# Cright SoC Design Research Group, All rights reserved.
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

ORIGINAL_FILE_LIST = ./git ./Makefile ./synthesizer_common.mh
REMOVE_FILE_LIST = $(filter-out ${ORIGINAL_FILE_LIST}, $(wildcard ./*))

-include ./synthesizer_common.mh

default:

syn_arch:
	@./rvx_synthesizer -xml ${INPUT_XML} -d ${OUTPUT_DIR}

empty:
	-@rm -rf ${REMOVE_FILE_LIST}

test:
	@echo ${PYTHON3_CMD}
