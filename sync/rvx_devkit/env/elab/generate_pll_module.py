import argparse
import re
import os
from pathlib import Path

from xml.etree.ElementTree import *

import import_util
from os_util import *
from xml_util import *
from generate_copyright import *

def get_ip_name(dec_ip):
  return dec_ip.find('name').text

def get_library_name(dec_ip):
  return dec_ip.find('library_name').text

def get_xci_module_name(dec_ip):
  return f'xilinx_{get_library_name(dec_ip)}'

def get_imp_type(imp_class_info):
  return imp_class_info['type']

def speed2name(clk_speed: int):
  return f'clk_{clk_speed}'

def get_clock_info(formatted_clk_name:str, clk_value:str):
  clk_diff = clk_value[-1]=='d'
  clk_speed = float(clk_value[:-1]) if clk_value[-1]=='d' else float(clk_value)
  if formatted_clk_name.endswith('_hz'):
    clk_name = formatted_clk_name[:-3]
  elif formatted_clk_name.endswith('_khz'):
    clk_name = formatted_clk_name[:-4]
    clk_speed = clk_speed * 1000
  elif formatted_clk_name.endswith('_mhz'):
    clk_name = formatted_clk_name[:-4]
    clk_speed = clk_speed * 1000000
  else:
    assert 0, formatted_clk_name
  return (clk_name, clk_speed, clk_diff)

class ElaboratedClockInfo():
  def __init__(self, dec_ip, imp_class_info):
    self.imp_class_info = imp_class_info
    library_name = get_library_name(dec_ip)
    self.pll_index = int(library_name[-1])    
    self.external_clk_speed = None
    self.external_clk_diff = None

    #
    imp_class_clock_info = {}
    if get_imp_type(self.imp_class_info)=='fpga':
      fpga_info = self.imp_class_info['fpga_info']
      for key, value in fpga_info.items():
        if key.startswith('clk_'):
          clk_name, clk_speed, _ = get_clock_info(key,value)
          imp_class_clock_info[clk_name] = int(clk_speed)
      _ , self.external_clk_speed, self.external_clk_diff = get_clock_info('extenral_clk_mhz',fpga_info['extenral_clk_mhz'].split(',')[self.pll_index])

    clock_info_xml = dec_ip.find('clock_info')
    assert clock_info_xml
    self.clock_dict = {}    
    for clock_define_xml in clock_info_xml.findall('define'):
      name = clock_define_xml.find('name').text
      value = clock_define_xml.find('value')
      value_type = value.attrib['type']
      if value_type=='dec':
        clk_speed = int(clock_define_xml.find('value').text)
      elif value_type=='text':
        if value.text=='imp_dependent':
          clk_speed = imp_class_clock_info.get(name)
        else:
          assert 0, value.text
      else:
        assert 0, value_type
      self.clock_dict[name] = clk_speed
    
    #print(self.clock_dict)
    
    #
    self.pll_clock_speed_set = set()
    self.div_clock_speed_set = set()
    self.invalid_list = []
    self.direct_list = []
    self.gen_from_pll_list = []
    self.gen_from_div_list = []
    
    if 'uses_pll_manually' in self.imp_class_info:
      self.requires_pll_generation = False
    else:
      self.requires_pll_generation = True
    
    for clk_name, clk_speed in self.clock_dict.items():
      if not clk_speed:
        self.invalid_list.append(clk_name)
      elif (not self.external_clk_diff) and (clk_speed==self.external_clk_speed):
        self.direct_list.append(clk_name)
      else:
        if get_imp_type(self.imp_class_info)=='fpga':
          upscale_clk_speed = clk_speed
          bw_counter = 1
          while upscale_clk_speed < 10000000:
            upscale_clk_speed <<= 1
            bw_counter += 1
          if bw_counter > 1:
            self.gen_from_div_list.append((clk_name, clk_speed))
            self.div_clock_speed_set.add((clk_speed, upscale_clk_speed, bw_counter))
            self.pll_clock_speed_set.add(upscale_clk_speed)
          else:
            self.gen_from_pll_list.append((clk_name, clk_speed))
            self.pll_clock_speed_set.add(clk_speed)
        else:
          self.gen_from_pll_list.append((clk_name, clk_speed))
          self.pll_clock_speed_set.add(clk_speed)
    self.requires_real_pll = (len(self.pll_clock_speed_set) > 0)

