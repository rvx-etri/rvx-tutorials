import argparse
import re
import os
from pathlib import Path

from rvx_re_util import *

# from configure_template.py in rvx_util
def configure_template_text(contents:str, conv_info=None):
  if not conv_info:
    config_line = ''
    conv_dict = {}
  elif type(conv_info)==dict:
    config_line = ' '.join([f'{x}={y}' for x, y in conv_info.items()])
    conv_dict = conv_info
  elif type(conv_info)==list or type(conv_info)==tuple:
    config_line = ' '.join([f'{x}={y}' for x, y in conv_info])
    conv_dict = {}
    for before_word, after_word in conv_info:
      assert before_word
      conv_dict[before_word] = after_word
  #
  contents = re.sub('\"\${TEMPLATE_CONFIG}\"', config_line, contents)
  #
  reexp_template_var = '\"\${' + memorize(reexp_identifier) + '}\"'
  re_template_var = re.compile(reexp_template_var)
  contents = re_template_var.sub(lambda x: conv_dict[x.group(1)], contents)
  return contents

class MemoryStatus():
  def __init__(self, define_dict:dict):
    self.define_dict = define_dict

  @property
  def include_small_ram(self):
    return 'INCLUDE_SMALL_RAM' in define_dict;
  @property
  def include_cache(self):
    return 'INCLUDE_CACHE' in define_dict
  @property
  def include_large_ram(self):
    return 'INCLUDE_LARGE_RAM' in define_dict
  @property
  def use_small_ram(self):
    return 'USE_SMALL_RAM' in define_dict
  @property
  def use_large_ram(self):
    return 'USE_LARGE_RAM' in define_dict

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description='Generating Linker Script')
  parser.add_argument('-platform_dir', '-p', help='platform directory')
  parser.add_argument('-info', '-i', nargs='+', help='platform info files')
  parser.add_argument('-template', '-t', help='linker template directory')  
  parser.add_argument('-output', '-o', help='output directory')

  args = parser.parse_args()
  assert args.platform_dir
  assert args.template
  assert args.info
  assert args.output

  info_path_list = [Path(x).resolve() for x in args.info]

  define_list = []
  for info_path in info_path_list:
    if info_path.is_file():
      define_list += get_define_list('#define', info_path)
  
  define_dict = {}
  for id, value in define_list:
    define_dict[id] = value

  output_dir = Path(args.output).resolve()
  assert output_dir.is_dir(), output_dir

  ###################
  ## linker script ##
  ###################

  memory_status = MemoryStatus(define_dict)

  sram_section_list = []
  dram_section_list = []

  check_pair = (('SINGLE_WRITE_MEMORY_REGION', 'link_section_sw.ld'),
                ('MULTI_WRITE_MEMORY_REGION', 'link_section_mw.ld'),
                ('UNCACHEABLE_MEMORY_REGION', 'link_section_uc.ld'),
               )

  for define_name, section_file in check_pair:
    if define_dict[define_name]=='sram':
      sram_section_list.append(section_file)
    else:
      dram_section_list.append(section_file)

  if memory_status.use_small_ram:
    sram_section_list.append('link_section_heap_sram.ld')
  if memory_status.use_large_ram:
    dram_section_list.append('link_section_heap_dram.ld')

  linker_file_list = []
  if memory_status.use_small_ram:
    linker_file_list.append('link_memory_sram.ld')
  if memory_status.use_large_ram:
    linker_file_list.append('link_memory_dram.ld')
  linker_file_list.append('link_section_prolog.ld')
  if memory_status.use_small_ram:
    linker_file_list.append('link_section_sram_start.ld')
    linker_file_list += sram_section_list
    linker_file_list.append('link_section_sram_end.ld')
  if memory_status.use_large_ram:
    linker_file_list.append('link_section_dram_start.ld')
    linker_file_list += dram_section_list
    linker_file_list.append('link_section_dram_end.ld')
  else:
    assert len(dram_section_list)==0
  linker_file_list.append('link_section_epilog.ld')

  # concatenate
  linker_template_path = Path(args.template)
  assert linker_template_path.is_dir()
  file_contents = ''
  for linker_file_name in linker_file_list:
    file_contents += (linker_template_path / linker_file_name).read_text()
  
  # replace
  file_contents = configure_template_text(file_contents, define_dict)

  linker_file = output_dir / 'link.ld'
  linker_file.write_text(file_contents)

  ###################
  ## memory script ##
  ###################


  line_list = []
  if 'USE_SMALL_RAM' in define_dict:
    line_list.append('MEMORY_CONFIG= -sram {0} {1}'.format(define_dict['SMALL_RAM_BASEADDR'],define_dict['SMALL_RAM_SIZE']))
  if 'USE_LARGE_RAM' in define_dict:
    line_list.append('MEMORY_CONFIG+= -dram {0} {1}'.format(define_dict['LARGE_RAM_BASEADDR'],define_dict['LARGE_RAM_SIZE']))

  set_memory = output_dir / 'set_memory.mh'
  set_memory.write_text('\n'.join(line_list))

  ################
  ## env script ##
  ################

  base_directory = os.path.abspath(args.platform_dir)
  include_dir_set = set()
  for info_path in info_path_list:
    if info_path.is_file():
      relative_dir = str(info_path.parent).replace(base_directory,'${PLATFORM_DIR}')
    include_dir_set.add(relative_dir)
  
  line_list = []
  line_list.append('INCLUDES += -I${RVX_SSW_HOME}/memorymap')
  for include_dir in include_dir_set:
    line_list.append('INCLUDES += -I{0}'.format(include_dir))
    line_list.append('C_SRC += $(wildcard {0}/*.c)'.format(include_dir))

  num_main_core = 0
  for define_name in define_dict.keys():
    if define_name.startswith('INCLUDE_RVC_'):
      assert num_main_core <= 2
      num_main_core += 1
      core_name = define_name[len('INCLUDE_RVC_'):].lower()
      line_list.append('-include ${{RVX_SSW_HOME}}/core/{0}/set_env.mh'.format(core_name))

  basic_library_list = ('system_utility', 'utility', 'uthash', 'image', 'matrix' , 'mmiox',
                          'api/munoc', 'api/platform_controller', 'api/core_peri_group', 'api/external_peri_group',
                         )
  for library in basic_library_list:
    line_list.append(f'-include ${{RVX_SSW_HOME}}/{library}/env/set_env.mh')

  hw_library_list = ('timer', 'plic', 'oled_bw', 'oled_rgb', 'lcd_shield', 'tcaching', 'fuse_box', 'arducam',
                    'bluetooth', 'wifi', 'i2s', 'dma', 'florian', 'adc',
                    'jpeg_encoder', 'jpegls_encoder', 'hbc1_tx', 'hbc1_rx', 'pact', 'sdram', 'edge_video_system', 'starc','dca','c2c','mmiox','extinput_backend')
  for hw in hw_library_list:
    if f'INCLUDE_{hw.upper()}' in define_dict:
      line_list.append(f'-include ${{RVX_SSW_HOME}}/api/{hw}/env/set_env.mh')
  
  if 'INCLUDE_TRAFFIC_GENERATOR' in define_dict and 'INCLUDE_DMA' not in define_dict:
      line_list.append(f'-include ${{RVX_SSW_HOME}}/api/dma/env/set_env.mh')

  set_env_file = output_dir / 'set_env.mh'
  set_env_file.write_text('\n'.join(line_list))
