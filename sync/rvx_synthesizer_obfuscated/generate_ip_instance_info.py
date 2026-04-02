import os
import argparse
import itertools
from pathlib import *
from xml.dom import minidom
import xml.etree.ElementTree as XmlTree

def get_library_core_name(library_name:str):
  hier_name_list = library_name.split('_')
  library_core_name_list = []
  for hier_name in hier_name_list:
    if hier_name.startswith('mmiox'):
      break
    else:
      library_core_name_list.append(hier_name)
  return '_'.join(library_core_name_list)

def generate_mmiox1(xml_root):
  header_include_list =[]
  header_line_list =[]
  body_include_list = []
  body_dec_list = []
  body_construct_list = []
  body_destruct_list = []
  body_line_list = []
  
  for interface_xml in xml_root.findall('interface'):
    interface_name = interface_xml.find('name').text
    library_name = interface_xml.find('library_name').text
    if library_name.startswith('mmiox1'):
      header_include_list.append('ervp_mmiox1.h')
      info_struct_name = 'ervp_mmiox1_hwinfo_t'
      info_variable_name = f'{interface_name}_info'
      info_static_variable_name = f'{info_variable_name}_static'
      para_struct_name = 'ervp_mmiox1_hwpara_t'
      para_variable_name = f'{info_variable_name}_para'
      busy_function_name = f'{info_variable_name}_is_busy'
      #
      header_line_list.append(f'//{info_variable_name}')
      header_line_list.append(f'extern const {info_struct_name}* const {info_variable_name};')
      #
      body_dec_list.append(f'//{info_variable_name}')
      body_dec_list.append(f'static {info_struct_name} {info_static_variable_name} CACHED_DATA;')
      body_dec_list.append(f'const {info_struct_name}* const {info_variable_name} = &{info_static_variable_name};')
      body_dec_list.append(f'static int {busy_function_name}(int task_id) {{')
      body_dec_list.append(f'\treturn mmiox1_inst_is_busy({info_variable_name});')
      body_dec_list.append(f'}}')
      #
      body_construct_list.append(f'\t//{info_variable_name}')
      body_construct_list.append(f'\t{para_struct_name} {para_variable_name};')

      for parameter_xml in interface_xml.findall('parameter'):
        parameter_name = parameter_xml.find('id').text
        parameter_value = parameter_xml.find('value').text
        body_construct_list.append(f'\t{para_variable_name}.{parameter_name} = {parameter_value};')
      body_construct_list.append(f'\tmmiox1_hwinfo_elaborate(&{para_variable_name}, &{info_static_variable_name});')
      body_construct_list.append(f'\t{info_static_variable_name}.baseaddr = {interface_name.upper()}_BASEADDR;')
      body_construct_list.append(f'\t{info_static_variable_name}.busy_fx = {busy_function_name};')
      
  return header_include_list, header_line_list, body_include_list, body_dec_list, body_construct_list, body_destruct_list, body_line_list

def generate_extinput(xml_root):
  header_include_list =[]
  header_line_list =[]
  body_include_list = []
  body_dec_list = []
  body_construct_list = []
  body_destruct_list = []
  body_line_list = []
  
  for ip_instance_xml in xml_root.findall('ip_instance'):
    ip_instance_name = ip_instance_xml.find('name').text
    library_name = ip_instance_xml.find('library_name').text
    if library_name.startswith('extinput_backend'):
      header_include_list.append('ervp_extinput_backend.h')
      info_struct_name = 'ervp_extinput_hwinfo_t'
      info_variable_name = f'{ip_instance_name}_info'
      for interface_xml in ip_instance_xml.findall('interface'):
        library_name = interface_xml.find('library_name').text
        if library_name.startswith('mmiox1'):
          mmiox_name = interface_xml.find('name').text
      mmiox_info_name = f'{ip_instance_name}_{mmiox_name}_info'
      assert mmiox_name
      #
      header_line_list.append(f'//{info_variable_name}')
      header_line_list.append(f'extern const {info_struct_name}* const {info_variable_name};')
      #
      body_dec_list.append(f'//{info_variable_name}')
      body_dec_list.append(f'const {info_struct_name}* const {info_variable_name} = {mmiox_info_name};')
      #
      body_construct_list.append(f'\t//{info_variable_name}')
      
  return header_include_list, header_line_list, body_include_list, body_dec_list, body_construct_list, body_destruct_list, body_line_list

