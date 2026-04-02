ifdef RVX_MINI_HOME
	include ${RVX_MINI_HOME}/rvx_config.mh
else
	include ${RVX_DEVKIT_HOME}/rvx_config.mh
endif

TEMPLATE_FILE = ${RVX_ENV}/util/Makefile.arx.template
include ${RVX_UTIL_HOME}/template_makefile.mh

-include ./rvx_each.mh

ARX_API_GIT_VERSION_STABLE=89bcda
ARX_API_GIT_VERSION?=${ARX_API_GIT_VERSION_STABLE}
ARX_API_DIR=${CURDIR}/arx-api
APP_NAME_LIST=mnist small_charRNN

check:
	@echo ${ARX_API_GIT_VERSION}
	@echo ${APP_NAME_LIST}

all: arx_all

clean:
	@true

reall: arx_api_clean all

setup: arx_api_setup

arx_all: onnx arx_api

onnx:
	${PYTHON3_CMD} -m pip install onnx onnxruntime

arx_api_clean:
	rm -rf ${ARX_API_DIR}

arx_api: arx_api_clone arx_api_setup arx_api_build arx_api_fix

arx_api_clone:
	@if [ ! -d ${ARX_API_DIR} ]; \
	then \
		git clone https://gitlab.com/ones-ai/arx-api.git ${ARX_API_DIR}; \
	fi

arx_api_setup:
	@cd ${ARX_API_DIR} && git pull origin main
	@cd ${ARX_API_DIR} && git checkout ${ARX_API_GIT_VERSION}

$(addprefix arx_api_build., $(APP_NAME_LIST)):arx_api_build.%:
	cd ${ARX_API_DIR}/test/board_test/$(*) && make build

arx_api_build: $(addprefix arx_api_build., $(APP_NAME_LIST))

arx_api_fix:
	cd ${ARX_API_DIR} && git checkout .
	#cd ${ARX_API_DIR} && find -type f -exec sed -i "s/\"..\/include\//\"/g" {} \;
	#cd ${ARX_API_DIR} && find -type f -exec sed -i "s/ervp_memory_util/ervp_malloc/g" {} \;

FORCE:

.PHONY: FORCE clean
