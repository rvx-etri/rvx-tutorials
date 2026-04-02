import os
import argparse
import itertools
from pathlib import *
from xml.dom import minidom
import xml.etree.ElementTree as XmlTree

from rvx_util import *
from configure_template import *

def execute_shell_cmd(cmd, cwd=None):
  if cwd:
    os.chdir(cwd)
  result = os.system(cmd)
  success = True if result==0 else False
  return success

def get_hex_size_in_byte(hex32_list:list):
  size = len(hex32_list)
  if size:
    byte_per_line = len(hex32_list[0]) >> 1
    size = size * byte_per_line
  return size

def append_child_xml_element(xml_parent:XmlTree, tag:str, text:str, attrib:dict=None):
  xml_child = XmlTree.SubElement(xml_parent, tag)
  xml_child.text = text
  if attrib:
    xml_child.attrib = attrib
  return xml_child

def generate_value_xml_element(value, type):
  xml_element = XmlTree.Element('value')
  xml_element.text = str(value)
  xml_element.attrib['type'] = type
  return xml_element

def generate_define_xml_element(name, value, type):
  xml_element = XmlTree.Element('define')
  append_child_xml_element(xml_element,'name', name.upper())
  xml_element.append(generate_value_xml_element(value, type))
  return xml_element

def convert_rom(hex32_list:list, cell_width_in_byte:int):
  num_group = cell_width_in_byte >> 2
  hex_list = []
  for i in range(0,len(hex32_list),num_group):
    hex_list.append(''.join(reversed(hex32_list[i:i+num_group])))
  return hex_list;

def generate_rom_data_code(hex_list:list, rom_data_name:str, index_name:str):
  if len(hex_list):
    data_width = len(hex_list[0])*4
  line_list = []
  line_list.append('always@(*)')
  line_list.append('begin')
  line_list.append(f'\t{rom_data_name} = 0;')
  line_list.append(f'\tcase({index_name})')
  for i, line in enumerate(hex_list):
    line_list.append(f'\t\t{i}: {rom_data_name} = {data_width}\'h {line};')
  line_list.append('\tendcase')
  line_list.append('end')
  return '\n'.join(line_list)