def generate_dca_mru(xml_root):
  header_include_list =[]
  header_line_list =[]
  body_include_list = []
  body_dec_list = []
  body_construct_list = []
  body_destruct_list = []
  body_line_list = []
  
  for ip_instance_xml in xml_root.findall('ip_instance'):
    ip_instance_name = ip_instance_xml.find('name').text
    library_name = ip_instance_xml.find('library_name').text
    if library_name.startswith('dca_mru') and 'mmiox' in library_name:
      library_core_name = get_library_core_name(library_name)
      header_include_list.append(f'{library_core_name}.h')
      info_struct_name = f'{library_core_name}_hwinfo_t'
      info_variable_name = f'{ip_instance_name}_info'
      info_static_variable_name = f'{info_variable_name}_static'
      para_struct_name = f'{library_core_name}_hwpara_t'
      para_variable_name = f'{info_variable_name}_para'
      for interface_xml in ip_instance_xml.findall('interface'):
        library_name = interface_xml.find('library_name').text
        if library_name.startswith('mmiox1'):
          mmiox_name = interface_xml.find('name').text
      mmiox_info_name = f'{ip_instance_name}_{mmiox_name}_info'
      assert mmiox_name
      #
      header_line_list.append(f'//{info_variable_name}')
      header_line_list.append(f'extern const {info_struct_name}* const {info_variable_name};')
      #
      body_dec_list.append(f'//{info_variable_name}')
      body_dec_list.append(f'static {info_struct_name} {info_static_variable_name} CACHED_DATA;')
      body_dec_list.append(f'const {info_struct_name}* const {info_variable_name} = &{info_static_variable_name};')
      #body_dec_list.append(f'const {info_struct_name}* const {info_variable_name} = {mmiox_info_name};')
      #
      body_construct_list.append(f'\t//{info_variable_name}')
      body_construct_list.append(f'\t{para_struct_name} {para_variable_name};')
      for parameter_xml in ip_instance_xml.findall('parameter'):
        parameter_name = parameter_xml.find('id').text.lower()
        parameter_value = parameter_xml.find('value').text
        body_construct_list.append(f'\t{para_variable_name}.{parameter_name} = {parameter_value};')
      body_construct_list.append(f'\t{library_core_name}_hwinfo_elaborate(&{para_variable_name},&{info_static_variable_name});')
      body_construct_list.append(f'\t{info_static_variable_name}.mmiox_info = {mmiox_info_name};')
      
  return header_include_list, header_line_list, body_include_list, body_dec_list, body_construct_list, body_destruct_list, body_line_list

def generate_dca_matrix(xml_root):
  header_include_list =[]
  header_line_list =[]
  body_include_list = []
  body_dec_list = []
  body_construct_list = []
  body_destruct_list = []
  body_line_list = []
  
  for ip_instance_xml in xml_root.findall('ip_instance'):
    ip_instance_name = ip_instance_xml.find('name').text
    library_name = ip_instance_xml.find('library_name').text
    if library_name.startswith('dca_matrix') and 'mmiox' in library_name:
      library_core_name = get_library_core_name(library_name)
      header_include_list.append(f'{library_core_name}.h')
      info_struct_name = f'{library_core_name}_hwinfo_t'
      info_variable_name = f'{ip_instance_name}_info'
      info_static_variable_name = f'{info_variable_name}_static'
      para_struct_name = f'{library_core_name}_hwpara_t'
      para_variable_name = f'{info_variable_name}_para'
      for interface_xml in ip_instance_xml.findall('interface'):
        library_name = interface_xml.find('library_name').text
        if library_name.startswith('mmiox1'):
          mmiox_name = interface_xml.find('name').text
      mmiox_info_name = f'{ip_instance_name}_{mmiox_name}_info'
      assert mmiox_name
      #
      header_line_list.append(f'//{info_variable_name}')
      header_line_list.append(f'extern const {info_struct_name}* const {info_variable_name};')
      #
      body_dec_list.append(f'//{info_variable_name}')
      body_dec_list.append(f'static {info_struct_name} {info_static_variable_name} CACHED_DATA;')
      body_dec_list.append(f'const {info_struct_name}* const {info_variable_name} = &{info_static_variable_name};')
      #body_dec_list.append(f'const {info_struct_name}* const {info_variable_name} = {mmiox_info_name};')
      #
      body_construct_list.append(f'\t//{info_variable_name}')
      body_construct_list.append(f'\t{para_struct_name} {para_variable_name};')
      for parameter_xml in ip_instance_xml.findall('parameter'):
        parameter_name = parameter_xml.find('id').text.lower()
        parameter_value = parameter_xml.find('value').text
        body_construct_list.append(f'\t{para_variable_name}.{parameter_name} = {parameter_value};')
      body_construct_list.append(f'\t{library_core_name}_hwinfo_elaborate(&{para_variable_name},&{info_static_variable_name});')
      body_construct_list.append(f'\t{info_static_variable_name}.mmiox_info = {mmiox_info_name};')
      
  return header_include_list, header_line_list, body_include_list, body_dec_list, body_construct_list, body_destruct_list, body_line_list