def generate_rtl_module(dec_ip, imp_class_info:dict, elaborated_clock_info):
  imp_type = get_imp_type(imp_class_info)
  clock_info = dec_ip.find('clock_info')
  assert clock_info
  clock_list = []
  for each_clock_info in clock_info.findall('define'):
    clock_list.append((each_clock_info.find('name').text, each_clock_info.find('value').text))

  line_list = []
  line_list.append(f'module {get_ip_name(dec_ip).upper()}')
  line_list.append('(')
  port_list = '\texternal_clk'
  port_list += ',\n\texternal_clk_pair'
  port_list += ',\n\texternal_rstnn'
  for clk_name, clk_value in clock_list:
    port_list += f',\n\t{clk_name}'
  line_list.append(port_list)
  line_list.append(');')
  line_list.append('')
  line_list.append('input wire external_clk;')
  line_list.append('input wire external_clk_pair;')
  line_list.append('input wire external_rstnn;')
  for clk_name, clk_value in clock_list:
    line_list.append(f'output wire {clk_name};')
  line_list.append('')

  if elaborated_clock_info.requires_real_pll:
    for clk_speed in elaborated_clock_info.pll_clock_speed_set:
      line_list.append(f'wire {speed2name(clk_speed)};')
    line_list.append('')

  for clk_name in elaborated_clock_info.direct_list:
    line_list.append(f'assign {clk_name} = external_clk;')

  for clk_name in elaborated_clock_info.invalid_list:
    line_list.append(f'assign {clk_name} = 0;')

  if elaborated_clock_info.requires_real_pll:
    if imp_type=='rtl':
      for i, clk_speed in enumerate(elaborated_clock_info.pll_clock_speed_set):
        clk_name = speed2name(clk_speed)
        clk_period_ns = int(1000000000 / clk_speed)
        line_list.append('')
        line_list.append('CLOCK_GENERATOR')
        line_list.append('#(')
        line_list.append(f'\t.CLK_INITIAL_DELAY(1),')
        line_list.append(f'\t.CLK_PERIOD({clk_period_ns})')
        line_list.append(')')
        line_list.append(f'i_gen_clk_{i}')        
        line_list.append('(')
        line_list.append(f'\t.clk({clk_name}),')
        line_list.append('\t.rstnn(),')
        line_list.append('\t.rst()')
        line_list.append(');')
        line_list.append('')
    elif imp_type=='fpga':
      line_list.append('')
      xci_module_name = get_xci_module_name(dec_ip)
      line_list.append(xci_module_name)
      line_list.append(f'i_{xci_module_name}')
      line_list.append('(')
      if elaborated_clock_info.external_clk_diff:
        line_list.append('\t.clk_in1_p(external_clk),')
        line_list.append('\t.clk_in1_n(external_clk_pair),')
      else:
        line_list.append('\t.clk_in1(external_clk),')
      line_list.append(',\n'.join([f'\t.{clk_name}({clk_name})' for clk_name in [speed2name(clk_speed) for clk_speed in elaborated_clock_info.pll_clock_speed_set]]))
      line_list.append(');')
    else:
      assert 0, imp_type
    #
    line_list.append('')
    for clk_name, clk_speed in elaborated_clock_info.gen_from_pll_list:
      line_list.append(f'assign {clk_name} = {speed2name(clk_speed)};')

    #
    for clk_speed, upscale_clk_speed, bw_counter in elaborated_clock_info.div_clock_speed_set:
      line_list.append('')
      counter_value_name = f'i_gen_{clk_speed}_value'
      line_list.append(f'wire [{bw_counter}-1:0] {counter_value_name};')
      line_list.append(f'''ERVP_COUNTER
#(
  .BW_COUNTER({bw_counter}),
  .CIRCULAR(1)
)
i_gen_{clk_speed}
(
  .clk({speed2name(upscale_clk_speed)}),
	.rstnn(external_rstnn),
	.enable(1'b 1),
	.init(1'b 0),
	.count(1'b 1),
	.value({counter_value_name}),
	.is_first_count(),
	.is_last_count()
);''')
      line_list.append(f'assign {speed2name(clk_speed)} = {counter_value_name}[{bw_counter}-1];')
    
    line_list.append('')
    for clk_name, clk_speed in elaborated_clock_info.gen_from_div_list:
      line_list.append(f'assign {clk_name} = {speed2name(clk_speed)};')

  line_list.append('')
  line_list.append('endmodule')

  return line_list