if __name__ == '__main__':
  # parse argument
  parser = argparse.ArgumentParser(description='Synthesizing ROM components')
  parser.add_argument('-name', '-n', help='ROM name')
  parser.add_argument('-type', '-t', help='ROM type')
  parser.add_argument('-axi', '-a', help='AXI width')
  parser.add_argument('-cell', '-c', help='CELL width')
  parser.add_argument('-input', '-i', nargs='+', help='an input .so file')
  parser.add_argument('-output', '-o', help='output directory')
  args = parser.parse_args()

  assert args.name
  assert args.type in ('rtl','idip'), args.type
  assert args.axi
  assert args.cell
  assert args.input
  assert args.output

  axi_width = int(args.axi)
  assert axi_width in (32,64,128), axi_width
  
  cell_width = int(args.cell)
  assert cell_width in (32,64,128,256,512,1024), cell_width
  assert cell_width >= axi_width
  cell_width_in_byte = cell_width >> 3

  output_dir = Path(args.output)
  assert output_dir.is_dir(), output_dir

  input_path_list = []
  for input_pattern in args.input:
    input_path = Path(input_pattern)
    if input_path.is_dir():
      input_path_list += input_path.glob('*.h')
    elif input_path.is_file():
      assert str(input_path).endswith('.h'), input_pattern
      input_path_list.append(input_path)
    else:
      assert 0, input_pattern
      
  # generate hex
  for input_path in input_path_list:
    array_name = input_path.stem
    template_file = Path('.') / 'rom' / 'generate_hex.c.template'
    generate_file = output_dir / f'generate_hex_from_{array_name}.c'
    config_list = ( \
                   ('ARRAY_NAME', array_name), \
                   ('OUTPUT_FILE', f'{array_name}.hex')
                  )
    configure_template_file(template_file, generate_file, config_list)
    execute_shell_cmd(f'cd {output_dir} && make __gen_each_rom GENERATE_FILE_STEM={generate_file.stem} ARRAY_NAME={array_name}')

  # generate total hex
  total_hex32_list = []
  hex_info = []
  for input_path in input_path_list:
    array_name = input_path.stem
    hex_path = output_dir / f'{array_name}.hex'
    assert hex_path.is_file(), hex_path
    hex32_list = hex_path.read_text().replace('\n\n','\n').split('\n')
    original_size = get_hex_size_in_byte(hex32_list)
    num_spare = original_size % cell_width_in_byte
    if num_spare > 0:
      assert (num_spare%4)==0, original_size
      num_fill = (cell_width_in_byte - num_spare) >> 2
      for i in range(num_fill):
        hex32_list.append('00000000')
    modified_size = get_hex_size_in_byte(hex32_list)
    assert (modified_size % cell_width_in_byte)==0
    hex_info.append((array_name, original_size, modified_size))
    total_hex32_list += hex32_list

  total_hex_list = convert_rom(total_hex32_list, cell_width_in_byte)
  total_hex_path = output_dir / f'{args.name}.hex'
  total_hex_path.write_text('\n'.join(total_hex_list))

  # variable
  capacity = get_hex_size_in_byte(total_hex_list)
  capacity = (((capacity-1)>>12)+1)<<12
  cell_depth = int((capacity-1)/cell_width_in_byte)+1

  # generate axi ip
  template_file = Path('.') / 'rom' / 'rom_axi.v.template'
  rom_axi_path = output_dir / f'{args.name}_axi.v'
  config_list = ( \
                 ('MODULE_NAME', f'{args.name.upper()}_AXI'), \
                 ('BASEADDR', f'I_{args.name.upper()}_BASEADDR'), \
                 ('BW_DATA', str(axi_width)), \
                 ('CELL_SIZE', str(capacity)), \
                 ('CELL_WIDTH', str(cell_width)), \
                 ('CELL_NAME', f'{args.name.upper()}_CELL'), \
                )
  configure_template_file(template_file, rom_axi_path, config_list)

  # generate hex data for simulation
  rom_data_path = output_dir / f'{args.name}_data.vb'
  rom_data_path.write_text(generate_rom_data_code(total_hex_list, 'rom_data', 'rindex'))

  # generate cell ip for simulation or rtl
  template_file = Path('.') / 'rom' / 'rom_cell_rtl.v.template'
  if args.type=='rtl':
    rom_cell_path = output_dir / f'{args.name}_cell_rtl.v'
  else:
    rom_cell_path = output_dir / f'{args.name}_cell_sim.v'
  config_list = ( \
                 ('MODULE_NAME', f'{args.name.upper()}_CELL'), \
                 ('ROM_DATA_FILE', rom_data_path.name), \
                )
  configure_template_file(template_file, rom_cell_path, config_list)

  # generate coe for fpga
  # max is 65536 x 1024
  if args.type=='idip':
    coe_contents  = 'memory_initialization_radix = 16;'
    coe_contents += '\nmemory_initialization_vector ='
    for line in total_hex_list:
      coe_contents += f'\n{line}'
    coe_contents += ';'
    coe_path = output_dir / f'{args.name}_data.coe'
    coe_path.write_text(coe_contents)

    xilinx_module_name = f'xilinx_{args.name}'
    template_file = Path('.') / 'rom' / 'generate_xilinx_rom.tcl.template'
    script_path = output_dir / f'generate_{xilinx_module_name}.tcl'
    config_list = ( \
                   ('MODULE_NAME', xilinx_module_name), \
                   ('OUTPUT_DIR', str(output_dir)), \
                   ('CELL_DEPTH', str(cell_depth)), \
                   ('CELL_WIDTH', str(cell_width)), \
                   ('COE_FILE', f'../user/fpga/common/{coe_path.name}'), \
                  )
    configure_template_file(template_file, script_path, config_list)

  # generate cell ip for fpga
  if args.type=='idip':
    template_file = Path('.') / 'rom' / 'rom_cell_fpga.v.template'
    rom_cell_path = output_dir / f'{args.name}_cell_fpga.v'
    config_list = ( \
                   ('MODULE_NAME', f'{args.name.upper()}_CELL'), \
                   ('XILINX_MODULE_NAME', f'xilinx_{args.name}'), \
                  )
    configure_template_file(template_file, rom_cell_path, config_list)

  # generate memorymap
  xml_root = XmlTree.Element('rvx')
  xml_info = XmlTree.SubElement(xml_root, 'info')
  append_child_xml_element(xml_info, 'name', f'{args.name}_memorymap')
  append_child_xml_element(xml_info, 'include_header', 'platform_info')
  #
  xml_group = XmlTree.SubElement(xml_info, 'define_group')
  xml_group.append(generate_define_xml_element(f'{args.name}_capacity', hex(capacity), 'hex'))
  for image_name, original_size, modified_size in hex_info:
    size_of_name = f'size_of_{image_name}'.upper()
    xml_group.append(generate_define_xml_element(size_of_name, original_size,'dec'))

  accumulated_size = 0
  for image_name, original_size, modified_size in hex_info:
    offset_name = f'mmap_offset_{args.name}_{image_name}'.upper()
    xml_group.append(generate_define_xml_element(offset_name, hex(accumulated_size),'hex'))
    accumulated_size += modified_size
  
  for image_name, original_size, modified_size in hex_info:
    baseaddr_name = f'i_{args.name}_baseaddr'.upper()
    offset_name = f'mmap_offset_{args.name}_{image_name}'.upper()
    xml_group.append(generate_define_xml_element(f'mmap_{args.name}_{image_name}',f'({baseaddr_name} + {offset_name})','exp'))

  xml_str = XmlTree.tostring(xml_root, encoding="unicode")
  xml_str = minidom.parseString(xml_str).toprettyxml(indent="\t")
  xml_file = output_dir / f'{args.name}_memorymap.xml'
  xml_file.write_text(xml_str)

  execute_shell_cmd(f'make _convert_xml INPUT_XML={xml_file} LANGAUGE_TYPE=c OUTPUT_DIR={output_dir}')
  execute_shell_cmd(f'make _convert_xml INPUT_XML={xml_file} LANGAUGE_TYPE=verilog OUTPUT_DIR={output_dir}')