def generate_dca_neugemm(xml_root):
  header_include_list =[]
  header_line_list =[]
  body_include_list = []
  body_dec_list = []
  body_construct_list = []
  body_destruct_list = []
  body_line_list = []
  
  for ip_instance_xml in xml_root.findall('ip_instance'):
    ip_instance_name = ip_instance_xml.find('name').text
    library_name = ip_instance_xml.find('library_name').text
    if library_name.startswith('dca_neugemm') and 'mmiox' in library_name:
      library_core_name = get_library_core_name(library_name)
      header_include_list.append(f'{library_core_name}.h')
      info_struct_name = f'{library_core_name}_hwinfo_t'
      info_variable_name = f'{ip_instance_name}_info'
      info_static_variable_name = f'{info_variable_name}_static'
      para_struct_name = f'{library_core_name}_hwpara_t'
      para_variable_name = f'{info_variable_name}_para'
      for interface_xml in ip_instance_xml.findall('interface'):
        library_name = interface_xml.find('library_name').text
        if library_name.startswith('mmiox1'):
          mmiox_name = interface_xml.find('name').text
      mmiox_info_name = f'{ip_instance_name}_{mmiox_name}_info'
      assert mmiox_name
      #
      header_line_list.append(f'//{info_variable_name}')
      header_line_list.append(f'extern const {info_struct_name}* const {info_variable_name};')
      #
      body_dec_list.append(f'//{info_variable_name}')
      body_dec_list.append(f'static {info_struct_name} {info_static_variable_name} CACHED_DATA;')
      body_dec_list.append(f'const {info_struct_name}* const {info_variable_name} = &{info_static_variable_name};')
      #body_dec_list.append(f'const {info_struct_name}* const {info_variable_name} = {mmiox_info_name};')
      #
      body_construct_list.append(f'\t//{info_variable_name}')
      body_construct_list.append(f'\t{para_struct_name} {para_variable_name};')
      for parameter_xml in ip_instance_xml.findall('parameter'):
        parameter_name = parameter_xml.find('id').text.lower()
        parameter_value = parameter_xml.find('value').text
        body_construct_list.append(f'\t{para_variable_name}.{parameter_name} = {parameter_value};')
      body_construct_list.append(f'\t{library_core_name}_hwinfo_elaborate(&{para_variable_name},&{info_static_variable_name});')
      body_construct_list.append(f'\t{info_static_variable_name}.mmiox_info = {mmiox_info_name};')
      
  return header_include_list, header_line_list, body_include_list, body_dec_list, body_construct_list, body_destruct_list, body_line_list

