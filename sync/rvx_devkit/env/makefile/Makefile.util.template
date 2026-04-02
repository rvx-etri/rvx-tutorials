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
## 2022-10-12
## Kyuseung Han (han@etri.re.kr)
## ****************************************************************************
## ****************************************************************************

ifdef RVX_MINI_HOME
	include ${RVX_MINI_HOME}/rvx_config.mh
else
	include ${RVX_DEVKIT_HOME}/rvx_config.mh
endif

TEMPLATE_FILE = ${RVX_ENV}/makefile/Makefile.util.template
include ${RVX_UTIL_HOME}/template_makefile.mh

UTIL_LIST=mmio rom image fakefile npx arx

$(addsuffix .base, ${UTIL_LIST}):%.base:
	@${PYTHON3_CMD} ${RVX_UTIL_HOME}/os_util.py mkdir ${CURDIR}/$(*)
	@${PYTHON3_CMD} ${RVX_UTIL_HOME}/os_util.py cp_file ${RVX_ENV}/util/Makefile.$(*).template ${CURDIR}/$(*)/Makefile
	@${PYTHON3_CMD} ${RVX_UTIL_HOME}/os_util.py cp_file ${RVX_ENV}/util/gitignore.$(*).txt ${CURDIR}/$(*)/.gitignore

fakefile: FORCE fakefile.base
	@${PYTHON3_CMD} ${RVX_UTIL_HOME}/os_util.py mkdir ${CURDIR}/fakefile/target

npx: FORCE npx.base
	@${PYTHON3_CMD} ${RVX_UTIL_HOME}/os_util.py mkdir ${CURDIR}/npx/app

arx: FORCE arx.base

generated: FORCE
	@${PYTHON3_CMD} ${RVX_UTIL_HOME}/os_util.py mkdir ${CURDIR}/generated
	@${PYTHON3_CMD} ${RVX_UTIL_HOME}/os_util.py cp_file ${RVX_ENV}/util/gitignore.generated.txt ${CURDIR}/generated/.gitignore

clean: $(addsuffix .clean, ${UTIL_LIST})

$(addsuffix .clean, ${UTIL_LIST}):%.clean:
	-@cd $(*) && make --no-print-directory clean

-include ./user_util.mh

FORCE:

.PHONY: FORCE clean
