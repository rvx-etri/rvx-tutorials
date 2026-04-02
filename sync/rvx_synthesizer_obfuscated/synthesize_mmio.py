import argparse
import copy
import xml.etree.ElementTree as XmlTree
from xml.dom import minidom
from pathlib import Path
from itertools import chain
from collections import defaultdict

from rvx_util import *
from verilog_generator import *
from c_generator import *
from tcl_generator import *
#import generate_copyright

def set_from_xml_element(xml_parent:XmlTree, tag:str, type_convertor, default_value=None):
  assert xml_parent
  xml_element = xml_parent.find(tag)
  if xml_element==None:
    result = default_value
  else:
    result = type_convertor(xml_element.text)
  return result

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

def generate_exp_xml_element(op, xml_value_list):
  xml_element = XmlTree.Element('exp')
  xml_element.attrib['op'] = op
  for xml_value in xml_value_list:
    xml_element.append(xml_value)
  return xml_element

def generate_comment_xml_elemenet(text):
  xml_element = XmlTree.Element('comment')
  xml_element.text = str(text)
  return xml_element

def divide_value_list(text:str):
  unassigned_list = [] 
  assigned_list = []
  modified_list = []
  for value in text.split(','):
    if '*' in value:
      assert ':' not in value, value
      value, size = value.split('*')
      size = int(size)
      assert size < 100, size
      for i in range(size):
        modified_list.append(f'{value}_{i:03}')
    else:
      modified_list.append(value)
  for i, assigned_value in enumerate(modified_list):
    if i==0:
      is_assigned = True if ':' in assigned_value else False
    assign_info = assigned_value.split(':')
    if len(assign_info)==1:
      if is_assigned:
        recent_value += 1
        assigned_list.append((assign_info[0].strip(),recent_value))
      else:
        unassigned_list.append(assign_info[0].strip())
    elif len(assign_info)==2:
      recent_value = int(eval(assign_info[1]))
      assigned_list.append((assign_info[0].strip(),recent_value))
    else:
      assert 0
  return (unassigned_list, assigned_list)

class RegValue():
  def __init__(self, xml:XmlTree=None):
    self.name = None
    self.assigned_list = []
    self.unassigned_list = []
    self.bitwidth = None
    self.default_value = 0
    self.attrib = dict()

    if xml:
      self.name = xml.find('name').text
      self.attrib = xml.attrib.copy()
      for formatted_text in xml.findall('assign'):
        unassigned_list, assigned_list = divide_value_list(formatted_text.text)
        self.unassigned_list += unassigned_list
        self.assigned_list += assigned_list
      self.bitwidth = set_from_xml_element(xml, 'bitwidth', int)
      self.default_value = set_from_xml_element(xml, 'default_value', eval, 0)

  @property
  def assign_type(self):
    return self.attrib['assign_type']
  
  def synthesize(self):
    self.attrib.setdefault('assign_type', 'min')
    self.attrib.setdefault('default_value_type', 'define')
    max = 0
    assigned_value_set = set()
    for name, value in self.assigned_list:
      assigned_value_set.add(value)
      if value > max:
        max = value
    if self.assign_type=='min':
      count = 0
    elif self.assign_type=='onehot':
      count = 1
    else:
      assert 0, self.type
    for name in self.unassigned_list:
      while 1:
        if count not in assigned_value_set:
          assigned_value_set.add(count)
          break
        else:
          if self.assign_type=='min':
            count += 1
          elif self.assign_type=='onehot':
            count <<= 1
      self.assigned_list.append((name,count))
    self.unassigned_list = []
    
    for name, count in tuple(self.assigned_list):
      if count<=0:
        continue
      index = int(round(math.log(count,2)))
      if (1<<index)==count:
        index = int(index)
        self.assigned_list.append((f'INDEX_{name}',index))
      elif self.assign_type=='onehot':
        assert 0, (count, index)
    if self.assign_type=='onehot':
      self.assigned_list.append((f'NONE',0))
    if count > max:
      max = count
    #
    if self.bitwidth==None:
      if len(self.assigned_list)==0 and len(self.unassigned_list)==0:
        self.bitwidth = 32
      else:
        self.bitwidth = get_required_bitwidth_unsigend(max)

  def export_as_xml(self):
    xml_regvalue = XmlTree.Element('regvalue')
    xml_regvalue.attrib = self.attrib.copy()
    # name
    append_child_xml_element(xml_regvalue, 'name', self.name)
    # bitwidth
    append_child_xml_element(xml_regvalue, 'bitwidth', str(self.bitwidth))
    # default_value
    append_child_xml_element(xml_regvalue, 'default_value', str(self.default_value))
    # value
    for name, value in self.assigned_list:
      append_child_xml_element(xml_regvalue, 'assign', f'{name}:{value}')
    return xml_regvalue
  
  def export_define_as_xml(self, use_reduced_name:bool):
    converted_name = self.name.replace('.','_')
    xml_group = XmlTree.Element('define_group')
    # comment
    xml_group.append(generate_comment_xml_elemenet(f'reg {self.name}'))
    # bitwidth
    xml_element = generate_define_xml_element(f'bw_{converted_name}',self.bitwidth,'dec')
    xml_group.append(xml_element)
    # default_value
    if self.attrib['default_value_type']=='define':
      xml_element = generate_define_xml_element(f'{converted_name}_default_value',self.default_value,'lang_verilog')
      xml_group.append(xml_element)
    # value
    for name, value in self.assigned_list:
      xml_element = generate_define_xml_element(f'{converted_name}_{name}',value,'dec')
      xml_group.append(xml_element)
    return xml_group

  def export_memorymap_as_xml(self, use_reduced_name:bool):
    converted_name = self.name.replace('.','_')
    xml_group = XmlTree.Element('define_group')
    # comment
    xml_group.append(generate_comment_xml_elemenet(f'reg {self.name}'))
    # bitwidth
    xml_element = generate_comment_xml_elemenet(f'BW_{converted_name.upper()} {self.bitwidth}')
    xml_group.append(xml_element)
    # value
    for name, value in self.assigned_list:
      xml_element = generate_comment_xml_elemenet(f'{converted_name.upper()}_{name.upper()} {value}')
      xml_group.append(xml_element)
    return xml_group

