from pathlib import *
import math
import copy
import os
import re
import ast
from enum import Enum
from typing import NamedTuple

from npx_neuron_type import *

class NpxCfgSection(dict):
  def __init__(self, name:str=None):
    self.name = name
    
  @staticmethod
  def convert_value(text):
    try:
      return ast.literal_eval(text)
    except (ValueError, SyntaxError):
      return text
  
  def add_option(self, line:str):
    key, value = line.strip().split('=')
    self[key] = NpxCfgSection.convert_value(value)
  
  def __str__(self) -> str:
    line_list = []
    line_list.append(f'[{self.name}]')
    for key, value in self.items():
      line_list.append(f'{key}={value}')
    return '\n'.join(line_list)

class SignedType(Enum):
  ERROR = -1
  UNSIGNED = 0
  SIGNED = 1
  
  def __mul__ (self, other):
    assert(self!=SignedType.ERROR)
    assert(other!=SignedType.ERROR)
    if self==SignedType.SIGNED or other==SignedType.SIGNED:
      result = SignedType.SIGNED
    else:
      result = SignedType.UNSIGNED
    return result
  
  @property
  def is_signed(self):
    return self.value
  
class NumberType(Enum):
  ERROR = -1
  CONTI = 0
  DISCR = 1
  
  def __mul__ (self, other):
    assert(self!=NumberType.ERROR)
    assert(other!=NumberType.ERROR)
    if self==NumberType.CONTI or other==NumberType.CONTI:
      result = NumberType.CONTI
    else:
      result = NumberType.DISCR
    return result
  
  @property
  def is_quantized(self):
    return self.value
  
class DataType(NamedTuple):
  signedtype: SignedType
  numbertype: NumberType
  maxvalue: int
  
  def __str__(self):
    return str((self.name, self.numbertype.name, self.maxvalue))
  
  def __mul__ (self, other):
    if type(other)==DataType:
      other_datatype = other
    elif type(other)==int:
      other_datatype = DataType(SignedType.SIGNED, NumberType.DISCR, other)
    elif type(other)==float:
      other_datatype = DataType(SignedType.SIGNED, NumberType.CONTI, other)
    else:
      assert 0
    signedtype = self.signedtype*other_datatype.signedtype
    numbertype = self.numbertype*other_datatype.numbertype
    maxvalue = self.maxvalue*other_datatype.maxvalue
    return DataType(signedtype, numbertype,maxvalue)
  
  @property
  def bitwidth(self):
    return max(1, math.ceil(math.log2(self.maxvalue)))

  @staticmethod
  def convert_neuron_type(neuron_type:NpxNeuronType):
    signedtype = SignedType.SIGNED if neuron_type.is_signed_weight else SignedType.UNSIGNED
    numbertype = NumberType.DISCR if neuron_type.is_quantized else NumberType.CONTI
    return DataType(signedtype, numbertype, neuron_type.qscale)
  
class LayerIoInfo(NamedTuple):
  scale: int
  datatype: DataType
  channels: int
  size: tuple
  
  def __str__(self):
    return str((self.scale, self.datatype.signedtype.name, self.datatype.numbertype.name, self.datatype.maxvalue, self.datatype.bitwidth, self.channels, self.size))
  
  @property
  def dims(self):
    size = len(self.size)
    for each_size in self.size:
      if each_size==1:
        size -= 1
    return size
    