def generate_tcl_script(dec_ip, imp_class_info:dict, elaborated_clock_info):
  xci_module_name = get_xci_module_name(dec_ip)

  def make_mhz(clk_speed: int):
    return round(clk_speed/1000000, 3)

  tcl_list = []
  xci_dir = f'./xci/{xci_module_name}'
  xci_file = f'{xci_dir}/{xci_module_name}.xci'
  stub_file = f'{xci_dir}/{xci_module_name}_stub.v'
  #
  tcl_list.append(f'if {{[file exist {xci_file}]==0}} {{')
  tcl_list.append(f'\tcreate_ip -force -name clk_wiz -vendor xilinx.com -library ip -version 6.0 -module_name {xci_module_name} -dir ./xci')
  property_list = []
  property_list.append(('PRIMARY_PORT', 'clk_in1'))
  property_list.append(('PRIM_IN_FREQ', make_mhz(elaborated_clock_info.external_clk_speed)))
  if elaborated_clock_info.external_clk_diff:
    property_list.append(('PRIM_SOURCE', 'Differential_clock_capable_pin'))
    property_list.append(('CLK_IN1_BOARD_INTERFACE', 'sys_diff_clock'))
    property_list.append(('USE_LOCKED', 'false'))
    property_list.append(('USE_RESET', 'false'))
  for index, clk_speed in enumerate(elaborated_clock_info.pll_clock_speed_set):
    property_list.append((f'CLKOUT{index+1}_USED', 'true'))
    property_list.append((f'CLK_OUT{index+1}_PORT', speed2name(clk_speed)))
    property_list.append((f'CLKOUT{index+1}_REQUESTED_OUT_FREQ', make_mhz(clk_speed)))
  tcl_list.append('\n'.join([f'\tset_property CONFIG.{property_name} {property_value} [get_ips {xci_module_name}]' for property_name, property_value in property_list]))

  tcl_list.append('} else {')
  tcl_list.append(f'\tread_ip {xci_file}')
  tcl_list.append('}')
  tcl_list.append(f'if {{[file exist {stub_file}]==0}} {{')
  tcl_list.append(f'\tsynth_ip -quiet {xci_file}')
  tcl_list.append('} else {')
  tcl_list.append(f'\tread_verilog {stub_file}')
  tcl_list.append('}')

  return tcl_list

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Generating Implementation Dependent Modules')
  parser.add_argument('-module', '-m', help='a module xml file')
  parser.add_argument('-tic_name', help='target implementation class name')
  parser.add_argument('-tic_xml', help='target implementation class xml')
  parser.add_argument('-output', '-o', help='output directory')

  args = parser.parse_args()
  assert args.module
  assert Path(args.module).is_file()
  assert args.tic_name
  assert args.tic_xml
  assert Path(args.tic_xml).is_file()
  assert args.output
  output_dir = Path(args.output)
  assert output_dir.is_dir(), output_dir

  # tic_xml
  tic_xml_tree = parse(args.tic_xml)
  tic_xml_root = tic_xml_tree.getroot()
  assert tic_xml_root.tag=='rvx', tic_xml_root.tag
  assert len(tic_xml_root)==1, len(tic_xml_root)
  assert tic_xml_root[0].tag=='imp_class_list', tic_xml_root[0].tag
  imp_class_info = None
  for imp_class_xml in tic_xml_root[0].findall('imp_class'):
    imp_class_info_candidate = xml2dict(imp_class_xml)
    if imp_class_info_candidate['name']==args.tic_name:
      assert not imp_class_info, imp_class_info
      imp_class_info = imp_class_info_candidate
      break

  assert imp_class_info, tic_xml_root

  #
  xml_tree = parse(args.module)
  xml_root = xml_tree.getroot()
  assert len(xml_root)>=1, 'No available info'
  assert xml_root.tag=='rvx', xml_root.tag
  module_list = xml_root.findall('dec_ip')
  assert module_list

  for dec_ip in module_list:
    library_name = get_library_name(dec_ip)
    if not library_name.startswith('clock_pll_'):
      continue

    elaborated_clock_info = ElaboratedClockInfo(dec_ip, imp_class_info)
    
    if elaborated_clock_info.requires_pll_generation:
    
      ## RTL ##
      line_list = []
      line_list.append(gen_copyright_in_verilog())
      line_list.append('')    
      line_list += generate_rtl_module(dec_ip, imp_class_info, elaborated_clock_info)

      # file_contents
      file_contents = '\n'.join(line_list)

      # verilog generation
      output_src_dir = output_dir / 'src'
      if not output_src_dir.is_dir():
        output_src_dir.mkdir(parents=True)
      output_file = output_src_dir / '{0}.v'.format(get_ip_name(dec_ip))
      output_file.write_text(file_contents)

      ## TCL script ##

      if get_imp_type(imp_class_info)=='fpga':
        if elaborated_clock_info.requires_real_pll:
          line_list = []
          line_list.append(gen_copyright_in_tcl())
          line_list.append('')    
          line_list += generate_tcl_script(dec_ip, imp_class_info, elaborated_clock_info)

          # file_contents
          file_contents = '\n'.join(line_list)

          # tcl generation
          output_tcl_dir = output_dir / 'xci'
          if not output_tcl_dir.is_dir():
            output_tcl_dir.mkdir(parents=True)
          output_file = output_tcl_dir / f'generate_{get_library_name(dec_ip)}.tcl'
          output_file.write_text(file_contents)
