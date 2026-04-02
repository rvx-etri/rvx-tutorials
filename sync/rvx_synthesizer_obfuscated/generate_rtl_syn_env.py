import argparse
import re
import os

from rvx_util import *
from rvx_re_util import *

from verilog_generator import *

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Generating FPGA Environment')
  parser.add_argument('-info', '-i', nargs='+', help='platform info files')
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
  line_list.append('source ${RVX_HWLIB_HOME}/lib_rtl/env/add_syn_source.tcl')
  line_list.append('source ${MUNOC_HW_HOME}/env/add_syn_source.tcl')
  line_list.append('source ${RVX_HWLIB_HOME}/peripheral/platform_controller/env/add_syn_source.tcl')
  line_list.append('source ${RVX_HWLIB_HOME}/peripheral/core_peri_group/env/add_syn_source.tcl')
  line_list.append('source ${RVX_HWLIB_HOME}/peripheral/common_peri_group/env/add_syn_source.tcl')
  line_list.append('source ${RVX_HWLIB_HOME}/peripheral/external_peri_group/env/add_syn_source.tcl')
  line_list.append('source ${RVX_HWLIB_HOME}/peripheral/mmiox/env/add_syn_source.tcl')

  num_main_core = 0
  for define_name in define_dict.keys():
    if define_name.startswith('INCLUDE_RVC_'):
      assert num_main_core <= 2
      num_main_core += 1
      core_name = define_name[len('INCLUDE_RVC_'):].lower()
      line_list.append('source ${{RVX_HWLIB_HOME}}/core/{0}/env/add_syn_source.tcl'.format(core_name))

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
      line_list.append(f'source ${{RVX_HWLIB_HOME}}/peripheral/{hw}/env/add_syn_source.tcl')

  for hw in hwlib_special_list:
    name = hw.upper()
    if f'INCLUDE_{name}' in define_dict:
      line_list.append(f'set {name}_LOCAL_SOURCE_FILE ${{PLATFORM_DIR}}/user/{hw}/env/add_syn_source.tcl')
      line_list.append(f'set {name}_GLOBAL_SOURCE_FILE ${{RVX_SPECIAL_IP_HOME}}/{hw}/env/add_syn_source.tcl')
      line_list.append(f'if {{[file exist ${{{name}_LOCAL_SOURCE_FILE}}]}} {{')
      line_list.append(f'\tsource ${{{name}_LOCAL_SOURCE_FILE}}')
      line_list.append(f'}} elseif {{[file exist ${{{name}_GLOBAL_SOURCE_FILE}}]}} {{')
      line_list.append(f'\tsource ${{{name}_GLOBAL_SOURCE_FILE}}')
      line_list.append('}')

  for hw_list, relative_path in hwlib_exception_list:
    included = False
    for hw in hw_list:
      if f'INCLUDE_{hw.upper()}' in define_dict:
        included = True
        break
    if included:
      line_list.append(f'source ${{RVX_HWLIB_HOME}}/{relative_path}/env/add_syn_source.tcl')

  # individual
  individual_git_list = ('dca','pact','starc')
  for individual_git in individual_git_list:
    name = individual_git.upper()
    if f'INCLUDE_{name}' in define_dict:
      line_list.append(f'set {name}_SOURCE_FILE ${{{name}_HW_HOME}}/env/add_syn_source.tcl')
      line_list.append(f'if {{[file exist ${{{name}_SOURCE_FILE}}]}} {{')
      line_list.append(f'\tsource ${{{name}_SOURCE_FILE}}')
      line_list.append('}')

  line_list.append('')
  line_list.append('set verilog_module_list [concat $verilog_module_list [glob ${PLATFORM_DIR}/arch/rtl/src/*.v]]')
  line_list.append('lappend verilog_include_list ${PLATFORM_DIR}/arch/rtl/include')

  with open(os.path.join(args.output,'set_rtl_syn_env.tcl'),'w') as f:
    f.write('\n'.join(line_list))
