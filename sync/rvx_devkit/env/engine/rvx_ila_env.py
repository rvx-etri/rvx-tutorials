## ****************************************************************************
## ****************************************************************************
## Copyright SoC Design Research Group, All rights reserved.    
## Electronics and Telecommunications Research Institute (ETRI)
##
## THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE 
## WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS 
## TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE 
## REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL 
## SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE, 
## IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE 
## COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.
## ****************************************************************************
## 2019-10-11
## Kyuseung Han (han@etri.re.kr)
## ****************************************************************************
## ****************************************************************************

from xml.etree.ElementTree import *

class RvxIla():
  index = 1
  default_depth = 1024
  def __init__(self, ila_xml):
    self.index = RvxIla.index
    self.name = f'ila{self.index}'
    RvxIla.index += 1

    clock_list = ila_xml.findall('clock')
    assert len(clock_list)==1, len(clock_list)
    self.clock = clock_list[0].text
    self.signal_list = []
    for signal_xml in ila_xml.findall('signal'):
      name = signal_xml.find('name').text
      width = signal_xml.find('width').text
      value = signal_xml.find('value').text
      self.signal_list.append((name, width, value))
  
  def get_instance_name(self):
    return f'I_{self.name.upper()}'

  def get_filename(self):
    return f'{self.name}.vh'

  def _get_modified_signal_name(self, signal_name):
    return f'{self.name}_{signal_name}'

  def _generate_signal(self):
    assign_list = []
    assign_list.append((self._get_modified_signal_name('clk'), self.clock))
    for name, width, value in self.signal_list:
      assign_list.append((f'[{width}-1:0] {self._get_modified_signal_name(name)}',value))

    contents = '\n'.join([ f'wire {name} = {value};' for name, value in assign_list])
    return contents

  def _generate_instance(self):
    line_list = []
    line_list.append(self.name)
    line_list.append(self.get_instance_name())
    line_list.append('(')
    port_list = []
    port_list.append('\t.clk({0})'.format(self._get_modified_signal_name('clk')))
    for index, (name, width, value) in enumerate(self.signal_list):
      port_list.append(f'\t.probe{index}({self._get_modified_signal_name(name)})')
    line_list.append(',\n'.join(port_list))
    line_list.append(');')
    return '\n'.join(line_list)

  def generate_verilog(self):
    contents = '\n'
    contents += self._generate_signal()
    contents += '\n\n'
    contents += self._generate_instance()
    return contents

  def generate_tcl(self, ila_result_path, fpga_part_name):
    ila_default_depth = 1024
    ila_file_path = ila_result_path / self.name / f'{self.name}.xci'
    ila_stub_file_name = ila_result_path / self.name / f'{self.name}_stub.v'

    line_list = []
    line_list.append('\n\n')
    line_list.append(f'if {{[file exist {ila_file_path}]==0}} {{')
    line_list.append('')
    line_list.append(f'\tif [info exists ILA{self.index}_DEPTH]==0 {{')
    line_list.append(f'\t\tset ILA{self.index}_DEPTH {RvxIla.default_depth}')
    line_list.append('\t}')
    line_list.append('')
    line_list.append(f'\tcreate_ip -force -name ila -vendor xilinx.com -library ip -version 6.2 -module_name {self.name} -dir {ila_result_path}')
    property_list = [
      ( 'C_TRIGIN_EN', 'false' ),
      ( 'C_TRIGOUT_EN', 'false' ),
      ( 'C_INPUT_PIPE_STAGES', '1' ),
      ( 'C_EN_STRG_QUAL', 'false' ),
      ( 'C_ADV_TRIGGER', 'true' ),
      ( 'ALL_PROBE_SAME_MU', 'true' ),
      ( 'ALL_PROBE_SAME_MU_CNT', '4' )
    ]
    property_list.append(('C_DATA_DEPTH',f'$ILA{self.index}_DEPTH'))
    property_list.append(('C_NUM_OF_PROBES',len(self.signal_list)))
    for index, (name, width, value) in enumerate(self.signal_list):
      property_list.append((f'C_PROBE{index}_WIDTH', width))    
    line_list.append('\n'.join([f'\tset_property CONFIG.{x} {y} [get_ips {self.name}]' for x, y in property_list]))
    
    line_list.append('')    
    #line_list.append(f'\tgenerate_target {{instantiation_template}} [get_files {self.name}]')
    
    line_list.append('} else {')
    line_list.append('\tputs [format "\\[INFO\\] ILA{0} is already generated"]'.format(self.index))
    line_list.append(f'\tread_ip {ila_file_path}')
    line_list.append('}')

    line_list.append('')
    line_list.append(f'if {{[file exist {ila_stub_file_name}]==0}} {{')
    line_list.append(f'\tsynth_ip -quiet {ila_file_path}')
    line_list.append('} else {')
    line_list.append(f'\tread_verilog {ila_stub_file_name}')
    line_list.append('}')

    return '\n'.join(line_list)