class SubmoduleMemorymapOffset():
  def __init__(self, xml:XmlTree=None):
    self.owner = None
    self.name = None
    self.unassigned_list = []
    self.assigned_list = []
    self.bitwidth = None
    self.bw_unused = None
    self.index = None
    self.reg_attrib_dict = defaultdict(dict)

    if xml:
      self.name = xml.find('name').text
      for formatted_text in xml.findall('reg'):
        unassigned_list, assigned_list = divide_value_list(formatted_text.text)
        array_size = formatted_text.attrib.get('array_size')

        # unassigned_list
        if array_size:
          assert not assigned_list, assigned_list
          total_array_list = []
          for x in unassigned_list:
            attrib = formatted_text.attrib.copy()
            attrib.setdefault('type', 'general')
            attrib.setdefault('readyin', 'False')
            if attrib['type'].endswith('fifo'):
              attrib.setdefault('fifo_depth', '4')
            else:
              attrib.setdefault('fifo_depth', '0')
            assert not (attrib['type']=='simple' and attrib['readyin']=='True')
            attrib['array'] = x
            attrib.pop('array_size')
            array_list = [x+'{0:02}'.format(i) for i in range(0,int(array_size))]
            total_array_list += array_list
            for x in array_list:
              self.reg_attrib_dict[x] = attrib
          unassigned_list = total_array_list
        else:
          attrib = formatted_text.attrib.copy()
          attrib.setdefault('type', 'general')
          attrib.setdefault('readyin', 'False')
          if attrib['type'].endswith('fifo'):
            attrib.setdefault('fifo_depth', '4')
          else:
            attrib.setdefault('fifo_depth', '0')
          assert not (attrib['type']=='simple' and attrib['readyin']=='True')
          attrib.setdefault('array', '')
          for reg_name in chain(unassigned_list,[x for x,y in assigned_list]):
            self.reg_attrib_dict[reg_name] = copy.deepcopy(attrib)
        
        #
        self.unassigned_list += unassigned_list
        self.assigned_list += assigned_list
      
      self.bitwidth = set_from_xml_element(xml, 'bitwidth', int)
      self.bw_unused = set_from_xml_element(xml, 'bw_unused', int)
      self.index = set_from_xml_element(xml, 'index', int)
  
  def get_array_name(self, reg_name):
    return self.reg_attrib_dict[reg_name]['array']

  def get_type(self, reg_name):
    return self.reg_attrib_dict[reg_name]['type']

  def has_readyin(self, reg_name):
    return answer2bool(self.reg_attrib_dict[reg_name]['readyin'])

  def get_fifo_depth(self, reg_name):
    return int(self.reg_attrib_dict[reg_name]['fifo_depth'])

  def get_regvalue_name(self, reg_name):
    array_name = self.get_array_name(reg_name)
    if array_name:
      reg_name = array_name
    regvalue_name = f'{self.name}.{reg_name}'
    return regvalue_name

  def synthesize(self):
    if self.bw_unused==None:
      self.bw_unused = self.owner.bw_unused
    valid_num_list = []
    for reg_name in self.unassigned_list:
      attrib = self.reg_attrib_dict[reg_name]
      if attrib['type']=='rfifo':
        valid_num_reg_name = f'{reg_name}_valid_num'
        valid_num_reg_attrib = {}
        valid_num_reg_attrib['type'] = 'rfifo_valid_num'
        # bitwidth that represents the values ranging 0 to fifo_depth
        valid_num_reg_attrib['bitwidth'] = int(math.log(int(attrib['fifo_depth']), 2)) + 1
        valid_num_reg_attrib.setdefault('readyin', 'False')
        valid_num_reg_attrib.setdefault('array', '')
        valid_num_reg_attrib.setdefault('default_value_type', 'define')
        valid_num_list.append(valid_num_reg_name)
        self.reg_attrib_dict[valid_num_reg_name] = valid_num_reg_attrib
    self.unassigned_list += valid_num_list
    #
    count = -self.owner.addr_interval
    for reg_name in self.unassigned_list:
      count += self.owner.addr_interval
      self.assigned_list.append((reg_name,count))
    self.unassigned_list = []

    if not self.bitwidth:
      if count<=0:
        self.bitwidth = 1
      else:
        self.bitwidth = get_required_bitwidth_unsigend(count)
    if self.bitwidth < self.bw_unused:
      self.bitwidth = self.bw_unused

    for reg_name, assigned_value in self.assigned_list:
      regvalue_name = self.get_regvalue_name(reg_name)
      regvalue = self.owner.local_regvalue_dict.get(regvalue_name)
      if not regvalue:
        regvalue = RegValue()
        regvalue.name = regvalue_name
        self.owner.local_regvalue_dict[regvalue_name] = regvalue

      reg_attrib_bitwidth = self.reg_attrib_dict[reg_name].get('bitwidth')
      if reg_attrib_bitwidth:
        reg_attrib_bitwidth = int(reg_attrib_bitwidth)
        if regvalue.bitwidth==None:
          regvalue.bitwidth = reg_attrib_bitwidth
        else:
          assert regvalue.bitwidth==reg_attrib_bitwidth, (reg_name,regvalue.bitwidth,reg_attrib_bitwidth)
        del(self.reg_attrib_dict[reg_name]['bitwidth'])

    #
    for reg_name, assigned_value in self.assigned_list:
      regvalue_name = self.get_regvalue_name(reg_name)
      regvalue = self.owner.local_regvalue_dict.get(regvalue_name)
      assert regvalue
      default_value_type_from_reg_value = regvalue.attrib.get('default_value_type')
      default_value_type_from_reg = self.reg_attrib_dict[reg_name].get('default_value_type')
      if default_value_type_from_reg_value and default_value_type_from_reg:
        assert default_value_type_from_reg_value==default_value_type_from_reg
      elif default_value_type_from_reg_value:
        pass
      elif default_value_type_from_reg:
        regvalue.attrib['default_value_type'] = default_value_type_from_reg
        del(self.reg_attrib_dict[reg_name]['default_value_type'])
      else:
        regvalue.attrib['default_value_type'] = 'define'

  def export_as_xml(self):
    #
    xml_submodule = XmlTree.Element('submodule')
    # name
    append_child_xml_element(xml_submodule, 'name', self.name)
    # bitwidth
    if self.bitwidth!=None:
      append_child_xml_element(xml_submodule, 'bitwidth', str(self.bitwidth))
    # bw_unused
    if self.bw_unused!=None:
      append_child_xml_element(xml_submodule, 'bw_unused', str(self.bw_unused))
    # index
    if self.index!=None:
      append_child_xml_element(xml_submodule, 'index', str(self.index))
    # submodule offset
    for name, value in self.assigned_list:
      append_child_xml_element(xml_submodule, 'reg', f'{name}:{value}', self.reg_attrib_dict[name])
    return xml_submodule
  
  def export_define_as_xml(self, use_reduced_name:bool):
    xml_group_list = []
    #
    xml_group = XmlTree.Element('define_group')
    xml_group_list.append(xml_group)
    # comment
    xml_group.append(generate_comment_xml_elemenet(f'submodule {self.name}'))
    # bit width
    if self.owner.num_submodule > 1:
      xml_group.append(generate_define_xml_element(f'bw_mmap_suboffset_{self.name}',self.bitwidth,'dec'))
      xml_group.append(generate_define_xml_element(f'bw_unused_{self.name}',self.bw_unused,'dec'))
    # submodule offset
    for name, value in self.assigned_list:
      xml_group.append(generate_define_xml_element(f'mmap_suboffset_{self.name}_{name}',hex(value),'hex'))
    # module offset
    xml_group = XmlTree.Element('define_group')
    xml_group_list.append(xml_group)
    for name, value in self.assigned_list:
      xml_element = XmlTree.SubElement(xml_group, 'define')
      if use_reduced_name:
        mmap_offset_name = f'mmap_offset_{name}'.upper()
      else:
        mmap_offset_name = f'mmap_offset_{self.name}_{name}'.upper()
      append_child_xml_element(xml_element, 'name', mmap_offset_name)
      if self.owner.num_submodule <= 1:
        xml_element.append(generate_value_xml_element(f'($mmap_suboffset_{self.name}_{name})'.upper(),'exp'))
      else:
        var_a = f'submodule_addr_{self.owner.name}_{self.name}'.upper()
        var_b = f'mmap_suboffset_{self.name}_{name}'.upper()
        define_value = generate_value_xml_element(f'(`{var_a}+`{var_b})', 'lang_verilog')
        xml_element.append(define_value)
        define_value = generate_value_xml_element(f'({var_a}+{var_b})', 'lang_c')
        xml_element.append(define_value)
        define_value = generate_value_xml_element(f'[expr $::{var_a} + $::{var_b}]', 'lang_tcl')
        xml_element.append(define_value)
    # array_size
    num_dict = defaultdict(int)
    for reg_attrib in self.reg_attrib_dict.values():
      array_name = reg_attrib.get('array')
      if array_name:
        num_dict[array_name] += 1
    for array_name, array_num in num_dict.items():
      xml_group.append(generate_define_xml_element(f'num_{self.name}_{array_name}',array_num,'dec'))
    return xml_group_list

  def export_memorymap_as_xml(self, use_reduced_name:bool):
    xml_group = XmlTree.Element('define_group')
    # comment
    xml_group.append(generate_comment_xml_elemenet(f'submodule {self.name}'))
    # module offset
    for name, value in self.assigned_list:
      xml_element = XmlTree.SubElement(xml_group, 'define')
      if use_reduced_name:
        mmap_name = f'mmap_{name}'.upper()
      else:
        mmap_name = f'mmap_{self.name}_{name}'.upper()
      append_child_xml_element(xml_element, 'name', mmap_name)
      var_a = '\"${MODULE_BASEADDR}\"'
      if use_reduced_name:
        var_b = f'mmap_offset_{name}'.upper()
      else:
        var_b = f'mmap_offset_{self.name}_{name}'.upper()
      define_value = generate_value_xml_element(f'({var_a}+{var_b})', 'lang_c')
      xml_element.append(define_value)
    return xml_group

