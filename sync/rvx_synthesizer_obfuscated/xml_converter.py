import argparse
import os
import math
import shutil
import copy
import subprocess

from enum import Enum
from itertools import count
from xml.etree.ElementTree import *
from pathlib import Path

from verilog_generator import *
from c_generator import *
from tcl_generator import *
#import generate_copyright

from rvx_util import *

def convert_define(xml_element, generator):
  name = xml_element.find('name').text
  result = None
  for value in xml_element.findall('value'):
    format = value.get('type')
    result = generator.gen_define(name,value.text,format)
    if result!=None:
      break
  return result

def convert_comment(xml_element, generator):
  return generator.gen_short_comments(xml_element.text)

def convert_define_group(xml_element, generator):
  line_list = []
  for subelement in xml_element:
    if subelement.tag=='define':
      line_list.append(convert_define(subelement, generator))
    elif subelement.tag=='comment':
      line_list.append(convert_comment(subelement, generator))
  return line_list

if __name__ == '__main__':
  # parse argument
  parser = argparse.ArgumentParser(description='Convert xml to verilog, c, and tcl')
  parser.add_argument('-xml', help='an input xml file')
  parser.add_argument('-output', '-o', help='output path')
  parser.add_argument('-language', '-l', help='languages be converted')
  args = parser.parse_args()

  assert args.xml
  assert args.language

  if args.output:
    output_path = Path(args.output)
  else:
    output_path = Path('.')
  #print(args.xml)
  #print(args.language)

  genarator_info = []
  if 'v' in args.language:
    genarator_info.append(('.vh',VerilogGenerator))
  if 'c' in args.language:
    genarator_info.append(('.h',CGenerator))
  if 't' in args.language:
    genarator_info.append(('.tcl',TclGenerator))

  xml_tree = parse(args.xml)
  xml_root = xml_tree.getroot()
  assert len(xml_root)>=1, 'No available info'
  assert xml_root.tag=='rvx', xml_root.tag
  for info in xml_root:
    name = info.find('name').text
    for file_suffix, generator in genarator_info:
      file_name = name + file_suffix
      line_list = []
      line_list.append(generator.gen_header_prologue(file_name))
      line_list.append('')
      for include in info.findall('include_header'):
        line_list.append(generator.gen_include(include.text))
      line_list.append('')
      for element in info:
        if element.tag=='include_header':
          continue
        elif element.tag=='define':
          line_list.append(convert_define(element, generator))
        elif element.tag=='define_group':
          line_list.append('')
          line_list += convert_define_group(element, generator)
      line_list.append('')
      line_list.append(generator.gen_header_epilogue(file_name))
      
      if output_path.suffix:
        output_file = output_path
      else:
        output_file = output_path / file_name
        
      assert output_file.parent.is_dir(), output_file
      output_file.write_text(list2str(line_list,'\n','',''))
