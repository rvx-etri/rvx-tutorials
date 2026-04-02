import argparse
import re
import os

from pathlib import Path

from rvx_util import *
from xml_util import *
from configure_template import *

def is_defined_as_true(define_xml):
  result = False
  value_xml = define_xml.find('value')
  if value_xml.attrib['type']=='bool':
    if value_xml.text=='True':
      result = True
  return result

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Generating PACT componet')
  parser.add_argument('-xml', help='pact info XML file')
  parser.add_argument('-op', help='operation')
  parser.add_argument('-template', '-t', help='template dir')  
  parser.add_argument('-output', '-o', help='output directory')

  args = parser.parse_args()
  assert args.xml

  root_xml_file = Path(args.xml)
  assert root_xml_file.is_file(), root_xml_file
  template_dir = Path(args.template)
  assert template_dir.is_dir(), template_dir
  assert args.output
  output_dir = Path(args.output)

  #
  root_xml = read_xml_file(root_xml_file, 'rvx')

  if args.op=='user_node':
    # pact_template_node.v
    template_node_file = template_dir / 'pact_template_node.v.template'
    assert template_node_file.is_file(), template_node_file
    template_node_contents = template_node_file.read_text()

    for dec_ip in root_xml.findall('dec_ip'):
      replace_list = []
      for parameter in dec_ip.findall('parameter'):
        id = parameter.find('id').text
        value = parameter.find('value').text
        replace_list.append((id,value))
      user_node_contents = configure_template_text(template_node_contents, replace_list)
      user_node_name = dec_ip.find('name').text
      output_file = output_dir / 'src' / f'{user_node_name.lower()}.v'
      output_file.write_text(user_node_contents)

    # pact_extension.vh
    extention_template_file = template_dir / 'pact_extension.vh.template'
    assert extention_template_file.is_file(), extention_template_file
    extention_template_contents = extention_template_file.read_text()

    full_extention_contents = ''
    for dec_ip in root_xml.findall('dec_ip'):
      replace_list = []
      for parameter in dec_ip.findall('parameter'):
        id = parameter.find('id').text
        value = parameter.find('value').text
        replace_list.append((id,value))
      user_node_name = dec_ip.find('name').text
      instance_name = f'i_{user_node_name.lower()}'
      replace_list.append(('INSTANCE_NAME',instance_name))
      each_extention_contents = configure_template_text(extention_template_contents, replace_list)
      full_extention_contents += '\n'
      full_extention_contents += each_extention_contents
    
    output_file = output_dir / 'include' / 'pact_extension.vh'
    output_file.write_text(full_extention_contents)
  elif args.op=='core':
    template_core_file = template_dir / 'pact_core_node.v'
    assert template_core_file.is_file(), template_core_file
    template_core_contents = template_core_file.read_text()

    for info_xml in root_xml.findall('info'):
      info_name = info_xml.find('name').text
      if info_name!='pact_arch_config':
        continue
      for define_xml in info_xml.findall('define'):
        define_name = define_xml.find('name').text
        if not define_name.startswith('PACT_INCLUDE_NODE_CORE'):
          continue
        if not is_defined_as_true(define_xml):
          continue
        core_name = define_name[len('PACT_INCLUDE_NODE_'):]
        output_file = output_dir / 'src' / f'pact_{core_name.lower()}_node.v'
        modified_contents = template_core_contents.replace('PACT_CORE_NODE',f'PACT_{core_name.upper()}_NODE')
        output_file.write_text(modified_contents)
  elif args.op=='lsu':
    template_dma_file = template_dir / 'pact_lsu_node_with_dma.v'
    assert template_dma_file.is_file(), template_dma_file
    template_dma_contents = template_dma_file.read_text()

    template_cache_file = template_dir / 'pact_lsu_node_with_cache.v'
    assert template_cache_file.is_file(), template_cache_file
    template_cache_contents = template_cache_file.read_text()

    template_spm_file = template_dir / 'pact_lsu_node_with_spm.v'
    assert template_spm_file.is_file(), template_spm_file
    template_spm_contents = template_spm_file.read_text()

    for info_xml in root_xml.findall('info'):
      info_name = info_xml.find('name').text
      if info_name!='pact_arch_config':
        continue
      for define_xml in info_xml.findall('define'):
        define_name = define_xml.find('name').text
        if not define_name.startswith('PACT_INCLUDE_LSU'):
          continue
        if not is_defined_as_true(define_xml):
          continue
        define_name_back = define_name[len('PACT_INCLUDE_'):]
        is_dma = define_name_back.endswith('_DMA')
        is_cache = define_name_back.endswith('_CACHE')
        is_spm = define_name_back.endswith('_SPM')
        if is_dma:
          lsu_name = define_name_back[:-len('_DMA')]
        elif is_cache:
          lsu_name = define_name_back[:-len('_CACHE')]
        elif is_spm:
          lsu_name = define_name_back[:-len('_SPM')]
        else:
          continue
        output_file = output_dir / 'src' / f'pact_{lsu_name.lower()}_node.v'
        if is_dma:
          modified_contents = template_dma_contents.replace('PACT_LSU_NODE',f'PACT_{lsu_name.upper()}_NODE')
        elif is_cache:
          modified_contents = template_cache_contents.replace('PACT_LSU_NODE',f'PACT_{lsu_name.upper()}_NODE')
        elif is_spm:
          modified_contents = template_spm_contents.replace('PACT_LSU_NODE',f'PACT_{lsu_name.upper()}_NODE')
        else:
          assert 0
        output_file.write_text(modified_contents)
  else:
    assert 0, args.op
