import argparse
import re
import os

from rvx_util import *
from rvx_re_util import *

from verilog_generator import *

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Generating RTL Environment')
  parser.add_argument('-info', '-i', nargs='+', help='list of platform info files')
  parser.add_argument('-output', '-o', help='output directory')

  args = parser.parse_args()
  assert args.info
  assert args.output

  define_list = []
  for file_name in args.info:
    define_list += get_define_list('`define', file_name)

  define_dict = {}
  for id, value in define_list:
    define_dict[id] = value

  ################
  ## env script ##
  ################

  line_list = []
  line_list.append('include ${MUNOC_HW_HOME}/env/set_sim_env.mh')
  line_list.append('include ${RVX_HWLIB_HOME}/peripheral/platform_controller/env/set_sim_env.mh')
  line_list.append('include ${RVX_HWLIB_HOME}/peripheral/core_peri_group/env/set_sim_env.mh')
  line_list.append('include ${RVX_HWLIB_HOME}/peripheral/common_peri_group/env/set_sim_env.mh')
  line_list.append('include ${RVX_HWLIB_HOME}/peripheral/external_peri_group/env/set_sim_env.mh')
  line_list.append('include ${RVX_HWLIB_HOME}/peripheral/mmiox/env/set_sim_env.mh')
  
  num_main_core = 0
  for define_name in define_dict.keys():
    if define_name.startswith('INCLUDE_RVC_'):
      assert num_main_core <= 2
      num_main_core += 1
      core_name = define_name[len('INCLUDE_RVC_'):].lower()
      line_list.append('include ${{RVX_HWLIB_HOME}}/core/{0}/env/set_sim_env.mh'.format(core_name))

  # library
  hwlib_basic_list = ('timer','plic','fuse_box','florian','adc','c2c','extinput_backend')
  hwlib_special_list = ('jpeg_encoder','jpegls_encoder','sdram','edge_video_system','vta','hbc1')
  hwlib_exception_list = ((('led_checker',),'peripheral/checker'),
                          (('tcaching',),'peripheral/tcu'),
                          (('i2s',),'peripheral/i2s_master'),
                          (('dma','traffic_generator',),'peripheral/dma')
                         )
  
  for hw in hwlib_basic_list:
    if f'INCLUDE_{hw.upper()}' in define_dict:
      line_list.append(f'include ${{RVX_HWLIB_HOME}}/peripheral/{hw}/env/set_sim_env.mh')

  for hw in hwlib_special_list:
    if f'INCLUDE_{hw.upper()}' in define_dict:
      line_list.append(f'{hw.upper()}_LOCAL_ENV_FILE := $(wildcard ${{PLATFORM_DIR}}/user/{hw}/env/set_sim_env.mh)')
      line_list.append(f'{hw.upper()}_GLOBAL_ENV_FILE := $(wildcard ${{RVX_SPECIAL_IP_HOME}}/{hw}/env/set_sim_env.mh)')
      line_list.append(f'ifdef {hw.upper()}_LOCAL_ENV_FILE')
      line_list.append(f'\tinclude ${{{hw.upper()}_LOCAL_ENV_FILE}}')
      line_list.append(f'else')
      line_list.append(f'ifdef {hw.upper()}_GLOBAL_ENV_FILE')
      line_list.append(f'\tinclude ${{{hw.upper()}_GLOBAL_ENV_FILE}}')
      line_list.append(f'endif')
      line_list.append(f'endif')

  for hw_list, relative_path in hwlib_exception_list:
    included = False
    for hw in hw_list:
      if f'INCLUDE_{hw.upper()}' in define_dict:
        included = True
        break
    if included:
      line_list.append(f'include ${{RVX_HWLIB_HOME}}/{relative_path}/env/set_sim_env.mh')

  # individual
  if 'INCLUDE_DCA' in define_dict:
    line_list.append('-include ${DCA_HW_HOME}/env/set_sim_env.mh')
  if 'INCLUDE_PACT' in define_dict:
    line_list.append('-include ${PACT_HW_HOME}/env/set_sim_env.mh')
  if 'INCLUDE_STARC' in define_dict:
    line_list.append('-include ${STARC_HW_HOME}/env/set_sim_env.mh')
  
  line_list.append('SRC_DIR_LIST += ${PLATFORM_DIR}/arch/rtl/src')
  line_list.append('INC_DIR_LIST += ${PLATFORM_DIR}/arch/rtl/include')

  with open(os.path.join(args.output,'set_sim_env.mh'),'w') as f:
    f.write('\n'.join(line_list))