def generate_dca_neurgemm(xml_root):
  header_include_list =[]
  header_line_list =[]
  body_include_list = []
  body_dec_list = []
  body_construct_list = []
  body_destruct_list = []
  body_line_list = []
  
  for ip_instance_xml in xml_root.findall('ip_instance'):
    ip_instance_name = ip_instance_xml.find('name').text
    library_name = ip_instance_xml.find('library_name').text
    if library_name.startswith('dca_neurgemm') and 'mmiox' in library_name:
      library_core_name = get_library_core_name(library_name)
      header_include_list.append(f'{library_core_name}.h')
      info_struct_name = f'{library_core_name}_hwinfo_t'
      info_variable_name = f'{ip_instance_name}_info'
      info_static_variable_name = f'{info_variable_name}_static'
      para_struct_name = f'{library_core_name}_hwpara_t'
      para_variable_name = f'{info_variable_name}_para'
      for interface_xml in ip_instance_xml.findall('interface'):
        library_name = interface_xml.find('library_name').text
        if library_name.startswith('mmiox1'):
          mmiox_name = interface_xml.find('name').text
      mmiox_info_name = f'{ip_instance_name}_{mmiox_name}_info'
      assert mmiox_name
      #
      header_line_list.append(f'//{info_variable_name}')
      header_line_list.append(f'extern const {info_struct_name}* const {info_variable_name};')
      #
      body_dec_list.append(f'//{info_variable_name}')
      body_dec_list.append(f'static {info_struct_name} {info_static_variable_name} CACHED_DATA;')
      body_dec_list.append(f'const {info_struct_name}* const {info_variable_name} = &{info_static_variable_name};')
      #body_dec_list.append(f'const {info_struct_name}* const {info_variable_name} = {mmiox_info_name};')
      #
      body_construct_list.append(f'\t//{info_variable_name}')
      body_construct_list.append(f'\t{para_struct_name} {para_variable_name};')
      for parameter_xml in ip_instance_xml.findall('parameter'):
        parameter_name = parameter_xml.find('id').text.lower()
        parameter_value = parameter_xml.find('value').text
        body_construct_list.append(f'\t{para_variable_name}.{parameter_name} = {parameter_value};')
      body_construct_list.append(f'\t{library_core_name}_hwinfo_elaborate(&{para_variable_name},&{info_static_variable_name});')
      body_construct_list.append(f'\t{info_static_variable_name}.mmiox_info = {mmiox_info_name};')
      
  return header_include_list, header_line_list, body_include_list, body_dec_list, body_construct_list, body_destruct_list, body_line_list

def generate_starc(xml_root):
  header_include_list =[]
  header_line_list =[]
  body_include_list = []
  body_dec_list = []
  body_construct_list = []
  body_destruct_list = []
  body_line_list = []
  
  for ip_instance_xml in xml_root.findall('ip_instance'):
    ip_instance_name = ip_instance_xml.find('name').text
    library_name = ip_instance_xml.find('library_name').text
    if library_name.startswith('starc') and 'mmiox' in library_name:
      library_core_name = get_library_core_name(library_name)
      header_include_list.append(f'{library_core_name}.h')
      info_struct_name = f'{library_core_name}_hwinfo_t'
      info_variable_name = f'{ip_instance_name}_info'
      info_static_variable_name = f'{info_variable_name}_static'
      para_struct_name = f'{library_core_name}_hwpara_t'
      para_variable_name = f'{info_variable_name}_para'
      for interface_xml in ip_instance_xml.findall('interface'):
        library_name = interface_xml.find('library_name').text
        if library_name.startswith('mmiox1'):
          mmiox_name = interface_xml.find('name').text
      mmiox_info_name = f'{ip_instance_name}_{mmiox_name}_info'
      assert mmiox_name
      #
      header_line_list.append(f'//{info_variable_name}')
      header_line_list.append(f'extern const {info_struct_name}* const {info_variable_name};')
      #
      body_dec_list.append(f'//{info_variable_name}')
      body_dec_list.append(f'static {info_struct_name} {info_static_variable_name} CACHED_DATA;')
      body_dec_list.append(f'const {info_struct_name}* const {info_variable_name} = &{info_static_variable_name};')
      #body_dec_list.append(f'const {info_struct_name}* const {info_variable_name} = {mmiox_info_name};')
      #
      body_construct_list.append(f'\t//{info_variable_name}')
      body_construct_list.append(f'\t{para_struct_name} {para_variable_name};')
      for parameter_xml in ip_instance_xml.findall('parameter'):
        parameter_name = parameter_xml.find('id').text.lower()
        parameter_value = parameter_xml.find('value').text
        body_construct_list.append(f'\t{para_variable_name}.{parameter_name} = {parameter_value};')
      body_construct_list.append(f'\t{library_core_name}_hwinfo_elaborate(&{para_variable_name},&{info_static_variable_name});')
      body_construct_list.append(f'\t{info_static_variable_name}.mmiox_info = {mmiox_info_name};')
      
  return header_include_list, header_line_list, body_include_list, body_dec_list, body_construct_list, body_destruct_list, body_line_list

