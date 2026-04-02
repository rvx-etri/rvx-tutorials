import argparse
import re
import os
from pathlib import Path

from xml.etree.ElementTree import *
from generate_copyright import *

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Generating Implementation Dependent Modules')
  parser.add_argument('-xml', help='an input xml file')
  parser.add_argument('-imp_type', '-it', help='implementation type')
  parser.add_argument('-target_imp_class', '-tic', help='target implementation class')
  parser.add_argument('-template', '-t', help='template dir')
  parser.add_argument('-output', '-o', help='output directory')

  args = parser.parse_args()
  assert args.xml
  assert Path(args.xml).is_file()
  assert args.target_imp_class
  assert args.template
  assert Path(args.template).is_dir()
  assert args.output

  emtpy_body_file  = Path(args.template) / 'empty_body.vh'
  assert emtpy_body_file.is_file()

  assert args.imp_type in ('rtl','fpga','soc'), imp_type

  xml_tree = parse(args.xml)
  xml_root = xml_tree.getroot()
  assert len(xml_root)>=1, 'No available info'
  assert xml_root.tag=='rvx', xml_root.tag
  module_list = xml_root.findall('dec_ip')
  assert module_list  

  for dec_ip in module_list:
    library_name = dec_ip.find('library_name').text
    ip_template_directory = Path(args.template) / library_name
    if not ip_template_directory.is_dir():
      continue
    line_list = []
    line_list.append(gen_copyright_in_verilog())
    line_list.append('')

    # timescale
    if args.imp_type=='rtl':
      line_list.append('`include \"timescale.vh\"')
    # prolog
    prolog_file = ip_template_directory / f'{library_name}.prolog.vh'
    line_list.append(prolog_file.read_text())
    # body
    body_file = ip_template_directory / f'{library_name}.body.{args.target_imp_class}.vh'
    if not body_file.is_file():
      body_file = ip_template_directory / f'{library_name}.body.{args.imp_type}.vh'
    if not body_file.is_file():
      body_file = emtpy_body_file
    line_list.append(body_file.read_text())
    # epilog
    line_list.append('')
    epilog_file = ip_template_directory / f'{library_name}.epilog.vh'
    line_list.append(epilog_file.read_text())

    # file_contents
    file_contents = '\n'.join(line_list)
    
    # replace list
    replace_list = []
    replace_list.append(('MODULE_NAME', dec_ip.find('name').text.upper()))
    for parameter in dec_ip.findall('parameter'):
      id = parameter.find('id').text
      value_xml = parameter.find('value')
      value = value_xml.text
      if value_xml.attrib['type']=='hex':
        value = str(int(value,16))
      replace_list.append((id,value))

    # replace
    for before_word, after_word in replace_list:
      before_word = '\"\${' + before_word + '}\"'
      file_contents = re.sub(before_word,after_word, file_contents)

    # verilog generation
    output_src_dir = Path(args.output) / 'src'
    if not output_src_dir.is_dir():
      output_src_dir.mkdir(parents=True)
    output_file = output_src_dir / '{0}.v'.format(dec_ip.find('name').text)
    output_file.write_text(file_contents)
