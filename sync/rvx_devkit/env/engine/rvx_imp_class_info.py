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
## 2019-04-15
## Kyuseung Han (han@etri.re.kr)
## ****************************************************************************
## ****************************************************************************

import xml.etree.ElementTree as ET
import argparse
import os

from pathlib import Path

class RvxImpClassInfo():
  def __init__(self):
    self.clear()

  def clear(self):
    self.imp_class_dict = {}
    self.file_dict = {}
    self.fpga_list = []
    self.unique_instance_list = []

  def import_xml(self, xml_path:Path): 
    assert xml_path.is_file(), xml_path
    xml_tree = ET.parse(str(xml_path))
    xml_root = xml_tree.getroot()
    assert xml_root.tag=='rvx', xml_root.tag
    assert len(xml_root)==1, len(xml_root)
    assert xml_root[0].tag=='imp_class_list', xml_root[0].tag
    for imp_class_xml in xml_root[0].findall('imp_class'):
      imp_class_info = RvxImpClassInfo.xml2dict(imp_class_xml)
      name = imp_class_info['name']
      type = imp_class_info['type']
      self.imp_class_dict[name] = imp_class_info
      self.file_dict[name] = xml_path
      if type=='fpga':
        self.fpga_list.append(name)
      unique_instance = imp_class_info.get('unique_instance')
      if unique_instance:
        assert name==type
        self.unique_instance_list.append((unique_instance,type,type))

  @staticmethod
  def xml2dict(xml_element):
    if len(xml_element):
      result = {}
      for xml_child in xml_element:
        result[xml_child.tag] = RvxImpClassInfo.xml2dict(xml_child)
    else:
      result = xml_element.text
    return result

  def get_imp_class_info(self, imp_name:str):
    return self.imp_class_dict.get(imp_name)

  @property
  def imp_class_list(self):
    return self.imp_class_dict.keys()

  def get_fpga_info(self, fpga_name:str):
    result = None
    if fpga_name in self.fpga_list:
      result = self.get_imp_class_info(fpga_name)
      if result:
        result = result.get('fpga_info')
    return result
