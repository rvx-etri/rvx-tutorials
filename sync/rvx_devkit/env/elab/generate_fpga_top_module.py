import argparse
import re
import os
from pathlib import Path

from xml.etree.ElementTree import *

import import_util
from os_util import *
from xml_util import *
from generate_copyright import *

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Generating Top Modules for Target FPGA')
  parser.add_argument('-top_xml', '-top', help='an xml for top module')
  parser.add_argument('-tic_name', help='target implementation class name')
  parser.add_argument('-tic_xml', help='target implementation class xml')
  parser.add_argument('-output', '-o', help='output directory')

  args = parser.parse_args()
  assert Path(args.top_xml).is_file(), args.top_xml
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

  # fpga info
  if 'flash_sck_uses_startupe2' in imp_class_info['fpga_info']:
    flash_sck_uses_startupe2 = True
  else:
    flash_sck_uses_startupe2 = False
  if 'no_jtag_reset' in imp_class_info['fpga_info']:
    no_jtag_reset = True
  else:
    no_jtag_reset = False
  if 'ext_mram_levelshifter' in imp_class_info['fpga_info']:
    ext_mram_levelshifter = True
  else:
    ext_mram_levelshifter = False
    
  exclude_port_list = imp_class_info['fpga_info'].get('exclude_port')
  if not exclude_port_list:
    exclude_port_list = []
  elif type(exclude_port_list)==str:
    exclude_port_list = [exclude_port_list]
  if flash_sck_uses_startupe2:
    exclude_port_list.append('spi_flash_sclk')
  if no_jtag_reset:
    exclude_port_list.append('pjtag_rtrstnn')
  if not ext_mram_levelshifter:
    exclude_port_list.append('EXTMR_LS_OE_P')
    exclude_port_list.append('EXTMR_LS_CE_N')
  exclude_port_set = frozenset(exclude_port_list)

  #
  xml_tree = parse(args.top_xml)
  xml_root = xml_tree.getroot()
  assert len(xml_root)>=1, 'No available info'
  assert xml_root.tag=='rvx', xml_root.tag
  top_module = xml_root.find('dec_ip')
  assert top_module

  top_module_name = top_module.find('hdl_name').text.lower()
  fpga_top_module_name = f'{top_module_name}_fpga'
  #print(fpga_top_module_name)
  platform_signal_list = [ (x.find('name').text, x.find('width'), x.find('port').text) for x in top_module.findall('signal')]
  included_port_list = []
  for x in top_module.findall('included_port_dec'):
    included_port_name = x.text[:-len('_dec')]
    if included_port_name not in exclude_port_set:
      included_port_list.append(included_port_name)
  #print(included_port_list)
  
  # external_clk
  external_clk_valid_name_set = set()
  for i, value in enumerate(imp_class_info['fpga_info']['extenral_clk_mhz'].split(',')):
    external_clk_valid_name_set.add(f'external_clk_{i}')
    if value[-1]=='d':
      external_clk_valid_name_set.add(f'external_clk_{i}_pair')

  # elab
  top_port_list = []
  wire_list = []
  for name, width_xml, port in platform_signal_list:
    if name in exclude_port_set:
      wire_list.append((name, width_xml, port))
    elif name.startswith('external_clk') and (name not in external_clk_valid_name_set):
      wire_list.append((name, width_xml, port))
    else:
      top_port_list.append((name, width_xml, port))

  # copyright #
  line_list = []
  line_list.append(gen_copyright_in_verilog())
  line_list.append('')

  # include
  for include_header in top_module.findall('include_header'):
    line_list.append(f'`include \"{include_header.text}.vh\"')

  # module
  line_list.append('')
  line_list.append(f'module {fpga_top_module_name.upper()}')

  # port dec
  line_list.append('(')
  line_list.append(',\n'.join([ f'\t{name}' for name, width_xml, port in top_port_list ]))
  for included_port in included_port_list:
    line_list.append(f'\t`include \"{included_port}_dec.vh\"')
  line_list.append(');')

  # port def
  line_list.append('')
  for name, width_xml, port in top_port_list:
    if width_xml.attrib['type']=='dec' and width_xml.text=='1':
      line_list.append(f'{port} wire {name};')
    else:
      line_list.append(f'{port} wire [{width_xml.text}-1:0] {name};')
  line_list.append('')
  for included_port in included_port_list:
    line_list.append(f'`include \"{included_port}_def.vh\"')

  # signal not mapped to platform
  line_list.append('')
  for name, width_xml, port in wire_list:
    assignment = ' = 0' if port=='input' else ''
    if width_xml.attrib['type']=='dec' and width_xml.text=='1':
      line_list.append(f'wire {name}{assignment};')
    else:
      line_list.append(f'wire [{width_xml.text}-1:0] {name}{assignment};')
  
  # flash_sck_uses_startupe2
  if flash_sck_uses_startupe2:

    line_list.append('''
STARTUPE2
#(
  .PROG_USR("FALSE"),
  .SIM_CCLK_FREQ(0.0)
)
i_STARTUPE2
(
  .CFGCLK(),
  .CFGMCLK(),
  .EOS(),
  .PREQ(),

  .CLK(1'b0),
  .GSR(1'b0),
  .GTS(1'b0),
  .KEYCLEARB(1'b0),
  .PACK(1'b0),
  .USRCCLKO(spi_flash_sclk),
  .USRCCLKTS(1'b0),
  .USRDONEO(1'b1),
  .USRDONETS(1'b0)
);''')

  if no_jtag_reset:
    line_list.append('assign pjtag_rtrstnn = 1\'b 1;')
  
  # body
  line_list.append('')
  line_list.append(top_module_name.upper())
  line_list.append('i_platform')
  line_list.append('(')
  line_list.append(',\n'.join([ f'\t.{name}({name})' for name, width_xml, port in platform_signal_list ]))
  for included_port in included_port_list:
    line_list.append(f'\t`include \"{included_port}_mapping.vh\"')
  line_list.append(');')

  # epilog
  line_list.append('')
  line_list.append(f'endmodule')

  # file_contents
  file_contents = '\n'.join(line_list)

  # verilog generation
  if not output_dir.is_dir():
    output_dir.mkdir(parents=True)
  output_file = output_dir / f'{fpga_top_module_name}.v'
  output_file.write_text(file_contents)

