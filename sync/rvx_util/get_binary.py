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
## 2020-03-17
## Kyuseung Han (han@etri.re.kr)
## ****************************************************************************
## ****************************************************************************

import os
import argparse
from pathlib import Path

from os_util import *

class RvxBinary():
  def __init__(self, target:str, info_dir:Path):
    assert target
    assert info_dir.is_dir()
    assert info_dir.is_absolute()

    if is_windows:
      self.info_file = info_dir / f'{target}_windows_info.txt'
      if not self.info_file.is_file():
        self.info_file = info_dir / f'{target}_all_info.txt'
    elif is_linux:
      if is_centos:
        self.info_file = info_dir / f'{target}_centos_info.txt'
      elif is_ubuntu:
        self.info_file = info_dir / f'{target}_ubuntu_info.txt'
      if not self.info_file.is_file():
        self.info_file = info_dir / f'{target}_linux_info.txt'
      if not self.info_file.is_file():
        self.info_file = info_dir / f'{target}_all_info.txt'
    assert self.info_file.is_file()

  @staticmethod
  def __get_info(info_file:Path):
    return info_file.read_text().split('\n')

  def download(self, output_dir:Path, removes_top_dir:bool=False):
    assert output_dir.is_absolute()
    new_link = RvxBinary.__get_info(self.info_file)[0]
    current_link = ''
    current_info_file = output_dir / self.info_file.name
    if current_info_file.is_file():
      current_link = RvxBinary.__get_info(current_info_file)[0]

    if new_link!=current_link:
      remove_directory(output_dir)
      output_dir.mkdir(parents=True)
      output_file = output_dir / new_link.split('/')[-1]
      download_url(new_link, output_file=output_file)
      extract_file(output_file)
      copy_file(self.info_file, current_info_file)
      if removes_top_dir:
        extracted_dir = output_dir / output_file
        while 1:
          extracted_dir_candidate = extracted_dir.parent / extracted_dir.stem
          assert extracted_dir!=extracted_dir_candidate, 'top dir does NOT exist'
          extracted_dir = extracted_dir_candidate
          if extracted_dir.is_dir():
            break
        for object in extracted_dir.iterdir():
          if object.is_file():
            move_files(object, output_dir/object.name)
          elif object.is_dir():
            move_directory(object, output_dir/object.name)
          else:
            assert 0, object
        #remove_directory(extracted_dir)
      print('Downloaded')
    else:
      print('No update')

if __name__ =='__main__':
  parser = argparse.ArgumentParser(description='Get Binary Files')
  parser.add_argument('-target', '-t', help='target name')
  parser.add_argument('-info', '-i', help='info directory')
  parser.add_argument('-output', '-o', help='output directory')
  parser.add_argument('--removes_top_dir', action="store_true", help='')
  
  args = parser.parse_args()  
  info_dir = Path(args.info).resolve()
  output_dir = Path(args.output).resolve()
  binary = RvxBinary(args.target, info_dir)
  binary.download(output_dir, args.removes_top_dir)
