import argparse
import re
import os

from pathlib import Path

from rvx_util import *
from xml_util import *
from configure_template import *
from verilog_generator import *

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Generating RTL Simulation Environment')
  parser.add_argument('-top', help='top module declaration')
  parser.add_argument('-info', nargs='+', help='platform info file')
  parser.add_argument('-template', '-t', help='template dir')  
  parser.add_argument('-output', '-o', help='output directory')

  args = parser.parse_args()
  assert args.top
  assert args.info
  
  top_xml_file = Path(args.top)
  assert top_xml_file.is_file(), top_xml_file
  template_dir = Path(args.template)
  assert template_dir.is_dir(), template_dir
  assert args.output
  output_dir = Path(args.output)

  # top_xml
  root_xml = read_xml_file(top_xml_file, 'rvx')
  top_xml = root_xml.find('dec_ip')
  assert top_xml, root_xml
  top_module_signal_list = []
  included_port_list = []
  for signal in top_xml.findall('signal'):
    top_module_signal_list.append((signal.find('name').text, signal.find('width').text))
  for included_port_dec in top_xml.findall('included_port_dec'):
    included_port_list.append(included_port_dec.text[:-len('_port_dec')])
  has_user_region = (top_xml.find('has_user_region')!=None)
  
  #
  define_list = []
  for file_name in args.info:
    define_list += get_define_list('`define', file_name)

  define_dict = {}
  for id, value in define_list:
    define_dict[id] = value

  #print(define_dict)

  # external clk
  external_clk_list = []
  for i in range(0,10):
    clock_name_define = f'EXTERNAL_CLK_NAME_{i:02}'
    clock_name = define_dict.get(clock_name_define)
    if clock_name:
      if i==0:
        clock_speed = str(250000000)
      else:
        clock_speed_define = f'EXTERNAL_CLK_HZ_{i:02}'
        clock_speed = define_dict.get(clock_speed_define)
        assert clock_speed, clock_speed_define
      external_clk_list.append((clock_name,clock_speed))
    elif i>=1:
      break

  platform_name = define_dict['PLATFORM_NAME'][1:-1].upper()
  module_name = f'{platform_name}_SIM'

  # include
  line_list = []
  line_list.append((template_dir/'sim_include.vh').read_text())

  # module prolog
  line_list.append('')
  line_list.append(f'module {module_name}')
  line_list.append('();')
  
  # clock and reset
  line_list.append((template_dir/'sim_clock_prolog.vh').read_text())
  for clk_name, clock_speed in external_clk_list:
    sim_clock_body = (template_dir/'sim_clock_body.vh').read_text()
    clk_period = str(int(10**9 / int(clock_speed)))
    conv_info = []
    conv_info.append(('CLK_NAME', clk_name))
    conv_info.append(('CLK_PERIOD', clk_period))
    sim_clock_body = configure_template_text(sim_clock_body, conv_info)
    line_list.append(sim_clock_body)
  line_list.append((template_dir/'sim_clock_epilog.vh').read_text())

  # signal definition
  line_list.append('')
  for included_port in included_port_list:
    line_list.append(f'`include \"{included_port}_wire_def.vh\"')
  line_list.append((template_dir/'sim_signal.vh').read_text())

  # ncsim logics
  line_list.append((template_dir/'sim_ncsim.vh').read_text())

  # module instance
  port_list = []
  port_list += ['external_rstnn']
  port_list += ['pjtag_rtck','pjtag_rtrstnn','pjtag_rtms','pjtag_rtdi','pjtag_rtdo']
  for clk_name, clock_speed in external_clk_list:
    port_list.append(clk_name)
    if clk_name.startswith('external_clk_'):
      port_list.append(f'{clk_name}_pair')

  if 'INCLUDE_BOOT_MODE' in define_dict:
    port_list += ['boot_mode']

  if 'INCLUDE_UART_PRINTF' in define_dict:
    port_list += ['printf_tx','printf_rx']
  '''
  if 'INCLUDE_UART_USER' in define_dict:
      port_list += ['uart_tx_list','uart_rx_list']
  if 'INCLUDE_SPI_USER' in define_dict:
    port_list += ['spi_sclk_list','spi_scs_list','spi_sdq0_list',  'spi_sdq1_list']
  if 'INCLUDE_I2C_USER' in define_dict:
    port_list += ['i2c_sclk_list', 'i2c_sdata_list']
  if 'INCLUDE_GPIO_USER' in define_dict:
    port_list += ['gpio_list']
  '''
  
  instance_list = []
  instance_list.append(platform_name)
  instance_list.append('i_platform')
  instance_list.append('(')
  instance_list.append(list2str([f'.{port}({port})' for port in port_list],',\n','\t',''))
  for included_port in included_port_list:
    instance_list.append(f'\t`include \"{included_port}_port_mapping.vh\"')
  instance_list.append(');')
  line_list += instance_list

  # jtag
  line_list.append('')
  line_list.append((template_dir/'sim_jtag_prolog.vh').read_text())
  if 'INCLUDE_SMALL_RAM' in define_dict:
    for i in range(0, int(define_dict['NUM_SRAM_CELL'])):
      sram_text = (template_dir/'sim_jtag_sram_def.vh').read_text()
      sram_text = sram_text.replace('\"${CELL_INDEX}\"', str(i))
      line_list.append(sram_text)
    line_list.append('          case(cell_index)')
    for i in range(0, int(define_dict['NUM_SRAM_CELL'])):
      sram_text = (template_dir/'sim_jtag_sram_assign.vh').read_text()
      sram_text = sram_text.replace('\"${CELL_INDEX}\"', str(i))
      line_list.append(sram_text)
  line_list.append((template_dir/'sim_jtag_epilog.vh').read_text())

  # sim_user_region
  if has_user_region:
    line_list.append(f'`include \"sim_user_region.vh\"')

  # module epilog
  line_list.append('')
  line_list.append('endmodule')

  # file write
  if not output_dir.is_dir():
    output_dir.mkdir(parents=True)
  output_file = output_dir / f'{module_name.lower()}.v'
  output_file.write_text(list2str(line_list,'\n','',''))