class IpMemorymapOffset():
  def __init__(self, xml:XmlTree=None):
    self.submodule_dict = {}
    self.local_regvalue_dict = {}
    self.global_regvalue_dict = {}
    self.addr_interval = None
    self.bw_unused = None
    self.bitwidth = None
    self.num_submodule = None
    self.max_bw_submodule = None

    # must
    assert xml
    self.name = xml.find('name').text

    # set if exists
    self.addr_interval = set_from_xml_element(xml, 'addr_interval', int, 8)
    self.bw_unused = set_from_xml_element(xml, 'bw_unused', int)
    self.bitwidth = set_from_xml_element(xml, 'bitwidth', int)
    self.max_bw_submodule = set_from_xml_element(xml, 'max_bw_submodule', int)

    # auto
    for submodule_xml in xml.findall('submodule'):
      submodule = SubmoduleMemorymapOffset(submodule_xml)
      self.add_submodule(submodule)
    for regvalue_xml in xml.findall('regvalue'):
      regvalue = RegValue(regvalue_xml)
      self.add_regvalue(regvalue)
    self.num_submodule = len(self.submodule_dict)
    if self.num_submodule > 1:
      self.bw_sel_submodule = get_required_bitwidth_unsigend(self.num_submodule-1)
    else:
      self.bw_sel_submodule = 1

  def add_submodule(self, submodule):
    self.submodule_dict[submodule.name] = submodule
    assert submodule.owner==None, submodule.owner
    submodule.owner = self
  
  def add_regvalue(self, regvalue):
    if '.' in regvalue.name :
      self.local_regvalue_dict[regvalue.name] = regvalue
    else:
      self.global_regvalue_dict[regvalue.name] = regvalue

  def synthesize(self):
    if self.bw_unused==None:
      self.bw_unused = int(math.log(self.addr_interval,2))
    if self.addr_interval==None:
      self.addr_interval = 8
    for submodule in self.submodule_dict.values():
      submodule.synthesize()
    for regvalue in self.local_regvalue_dict.values():
      regvalue.synthesize()
    for regvalue in self.global_regvalue_dict.values():
      regvalue.synthesize()
    #
    if self.num_submodule > 1:
      index = 0
      self.max_bw_submodule = 0
      for submodule in self.submodule_dict.values():
        submodule.index = index
        index += 1
        if submodule.bitwidth > self.max_bw_submodule:
          self.max_bw_submodule = submodule.bitwidth
      self.bitwidth = self.bw_sel_submodule + self.max_bw_submodule
    elif self.num_submodule==1:
      for submodule in self.submodule_dict.values():
        self.bitwidth = submodule.bitwidth
      assert self.bitwidth, self.name
      assert self.bitwidth>=self.bw_unused, (self.bitwidth,self.bw_unused)
  
  def export_as_xml(self):
    xml_root = XmlTree.Element('rvx')
    xml_def_ip = XmlTree.SubElement(xml_root, 'def_ip')
    append_child_xml_element(xml_def_ip, 'name', self.name)
    if self.addr_interval!=None:
      append_child_xml_element(xml_def_ip, 'addr_interval', str(self.addr_interval))
    if self.bitwidth!=None:
      append_child_xml_element(xml_def_ip, 'bitwidth', str(self.bitwidth))
    if self.bw_unused!=None:
      append_child_xml_element(xml_def_ip, 'bw_unused', str(self.bw_unused))
    if self.max_bw_submodule!=None:
      append_child_xml_element(xml_def_ip, 'max_bw_submodule', str(self.max_bw_submodule))
    #
    for submodule in self.submodule_dict.values():
      xml_def_ip.append(submodule.export_as_xml())
    for regvalue in self.local_regvalue_dict.values():
      xml_def_ip.append(regvalue.export_as_xml())
    for regvalue in self.global_regvalue_dict.values():
      xml_def_ip.append(regvalue.export_as_xml())
    return xml_root

  def export(self, output_directory:Path):
    xml_root = self.export_as_xml()
    xml_str = XmlTree.tostring(xml_root, encoding="unicode")
    xml_str = minidom.parseString(xml_str).toprettyxml(indent="\t")
    output_file = output_directory / f'{self.name}_synthesized.xml'
    output_file.write_text(xml_str)
    
  def export_define_as_xml(self, use_reduced_name:bool):
    xml_root = XmlTree.Element('rvx')
    xml_info = XmlTree.SubElement(xml_root, 'info')
    append_child_xml_element(xml_info, 'name', f'{self.name}_memorymap_offset')
    if self.num_submodule > 0:
      #
      xml_group = XmlTree.SubElement(xml_info, 'define_group')
      xml_group.append(generate_comment_xml_elemenet(f'total'))
      #
      xml_group.append(generate_define_xml_element(f'bw_mmap_offset_{self.name}',self.bitwidth,'dec'))
      xml_group.append(generate_define_xml_element(f'{self.name}_addr_interval',self.addr_interval,'dec'))
      xml_group.append(generate_define_xml_element(f'bw_unused_{self.name}',self.bw_unused,'dec'))
      xml_group.append(generate_define_xml_element(f'num_{self.name}_submodule',self.num_submodule,'dec'))
    if self.num_submodule > 1:
      xml_group.append(generate_define_xml_element(f'bw_sel_{self.name}_submodule',self.bw_sel_submodule,'lang_verilog'))
      for submodule in self.submodule_dict.values():
        xml_group.append(generate_define_xml_element(f'submodule_index_{self.name}_{submodule.name}',submodule.index,'dec'))
        addr = submodule.index * (2**self.max_bw_submodule)
        xml_group.append(generate_define_xml_element(f'submodule_addr_{self.name}_{submodule.name}',hex(addr),'hex'))
    #
    for submodule in self.submodule_dict.values():
      for define_group in submodule.export_define_as_xml(use_reduced_name):
        xml_info.append(define_group)
    for regvalue in self.local_regvalue_dict.values():
      xml_info.append(regvalue.export_define_as_xml(use_reduced_name))
    for regvalue in self.global_regvalue_dict.values():
      xml_info.append(regvalue.export_define_as_xml(use_reduced_name))
    return xml_root

  def export_define(self, use_reduced_name:bool, output_directory:Path):
    xml_root = self.export_define_as_xml(use_reduced_name)
    xml_str = XmlTree.tostring(xml_root, encoding="unicode")
    xml_str = minidom.parseString(xml_str).toprettyxml(indent="\t")
    output_file = output_directory / f'{self.name}_memorymap_offset.xml'
    output_file.write_text(xml_str)

  def export_memorymap_as_xml(self, use_reduced_name:bool):
    xml_root = XmlTree.Element('rvx')
    xml_info = XmlTree.SubElement(xml_root, 'info')
    append_child_xml_element(xml_info, 'name', f'{self.name}_memorymap')
    append_child_xml_element(xml_info, 'include_header', 'platform_info')
    append_child_xml_element(xml_info, 'include_header', f'{self.name}_memorymap_offset')

    # submodule itself
    if self.num_submodule > 1:
      for submodule in self.submodule_dict.values():
        xml_element = XmlTree.SubElement(xml_info, 'define')
        mmap_name = f'mmap_submodule_{submodule.name}'.upper()
        append_child_xml_element(xml_element, 'name', mmap_name)
        var_a = '\"${MODULE_BASEADDR}\"'
        var_b = f'submodule_addr_{self.name}_{submodule.name}'.upper()
        define_value = generate_value_xml_element(f'({var_a}+{var_b})', 'lang_c')
        xml_element.append(define_value)

    #
    for submodule in self.submodule_dict.values():
      xml_info.append(submodule.export_memorymap_as_xml(use_reduced_name))
    for regvalue in self.local_regvalue_dict.values():
      xml_info.append(regvalue.export_memorymap_as_xml(use_reduced_name))
    for regvalue in self.global_regvalue_dict.values():
      xml_info.append(regvalue.export_memorymap_as_xml(use_reduced_name))
    return xml_root
  
  def export_memorymap(self, use_reduced_name:bool, output_directory:Path):
    xml_root = self.export_memorymap_as_xml(use_reduced_name)
    xml_str = XmlTree.tostring(xml_root, encoding="unicode")
    xml_str = minidom.parseString(xml_str).toprettyxml(indent="\t")
    output_file = output_directory / f'{self.name}_memorymap.xml'
    output_file.write_text(xml_str)
  
  def export_verilog(self, use_reduced_name:bool, submodule_list:list, output_directory:Path):
    template_dir = Path(os.environ['RVX_SYNTHESIZER_HOME']) / 'mmio_template'
    assert template_dir.is_dir(), template_dir
    mmio_list = []
    non_group_mmio_list = []
    group_mmio_dict = {}
    for submodule_name, submodule in self.submodule_dict.items():
      if submodule_name in submodule_list:
        for assigned_value in submodule.assigned_list:
          mmio_name = assigned_value[0]
          bitwidth = int(self.local_regvalue_dict[submodule.get_regvalue_name(mmio_name)].bitwidth)
          type = submodule.get_type(mmio_name)
          has_readyin = submodule.has_readyin(mmio_name)
          array_name = submodule.get_array_name(mmio_name)
          fifo_depth = submodule.get_fifo_depth(mmio_name)
          default_value_type = self.local_regvalue_dict[submodule.get_regvalue_name(mmio_name)].attrib['default_value_type']
          if array_name:
            array_signal_name = array_name if use_reduced_name else f'{submodule.name}_{array_name}'
            signal_name = mmio_name if use_reduced_name else f'{submodule.name}_{mmio_name}'
            group_mmio = group_mmio_dict.setdefault(array_signal_name, (submodule.name, array_name, bitwidth, type, has_readyin, fifo_depth, default_value_type, []))
            group_mmio[-1].append(signal_name)
            mmio_list.append((submodule.name, mmio_name, signal_name, array_signal_name, bitwidth, type, has_readyin, fifo_depth, default_value_type))
          else:
            signal_name = mmio_name if use_reduced_name else f'{submodule.name}_{mmio_name}'
            non_group_mmio_list.append((submodule.name, mmio_name, signal_name, bitwidth, type, has_readyin, fifo_depth, default_value_type))
            mmio_list.append((submodule.name, mmio_name, signal_name, None, bitwidth, type, has_readyin, fifo_depth, default_value_type))
    #print(non_group_mmio_list)
    #print(group_mmio_dict)
    #print(non_group_mmio_list)
    #
    line_list = []
    line_list.append((template_dir/'mmio.00.vh').read_text())
    port_list = []
    # port dec
    port_list.append('tick_gpio')
    for submodule_name, mmio_name, signal_name, bitwidth, type, has_readyin, fifo_depth, default_value_type in non_group_mmio_list:
      if type=='fake':
        continue
      elif type=='simple' or type=='read_reg' or type=='write_reg':
        port_list.append(f'{signal_name}')
      elif type=='general':
        port_list.append(f'{signal_name}_re')
        port_list.append(f'{signal_name}_rdata')
        port_list.append(f'{signal_name}_we')
        port_list.append(f'{signal_name}_wdata')
      elif type=='read':
        port_list.append(f'{signal_name}_re')
        port_list.append(f'{signal_name}_rdata')
      elif type=='write':
        port_list.append(f'{signal_name}_we')
        port_list.append(f'{signal_name}_wdata')
      elif type=='read_cmd' or type=='write_cmd' or type=='onehot_cmd':
        port_list.append(f'{signal_name}_{type}')
      elif type=='gpio':
        port_list.append(f'{signal_name}_soe')
        port_list.append(f'{signal_name}_soval')
        port_list.append(f'{signal_name}_sival')
        port_list.append(f'{signal_name}_sinterrupt')
      elif type=='wfifo':
        port_list.append(f'{signal_name}_clear')
        port_list.append(f'{signal_name}_rready')
        port_list.append(f'{signal_name}_rempty')
        port_list.append(f'{signal_name}_rnum')
        port_list.append(f'{signal_name}_rrequest')
        port_list.append(f'{signal_name}_rdata')
        port_list.append(f'{signal_name}_wready')
        port_list.append(f'{signal_name}_wfull')
        port_list.append(f'{signal_name}_wnum')
      elif type=='rfifo':
        port_list.append(f'{signal_name}_clear')
        port_list.append(f'{signal_name}_rready')
        port_list.append(f'{signal_name}_rempty')
        port_list.append(f'{signal_name}_rnum')
        port_list.append(f'{signal_name}_wready')
        port_list.append(f'{signal_name}_wfull')
        port_list.append(f'{signal_name}_wnum')
        port_list.append(f'{signal_name}_wrequest')
        port_list.append(f'{signal_name}_wdata')        
      elif type=='rfifo_valid_num':
        pass
      else:
        assert 0, type
      if has_readyin:
        port_list.append(f'{signal_name}_readyin')
    # port dec
    for array_signal_name, (submodule_name, array_name, bitwidth, type, has_readyin, fifo_depth, default_value_type, element_list) in group_mmio_dict.items():
      if type=='simple' or type=='read_reg' or type=='write_reg':
        port_list.append(f'{array_signal_name}_list')
      elif type=='general':
        port_list.append(f'{array_signal_name}_re_list')
        port_list.append(f'{array_signal_name}_rdata_list')
        port_list.append(f'{array_signal_name}_we_list')
        port_list.append(f'{array_signal_name}_wdata_list')
      elif type=='read':
        port_list.append(f'{array_signal_name}_re_list')
        port_list.append(f'{array_signal_name}_rdata_list')
      elif type=='write':
        port_list.append(f'{array_signal_name}_we_list')
        port_list.append(f'{array_signal_name}_wdata_list')
      elif type=='read_cmd' or type=='write_cmd' or type=='onehot_cmd':
        port_list.append(f'{array_signal_name}_{type}_list')
      elif type=='gpio':
        port_list.append(f'{array_signal_name}_sod_list')
        port_list.append(f'{array_signal_name}_soval_list')
        port_list.append(f'{array_signal_name}_sival_list')
        port_list.append(f'{array_signal_name}_sinterrupt_list')
      else:
        assert 0, type
      if has_readyin:
        port_list.append(f'{array_signal_name}_readyin_list')
    line_list.append(',\n'.join([ f'\t{x}' for x in port_list]))
    #
    line_list.append((template_dir/'mmio.01.vh').read_text())

    # parameter
    for submodule_name, mmio_name, signal_name, bitwidth, type, has_readyin, fifo_depth, default_value_type in non_group_mmio_list:
      if default_value_type=='para':
        line_list.append(f'parameter {mmio_name.upper()}_DEFAULT_VALUE = 0;')

    for array_signal_name, (submodule_name, array_name, bitwidth, type, has_readyin, fifo_depth, default_value_type, element_list) in group_mmio_dict.items():
      assert default_value_type=='define', array_signal_name
    line_list.append('')

    # port def
    line_list.append('input wire tick_gpio;')
    for submodule_name, mmio_name, signal_name, bitwidth, type, has_readyin, fifo_depth, default_value_type in non_group_mmio_list:
      if type=='fake':
        continue
      converted_name = signal_name.replace('.','_')
      bitwidth_name_by_define = f'`BW_{converted_name.upper()}'
      bitwidth_name = bitwidth_name_by_define
      line_list.append('')
      if type=='simple' or type=='write_reg':
        line_list.append(f'output wire [{bitwidth_name}-1:0] {signal_name};')
      elif type=='read_reg':
        line_list.append(f'input wire [{bitwidth_name}-1:0] {signal_name};')
      elif type=='general':
        line_list.append(f'output wire {signal_name}_re;')
        line_list.append(f'input wire [{bitwidth_name}-1:0] {signal_name}_rdata;')
        line_list.append(f'output wire {signal_name}_we;')
        line_list.append(f'output wire [{bitwidth_name}-1:0] {signal_name}_wdata;')
      elif type=='read':
        line_list.append(f'output wire {signal_name}_re;')
        line_list.append(f'input wire [{bitwidth_name}-1:0] {signal_name}_rdata;')
      elif type=='write':
        line_list.append(f'output wire {signal_name}_we;')
        line_list.append(f'output wire [{bitwidth_name}-1:0] {signal_name}_wdata;')
      elif type=='read_cmd' or type=='write_cmd':
        line_list.append(f'output wire {signal_name}_{type};')
      elif type=='onehot_cmd':
        line_list.append(f'output wire [{bitwidth_name}-1:0] {signal_name}_{type};')
      elif type=='gpio':
        line_list.append(f'output wire [{bitwidth_name}-1:0] {signal_name}_soe;')
        line_list.append(f'output wire [{bitwidth_name}-1:0] {signal_name}_soval;')
        line_list.append(f'input wire [{bitwidth_name}-1:0] {signal_name}_sival;')
        line_list.append(f'output wire {signal_name}_sinterrupt;')
      elif type=='wfifo':
        line_list.append(f'input wire {signal_name}_clear;')
        line_list.append(f'output wire {signal_name}_wready;')
        line_list.append(f'output wire {signal_name}_wfull;')
        line_list.append(f'output wire [BW_DATA-1:0] {signal_name}_wnum;')
        line_list.append(f'output wire {signal_name}_rready;')
        line_list.append(f'output wire {signal_name}_rempty;')
        line_list.append(f'output wire [BW_DATA-1:0] {signal_name}_rnum;')
        line_list.append(f'input wire {signal_name}_rrequest;')
        line_list.append(f'output wire [{bitwidth_name}-1:0] {signal_name}_rdata;')        
      elif type=='rfifo':
        line_list.append(f'input wire {signal_name}_clear;')
        line_list.append(f'output wire {signal_name}_rready;')
        line_list.append(f'output wire {signal_name}_rempty;')
        line_list.append(f'output wire [BW_DATA-1:0] {signal_name}_rnum;')
        line_list.append(f'output wire {signal_name}_wready;')
        line_list.append(f'output wire {signal_name}_wfull;')
        line_list.append(f'output wire [BW_DATA-1:0] {signal_name}_wnum;')
        line_list.append(f'input wire {signal_name}_wrequest;')
        line_list.append(f'input wire [{bitwidth_name}-1:0] {signal_name}_wdata;')
      elif type=='rfifo_valid_num':
        pass
      else:
        assert 0, type
      if has_readyin:
        line_list.append(f'input wire {signal_name}_readyin;')
    # port def
    for array_signal_name, (submodule_name, array_name, bitwidth, type, has_readyin, fifo_depth, default_value_type, element_list) in group_mmio_dict.items():
      array_size = len(element_list)
      bitwidth_name_by_define = f'`BW_{array_signal_name.upper()}'
      bitwidth_name = bitwidth_name_by_define
      array_size_by_define = f'`NUM_{array_signal_name.upper()}'
      array_size_name = array_size_by_define
      if type=='fake':
        continue
      line_list.append('')
      if type=='simple' or type=='write_reg':
        line_list.append(f'output wire [({bitwidth_name})*({array_size_name})-1:0] {array_signal_name}_list;')
      elif type=='read_reg':
        line_list.append(f'input wire [({bitwidth_name})*({array_size_name})-1:0] {array_signal_name}_list;')
      elif type=='general':
        line_list.append(f'output wire [{array_size_name}-1:0] {array_signal_name}_re_list;')
        line_list.append(f'input wire [({bitwidth_name})*({array_size_name})-1:0] {array_signal_name}_rdata_list;')
        line_list.append(f'output wire [{array_size_name}-1:0] {array_signal_name}_we_list;')
        line_list.append(f'output wire [({bitwidth_name})*({array_size_name})-1:0] {array_signal_name}_wdata_list;')
      elif type=='read':
        line_list.append(f'output wire [{array_size_name}-1:0] {array_signal_name}_re_list;')
        line_list.append(f'input wire [({bitwidth_name})*({array_size_name})-1:0] {array_signal_name}_rdata_list;')
      elif type=='write':
        line_list.append(f'output wire [{array_size_name}-1:0] {array_signal_name}_we_list;')
        line_list.append(f'output wire [({bitwidth_name})*({array_size_name})-1:0] {array_signal_name}_wdata_list;')
      elif type=='read_cmd' or type=='write_cmd' or type=='onehot_cmd':
        line_list.append(f'output wire [{array_size_name}-1:0] {array_signal_name}_{type}_list;')
      elif type=='gpio':
        line_list.append(f'output wire [({bitwidth_name})*({array_size_name})-1:0]{array_signal_name}_sod_list;')
        line_list.append(f'output wire [({bitwidth_name})*({array_size_name})-1:0] {array_signal_name}_soval_list;')
        line_list.append(f'input wire [({bitwidth_name})*({array_size_name})-1:0] {array_signal_name}_sival_list;')
        line_list.append(f'output wire [{array_size_name}-1:0] {array_signal_name}_sinterrupt_list;')        
      else:
        assert 0, type
      #
      if has_readyin:
        line_list.append(f'input wire [{array_size_name}-1:0] {array_signal_name}_readyin_list;')
    #
    line_list.append('')
    line_list.append((template_dir/'mmio.02.vh').read_text())
    #
    line_list.append(f'wire [`BW_MMAP_OFFSET_{self.name.upper()}-1:0] paddr_offset = rpaddr;')
    line_list.append(f'wire [`BW_MMAP_OFFSET_{self.name.upper()}-1:0] addr_offset;')
    line_list.append(f'wire [BW_ADDR-1:0] addr_aligned;')
    line_list.append(f'wire [`BW_UNUSED_{self.name.upper()}-1:0] addr_unaligned;')
    line_list.append(f'wire [`BW_UNUSED_{self.name.upper()}-1:0] addr_unused = 0;')

    # signal
    for submodule_name, mmio_name, signal_name, array_signal_name, bitwidth, type, has_readyin, fifo_depth, default_value_type in mmio_list:
      line_list.append(f'reg signal_{signal_name}_re;')
      line_list.append(f'wire [BW_DATA-1:0] signal_{signal_name}_rdata;')
      line_list.append(f'reg signal_{signal_name}_we;')
      line_list.append(f'wire [BW_DATA-1:0] signal_{signal_name}_wdata;')
      line_list.append(f'wire signal_{signal_name}_readyin;')
      if type=='gpio':
        line_list.append(f'wire [`BW_GPIO_VALUE-1:0] signal_{signal_name}_user_pinout = 0;')

      if type=='simple':
        line_list.append(f'reg [{bitwidth}-1:0] reg_{signal_name};')
      elif type=='read_reg' or type=='write_reg':
        line_list.append(f'wire [{bitwidth}-1:0] reg_{signal_name};')
      elif type=='wfifo' or type=='rfifo':
        line_list.append(f'wire fifo_{signal_name}_clear;')
        line_list.append(f'wire fifo_{signal_name}_wready;')
        line_list.append(f'wire fifo_{signal_name}_wfull;')
        line_list.append(f'wire fifo_{signal_name}_wrequest;')
        line_list.append(f'wire [{bitwidth}-1:0] fifo_{signal_name}_wdata;')
        line_list.append(f'wire [BW_DATA-1:0] fifo_{signal_name}_wnum;')
        line_list.append(f'wire fifo_{signal_name}_rready;')
        line_list.append(f'wire fifo_{signal_name}_rempty;')
        line_list.append(f'wire fifo_{signal_name}_rrequest;')
        line_list.append(f'wire [{bitwidth}-1:0] fifo_{signal_name}_rdata;')
        line_list.append(f'wire [BW_DATA-1:0] fifo_{signal_name}_rnum;')
        
    #
    line_list.append('')
    line_list.append((template_dir/'mmio.03.vh').read_text())
    #
    for submodule_name, mmio_name, signal_name, array_signal_name, bitwidth, type, has_readyin, fifo_depth, default_value_type in mmio_list:
      line_list.append(f'assign signal_{signal_name}_wdata = $unsigned(rpwdata);')    
    #
    line_list.append('')
    line_list.append('always@(*)');
    line_list.append('begin');
    line_list.append('\trpslverr = 0;');
    line_list.append('\tman_rprdata = 0;');
    line_list.append('\trpready = 1;');

    for submodule_name, mmio_name, signal_name, array_signal_name, bitwidth, type, has_readyin, fifo_depth, default_value_type in mmio_list:
      line_list.append('')
      line_list.append(f'\tsignal_{signal_name}_re = 0;')
      line_list.append(f'\tsignal_{signal_name}_we = 0;')
      #line_list.append(f'output reg [{bitwidth}-1:0] signal_{signal_name}_wdata;')
    line_list.append('')
    line_list.append('\tif(rpsel==1\'b 1)');
    line_list.append('\tbegin');
    line_list.append('\t\tcase(addr_offset)');
    for submodule_name, mmio_name, signal_name, array_signal_name, bitwidth, type, has_readyin, fifo_depth, default_value_type in mmio_list:
      if type=='fake':
        continue
      mmap_define_name = f'MMAP_OFFSET_{signal_name.upper()}'
      line_list.append(f'\t\t\t`{mmap_define_name}:');
      line_list.append('\t\t\tbegin');
      line_list.append(f'\t\t\t\tsignal_{signal_name}_re = read_request_from_bus;')
      line_list.append(f'\t\t\t\tsignal_{signal_name}_we = write_request_from_bus;')
      line_list.append(f'\t\t\t\tman_rprdata = $unsigned(signal_{signal_name}_rdata);')
      line_list.append(f'\t\t\t\trpready = signal_{signal_name}_readyin;')
      line_list.append('\t\t\tend');
    line_list.append('\t\t\tdefault:');
    line_list.append('\t\t\t\trpslverr = 1;');
    line_list.append('\t\tendcase');
    line_list.append('\tend');
    line_list.append('end');
    line_list.append('')
    #
    for submodule_name, mmio_name, signal_name, array_signal_name, bitwidth, type, has_readyin, fifo_depth, default_value_type in mmio_list:
      if type=='simple':
        line_list.append(f'// {signal_name}')
        if default_value_type=='define':
          if array_signal_name:
            default_value = f'`{array_signal_name}_default_value'.upper()
          else:
            default_value = f'`{signal_name}_default_value'.upper()
        elif default_value_type=='para':
          default_value = f'{mmio_name}_default_value'.upper()
        else:
          assert 0, default_value_type
        line_list.append('always@(posedge clk, negedge rstnn)');
        line_list.append('begin');
        line_list.append('\tif(rstnn==0)');
        line_list.append(f'\t\treg_{signal_name} <= {default_value};');
        line_list.append(f'\telse if (signal_{signal_name}_we==1\'b 1)');
        line_list.append(f'\t\treg_{signal_name} <= signal_{signal_name}_wdata;');
        line_list.append('end');
        line_list.append(f'assign signal_{signal_name}_rdata = reg_{signal_name};');
      elif type=='read_reg':
        line_list.append(f'// {signal_name}')
        if default_value_type=='define':
          if array_signal_name:
            default_value = f'`{array_signal_name}_default_value'.upper()
          else:
            default_value = f'`{signal_name}_default_value'.upper()
        elif default_value_type=='para':
          default_value = f'{mmio_name}_default_value'.upper()
        else:
          assert 0, default_value_type
        line_list.append('ERVP_MMIO_WIDE_READ');
        line_list.append('#(');
        assign_list = []
        assign_list.append(('BW_MMIO','BW_DATA'))
        assign_list.append(('BW_WIDE_DATA',bitwidth))
        line_list.append(',\n'.join([f'\t.{x}({y})' for x, y in assign_list]))
        line_list.append(')');
        line_list.append(f'i_{signal_name}');
        line_list.append('(');
        assign_list = []
        assign_list.append(('clk','clk'))
        assign_list.append(('rstnn','rstnn'))
        assign_list.append(('clear',f'1\'b 0'))
        assign_list.append(('enable',f'1\'b 1'))
        assign_list.append(('mmio_re',f'signal_{signal_name}_re'))
        assign_list.append(('mmio_rdata',f'signal_{signal_name}_rdata'))
        assign_list.append(('wide_data_in',f'reg_{signal_name}'))
        line_list.append(',\n'.join([f'\t.{x}({y})' for x, y in assign_list]))
        line_list.append(');');
      elif type=='write_reg':
        line_list.append(f'// {signal_name}')
        if default_value_type=='define':
          if array_signal_name:
            default_value = f'`{array_signal_name}_default_value'.upper()
          else:
            default_value = f'`{signal_name}_default_value'.upper()
        elif default_value_type=='para':
          default_value = f'{mmio_name}_default_value'.upper()
        else:
          assert 0, default_value_type
        line_list.append('ERVP_MMIO_WIDE_REG');
        line_list.append('#(');
        assign_list = []
        assign_list.append(('BW_MMIO','BW_DATA'))
        assign_list.append(('BW_WIDE_DATA',bitwidth))
        assign_list.append(('DEFAULT_VALUE',default_value))
        line_list.append(',\n'.join([f'\t.{x}({y})' for x, y in assign_list]))
        line_list.append(')');
        line_list.append(f'i_{signal_name}');
        line_list.append('(');
        assign_list = []
        assign_list.append(('clk','clk'))
        assign_list.append(('rstnn','rstnn'))
        assign_list.append(('clear',f'1\'b 0'))
        assign_list.append(('enable',f'1\'b 1'))
        assign_list.append(('mmio_re',f'signal_{signal_name}_re'))
        assign_list.append(('mmio_rdata',f'signal_{signal_name}_rdata'))
        assign_list.append(('mmio_we',f'signal_{signal_name}_we'))
        assign_list.append(('mmio_wdata',f'signal_{signal_name}_wdata'))
        assign_list.append(('wide_data_out',f'reg_{signal_name}'))
        line_list.append(',\n'.join([f'\t.{x}({y})' for x, y in assign_list]))
        line_list.append(');');
      elif type=='gpio':
        assert not array_signal_name, array_signal_name
        line_list.append(f'// {signal_name}')
        line_list.append('ERVP_GPIO');
        line_list.append('#(');
        assign_list = []
        assign_list.append(('BW_DATA','BW_DATA'))
        assign_list.append(('BW_GPIO',bitwidth))
        line_list.append(',\n'.join([f'\t.{x}({y})' for x, y in assign_list]))
        line_list.append(')');
        line_list.append(f'i_gpio_{mmio_name}');
        line_list.append('(');
        assign_list = []
        assign_list.append(('clk','clk'))
        assign_list.append(('rstnn','rstnn'))
        assign_list.append(('rwenable',f'signal_{signal_name}_we'))
        assign_list.append(('rwdata',f'signal_{signal_name}_wdata'))
        assign_list.append(('rrenable',f'signal_{signal_name}_re'))
        assign_list.append(('rrdata',f'signal_{signal_name}_rdata'))
        assign_list.append(('ruser_pinout',f'signal_{signal_name}_user_pinout'))
        assign_list.append(('ruser_pinin',''))
        assign_list.append(('rinterrupt',f'{signal_name}_sinterrupt'))
        assign_list.append(('tick_gpio','tick_gpio'))
        assign_list.append(('gpio_soe',f'{signal_name}_soe'))
        assign_list.append(('gpio_soval',f'{signal_name}_soval'))
        assign_list.append(('gpio_sival',f'{signal_name}_sival'))
        line_list.append(',\n'.join([f'\t.{x}({y})' for x, y in assign_list]))
        line_list.append(');');
      elif type=='wfifo' or type=='rfifo':
        assert not array_signal_name, array_signal_name
        line_list.append(f'// {signal_name}')
        line_list.append('ERVP_FIFO');
        line_list.append('#(');
        assign_list = []
        assign_list.append(('BW_DATA',bitwidth))
        assign_list.append(('DEPTH',fifo_depth))
        assign_list.append(('BW_NUM_DATA','BW_DATA'))
        line_list.append(',\n'.join([f'\t.{x}({y})' for x, y in assign_list]))
        line_list.append(')');
        line_list.append(f'i_fifo_{mmio_name}');
        line_list.append('(');
        assign_list = []
        assign_list.append(('clk','clk'))
        assign_list.append(('rstnn','rstnn'))
        assign_list.append(('enable',f'1\'b 1'))
        assign_list.append(('clear',f'fifo_{signal_name}_clear'))
        assign_list.append(('wready',f'fifo_{signal_name}_wready'))
        assign_list.append(('wfull',f'fifo_{signal_name}_wfull'))
        assign_list.append(('wrequest',f'fifo_{signal_name}_wrequest'))
        assign_list.append(('wdata',f'fifo_{signal_name}_wdata'))
        assign_list.append(('wnum',f'fifo_{signal_name}_wnum'))
        assign_list.append(('rready',f'fifo_{signal_name}_rready'))
        assign_list.append(('rempty',f'fifo_{signal_name}_rempty'))
        assign_list.append(('rrequest',f'fifo_{signal_name}_rrequest'))
        assign_list.append(('rdata',f'fifo_{signal_name}_rdata'))
        assign_list.append(('rnum',f'fifo_{signal_name}_rnum'))
        line_list.append(',\n'.join([f'\t.{x}({y})' for x, y in assign_list]))
        line_list.append(');');

        assign_list = []
        assign_list.append((f'fifo_{signal_name}_clear',f'{signal_name}_clear'))
        assign_list.append((f'{signal_name}_wready',f'fifo_{signal_name}_wready'))
        assign_list.append((f'{signal_name}_wfull',f'fifo_{signal_name}_wfull'))
        assign_list.append((f'{signal_name}_wnum',f'fifo_{signal_name}_wnum'))
        assign_list.append((f'{signal_name}_rready',f'fifo_{signal_name}_rready'))
        assign_list.append((f'{signal_name}_rempty',f'fifo_{signal_name}_rempty'))
        assign_list.append((f'{signal_name}_rnum',f'fifo_{signal_name}_rnum'))

        if type=='wfifo':
          assign_list.append((f'fifo_{signal_name}_wrequest',f'signal_{signal_name}_we'))
          assign_list.append((f'fifo_{signal_name}_wdata',f'signal_{signal_name}_wdata'))
          assign_list.append((f'signal_{signal_name}_rdata',f'fifo_{signal_name}_wnum'))
          assign_list.append((f'fifo_{signal_name}_rrequest',f'{signal_name}_rrequest'))
          assign_list.append((f'{signal_name}_rdata',f'fifo_{signal_name}_rdata'))
        elif type=='rfifo':
          assign_list.append((f'fifo_{signal_name}_wrequest',f'{signal_name}_wrequest'))
          assign_list.append((f'fifo_{signal_name}_wdata',f'{signal_name}_wdata'))
          assign_list.append((f'fifo_{signal_name}_rrequest',f'signal_{signal_name}_re'))
          assign_list.append((f'signal_{signal_name}_rdata',f'fifo_{signal_name}_rdata'))
          assign_list.append((f'signal_{signal_name}_valid_num_rdata',f'fifo_{signal_name}_rnum'))
        line_list.append('\n'.join([f'assign {x} = {y};' for x, y in assign_list]))
      elif type=='rfifo_valid_num':
        pass

    # port assign
    for submodule_name, mmio_name, signal_name, bitwidth, type, has_readyin, fifo_depth, default_value_type in non_group_mmio_list:
      line_list.append(f'// {signal_name}')
      if type=='simple' or type=='write_reg':
        line_list.append(f'assign {signal_name} = reg_{signal_name};')
      elif type=='read_reg':
        line_list.append(f'assign reg_{signal_name} = {signal_name};')
      elif type=='general':
        line_list.append(f'assign {signal_name}_re = signal_{signal_name}_re;')
        line_list.append(f'assign signal_{signal_name}_rdata = {signal_name}_rdata;')
        line_list.append(f'assign {signal_name}_we = signal_{signal_name}_we;')
        line_list.append(f'assign {signal_name}_wdata = signal_{signal_name}_wdata;')
      elif type=='read':
        line_list.append(f'assign {signal_name}_re = signal_{signal_name}_re;')
        line_list.append(f'assign signal_{signal_name}_rdata = {signal_name}_rdata;')
      elif type=='write':
        line_list.append(f'assign signal_{signal_name}_rdata = 0;')
        line_list.append(f'assign {signal_name}_we = signal_{signal_name}_we;')
        line_list.append(f'assign {signal_name}_wdata = signal_{signal_name}_wdata;')
      elif type=='read_cmd':
        line_list.append(f'assign {signal_name}_{type} = signal_{signal_name}_re;')
        line_list.append(f'assign signal_{signal_name}_rdata = 0;')
      elif type=='write_cmd':
        line_list.append(f'assign {signal_name}_{type} = signal_{signal_name}_we;')
        line_list.append(f'assign signal_{signal_name}_rdata = 0;')
      elif type=='onehot_cmd':
        line_list.append(f'assign {signal_name}_{type} = signal_{signal_name}_we? signal_{signal_name}_wdata : 0;')
        line_list.append(f'assign signal_{signal_name}_rdata = 0;')
      if has_readyin:
        line_list.append(f'assign signal_{signal_name}_readyin = {signal_name}_readyin;')
      else:
        line_list.append(f'assign signal_{signal_name}_readyin = 1;')
    # port assign
    for array_signal_name, (submodule_name, array_name, bitwidth, type, has_readyin, fifo_depth, default_value_type, element_list) in group_mmio_dict.items():
      array_size = len(element_list)
      if type=='fake':
        continue
      elif type=='simple' or type=='read_reg' or type=='write_reg':
        for element_name in element_list:
          line_list.append('')
          assert element_name.startswith(array_signal_name), (element_name,array_signal_name)
          index = element_name[len(array_signal_name):]
          line_list.append(f'assign {array_signal_name}_list[({bitwidth})*(({index})+1)-1-:{bitwidth}] = reg_{element_name};')
      elif type=='general':
        for element_name in element_list:
          assert element_name[:len(array_signal_name)]==array_signal_name, element_name
          index = element_name[len(array_signal_name):]
          line_list.append('')
          line_list.append(f'assign {array_signal_name}_re_list[{index}] = signal_{element_name}_re;')
          line_list.append(f'assign signal_{element_name}_rdata = {array_signal_name}_rdata_list[({bitwidth})*(({index})+1)-1-:{bitwidth}];')
          line_list.append(f'assign {array_signal_name}_we_list[{index}] = signal_{element_name}_we;')
          line_list.append(f'assign {array_signal_name}_wdata_list[({bitwidth})*(({index})+1)-1-:{bitwidth}] = signal_{element_name}_wdata;')
      elif type=='read':
        for element_name in element_list:
          assert element_name[:len(array_signal_name)]==array_signal_name, element_name
          index = element_name[len(array_signal_name):]
          line_list.append('')
          line_list.append(f'assign {array_signal_name}_re_list[{index}] = signal_{element_name}_re;')
          line_list.append(f'assign signal_{element_name}_rdata = {array_signal_name}_rdata_list[({bitwidth})*(({index})+1)-1-:{bitwidth}];')
      elif type=='write':
        for element_name in element_list:
          assert element_name[:len(array_signal_name)]==array_signal_name, element_name
          index = element_name[len(array_signal_name):]
          line_list.append('')
          line_list.append(f'assign signal_{element_name}_rdata = 0;')
          line_list.append(f'assign {array_signal_name}_we_list[{index}] = signal_{element_name}_we;')
          line_list.append(f'assign {array_signal_name}_wdata_list[({bitwidth})*(({index})+1)-1-:{bitwidth}] = signal_{element_name}_wdata;')
      elif type=='read_cmd' or type=='write_cmd' or type=='onehot_cmd':
        for element_name in element_list:
          assert element_name[:len(array_signal_name)]==array_signal_name, element_name
          index = element_name[len(array_signal_name):]
          line_list.append('')
          line_list.append(f'assign {array_signal_name}_{type}_list[({bitwidth})*(({index})+1)-1-:{bitwidth}] = signal_{element_name}_{type};')
      for element_name in element_list:
        assert element_name[:len(array_signal_name)]==array_signal_name, element_name
        index = element_name[len(array_signal_name):]
        line_list.append('')
        if has_readyin:
          assert 0
          line_list.append(f'assign signal_{element_name}_readyin = {array_signal_name}_readyin_list[({bitwidth})*(({index})+1)-1-:{bitwidth}];')
        else:
          line_list.append(f'assign signal_{element_name}_readyin = 1;')
    #
    line_list.append('')
    line_list.append((template_dir/'mmio.04.vh').read_text())
    #
    output_file = output_directory / f'{self.name}.v'
    output_file.write_text('\n'.join(line_list))