if __name__ == '__main__':
  # parse argument
  parser = argparse.ArgumentParser(description='Generate ip instance ssw')
  parser.add_argument('-input', '-i', help='an input xml file')
  parser.add_argument('-output', '-o', help='output directory')
  args = parser.parse_args()
  
  assert args.input
  assert args.output
  
  input_path = Path(args.input).resolve()
  assert input_path.is_file(), input_path
  output_dir = Path(args.output).resolve()
  assert output_dir.is_dir(), output_dir
  
  xml_tree = XmlTree.parse(input_path)
  xml_root = xml_tree.getroot()
  assert xml_root.tag=='rvx', xml_root.tag
  
  # basic
  total_header_include_list = []
  total_header_line_list = []
  total_body_include_list = []
  total_body_dec_list = []
  total_body_construct_list = []
  total_body_destruct_list = []
  total_body_line_list = []
  
  header_name = input_path.stem
  
  total_body_include_list.append('platform_info.h')
  total_body_include_list.append('ervp_malloc.h')
  total_body_include_list.append('ervp_variable_allocation.h')
  total_body_include_list.append('core_dependent.h')
  total_body_include_list.append(f'{header_name}.h')
  
  generate_function_list = (generate_mmiox1,generate_extinput,generate_dca_mru,generate_dca_matrix,generate_dca_neugemm,generate_dca_neurgemm,generate_starc)
  for generate_function in generate_function_list:
    header_include_list, header_line_list, body_include_list, body_dec_list, body_construct_list, body_destruct_list, body_line_list = generate_function(xml_root)
    total_header_include_list += header_include_list
    total_header_line_list += header_line_list
    total_body_include_list += body_include_list
    total_body_dec_list += body_dec_list
    total_body_construct_list += body_construct_list
    total_body_destruct_list += body_destruct_list
    total_body_line_list += body_line_list
  
  # header
  line_list = []
  line_list.append(f'#ifndef __{header_name.upper()}_H__')
  line_list.append(f'#define __{header_name.upper()}_H__')
  
  line_list.append('')
  total_header_include_list_unique = tuple(dict.fromkeys(total_header_include_list))
  line_list += [f'#include \"{x}\"' for x in total_header_include_list_unique]
  
  line_list.append('')
  line_list += total_header_line_list
  
  line_list.append('')
  line_list.append('#endif')
  
  header_path = output_dir / f'{header_name}.h'
  header_path.write_text('\n'.join(line_list))
  
  # body
  line_list = []
  total_body_include_list_unique = tuple(dict.fromkeys(total_body_include_list))
  line_list += [f'#include \"{x}\"' for x in total_body_include_list_unique]
  
  line_list.append('')
  line_list += total_body_dec_list
  
  if total_body_construct_list:
    line_list.append('')
    line_list.append(f'static void __attribute__ ((constructor)) construct_{header_name}()')
    line_list.append('{')
    line_list += total_body_construct_list
    line_list.append('\t//')
    line_list.append('\tflush_cache();')
    line_list.append('}')
    
  if total_body_destruct_list:  
    line_list.append('')
    line_list.append(f'static void __attribute__ ((destructor)) destruct_{header_name}()')
    line_list.append('{')
    line_list += total_body_destruct_list
    line_list.append('}')
  
  line_list.append('')
  line_list += body_line_list
  
  body_path = output_dir / f'{header_name}.c'
  body_path.write_text('\n'.join(line_list))