class NpxCfgParser():  
  def __init__(self, path:Path=None):
    super().__init__()
    self.find_section_name = re.compile('\[([^]]+)\]').findall
    self.train_info = None
    self.global_info = None
    self.preprocess_info = None
    self.layer_info_list = []
    if path and path.exists():
      assert path.is_file(), path
      self.parse_file(path)
  
  def parse_file(self, path:Path):
    assert path.is_file(), path
    line_list = path.read_text().split('\n')

    current_section = None
    for line in line_list:
      if line=='':
        continue
      if '[' in line and ']' in line:
        section_name = self.find_section_name(line)[0]
        current_section = NpxCfgSection(section_name)
        if section_name=='train':
          self.train_info = current_section
        elif section_name=='preprocess':
          self.preprocess_info = current_section
        elif section_name=='global':
          self.global_info = current_section
        else:
          self.layer_info_list.append(current_section)
      elif line.startswith('\0') or line.startswith('#') or line.startswith(';') or line.startswith('\n'):
          pass
      else:
        current_section.add_option(line)
  
  def __str__(self) -> str:
    line_list = []
    if self.preprocess_info:
      line_list.append(str(self.preprocess_info))
    if self.train_info:
      line_list.append(str(self.train_info))
    if self.global_info:
      line_list.append(str(self.global_info))
    for section in self.layer_info_list:
      line_list.append(str(section))
    return '\n'.join(line_list)
  
  def __repr__(self) -> str:
    return str(self)+'\n' 
  
  def write_file(self, path:Path):
    path.write_text(str(self))
    
  def add_train_info(self):
    assert self.train_info
    self.train_info = NpxCfgSection('train')
    return self.train_info
  
  def add_preprocess_info(self):
    assert self.preprocess_info
    self.preprocess_info = NpxCfgSection('preprocess')
    return self.preprocess_info
  
  def add_network(self, section_name:str):
    self.layer_info_list.append(NpxCfgSection(section_name))
    return self.layer_info_list[-1]
  
  def generate_preprocess_output_info(self):
    assert self.preprocess_info
    assert self.train_info
    if self.preprocess_info['input']=='mnist_dataset':
      if self.preprocess_info['step_generation']=='direct':
        scale = 1
        datatype = DataType(SignedType.UNSIGNED, NumberType.DISCR, scale)
      else:
        assert 0
    elif self.preprocess_info['input']=='dvsgesture_dataset':
      scale = 1
      datatype = DataType(SignedType.UNSIGNED, NumberType.DISCR, scale)
    else:
      assert 0
    return LayerIoInfo(scale, datatype, self.train_info['input_channels'], self.train_info['input_size'])
  
  def elaborate_for_riscv(self):
    output_info = self.generate_preprocess_output_info()    
    for layer_info in self.layer_info_list:
      layer_info['input_info'] = output_info
      neuron_type = NpxNeuronType(layer_info['neuron_type'])
      weight_datatype =  DataType.convert_neuron_type(neuron_type)
      output_scale = layer_info['input_info'].scale
      output_datatype = layer_info['input_info'].datatype      
      if layer_info.name=='Conv2d':
        in_channels = layer_info['in_channels']
        assert layer_info['input_info'].dims==2, layer_info['input_info'].size
        assert layer_info['input_info'].channels==in_channels, in_channels
        output_datatype *= (layer_info['kernel_size']*layer_info['kernel_size'])
        output_datatype *= weight_datatype
        output_size = []
        for each_size in output_info.size:
          new_size = int((each_size + (2*layer_info['padding']) - (layer_info['kernel_size']-1)) / layer_info['stride'])
          output_size.append(new_size)
        out_channels = layer_info['out_channels']
        
        layer_info['weight_bitwidth'] = neuron_type.num_bits
        
      elif layer_info.name=='Linear':
        assert layer_info['input_info'].dims==1, layer_info['input_info'].size
        assert layer_info['in_features']==layer_info['input_info'].size[0]
        output_datatype *= layer_info['in_features']
        output_datatype *= weight_datatype
        out_channels = layer_info['input_info'].channels
        output_size = (layer_info['out_features'],1)
        
        layer_info['weight_bitwidth'] = neuron_type.num_bits
        
      elif layer_info.name=='AvgPool2d' or layer_info.name=='MaxPool2d':
        assert layer_info['input_info'].dims==2, layer_info['input_info'].size
        out_channels = layer_info['input_info'].channels
        output_size = []
        for each_size in layer_info['input_info'].size:
          new_size = int(each_size/layer_info['stride'])
          output_size.append(new_size)
      elif layer_info.name=='Leaky':
        output_scale = 1
        output_datatype = DataType(SignedType.UNSIGNED,NumberType.DISCR,1)
        out_channels = layer_info['input_info'].channels
        output_size = layer_info['input_info'].size
        
        del layer_info['learn_threshold']
        del layer_info['mapped_fvalue']
        
      elif layer_info.name=='Flatten':
        out_channels = 1
        output_size = layer_info['input_info'].channels
        for each_size in layer_info['input_info'].size:
          output_size *= each_size
        output_size = (output_size,1)
      else:
        assert 0, layer_info.name
      output_info = LayerIoInfo(output_scale, output_datatype, out_channels, tuple(output_size))
      layer_info['output_info'] = output_info
    
    for layer_info in self.layer_info_list:
      for io_tpye in ('in','out'):
        layer_info[f'{io_tpye}_is_signed'] = layer_info[f'{io_tpye}put_info'].datatype.signedtype.is_signed
        layer_info[f'{io_tpye}_is_quantized'] = layer_info[f'{io_tpye}put_info'].datatype.numbertype.is_quantized
        layer_info[f'{io_tpye}_maxvalue'] = layer_info[f'{io_tpye}put_info'].datatype.maxvalue
        layer_info[f'{io_tpye}_channels'] = layer_info[f'{io_tpye}put_info'].channels
        layer_info[f'{io_tpye}_dims'] = layer_info[f'{io_tpye}put_info'].dims
        layer_info[f'{io_tpye}_size'] = layer_info[f'{io_tpye}put_info'].size
      
      print(layer_info.name)
      print(layer_info['input_info'])
      print(layer_info['output_info'])
      
      del layer_info['input_info']
      del layer_info['output_info']
      del layer_info['neuron_type']
      
    self.preprocess_info = None
    self.global_info = None