if __name__ == '__main__':
  # parse argument
  parser = argparse.ArgumentParser(description='Synthesizing MMIO components')
  parser.add_argument('-input', '-i', help='an input xml file')
  parser.add_argument('-output', '-o', help='output directory')
  parser.add_argument('-operation', '-op', help='operation')
  parser.add_argument('-submodule', '-s', nargs='+', help='target submodule')
  args = parser.parse_args()

  assert args.input
  assert args.output

  input_xml_file = Path(args.input)
  assert input_xml_file.is_file(), input_xml_file
  output_directory = Path(args.output).resolve()
  if not output_directory.is_dir():
    output_directory.mkdir(parents=True)

  xml_tree = XmlTree.parse(input_xml_file)
  xml_root = xml_tree.getroot()
  assert len(xml_root)>=1, 'No available info'
  assert xml_root.tag=='rvx', xml_root.tag

  for def_ip in xml_root.findall('def_ip'):
    ip_memorymap_offset = IpMemorymapOffset(def_ip)
    if 's' in args.operation:
      ip_memorymap_offset.synthesize()
      ip_memorymap_offset.export(output_directory)
    if 'd' in args.operation:
      ip_memorymap_offset.export_define('r' in args.operation, output_directory)
    if 'm' in args.operation:
      ip_memorymap_offset.export_memorymap('r' in args.operation, output_directory)
    if 'v' in args.operation:
      if args.submodule==None:
        submodule_list = list(ip_memorymap_offset.submodule_dict.keys())
      elif len(args.submodule)==1 and args.submodule[0]=='all':
        submodule_list = list(ip_memorymap_offset.submodule_dict.keys())
      else:
        submodule_list = args.submodule
      ip_memorymap_offset.export_verilog('r' in args.operation, submodule_list, output_directory)
