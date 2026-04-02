# ****************************************************************************
# ****************************************************************************
# Copyright SoC Design Research Group, All rights reserved.
# Electronics and Telecommunications Research Institute (ETRI)
# 
# THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE
# WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS
# TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE
# REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL
# SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE,
# IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE
# COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.
# ****************************************************************************
# 2019-06-14
# Kyuseung Han (han@etri.re.kr)
# ****************************************************************************
# ****************************************************************************

import argparse
import os
import re
from pathlib import Path

from os_util import *
from re_util import *
from config_file_manager import *

def rename_smart(base_path:Path, before:str, after:str):
  assert base_path.is_dir(), base_path
  assert base_path.is_absolute(), base_path

  conv_info = ((before.lower(),after.lower()), \
               (before.upper(),after.upper()), \
               (before.capitalize(),after.capitalize()), \
               (before.title(),after.title()))

  conv_dict = {}
  for before_word, after_word in conv_info:
    assert before_word
    conv_dict[before_word] = after_word

  reexp_conv = memorize(r'|'.join(conv_dict.keys()))
  re_conv = re.compile(reexp_conv,re.DOTALL)

  for path_object in base_path.glob('**/*'):
    if path_object.is_file():
      try:
        contents = path_object.read_text()
        if len(re_conv.findall(contents)) > 0:
          contents = re_conv.sub(lambda x: conv_dict[x.group()], contents)
          path_object.write_text(contents)
      except:
        pass

  for path_object in base_path.glob('**/*'):
    if path_object.is_file():
      original_name = path_object.name
      modified_name = re_conv.sub(lambda x: conv_dict[x.group()], original_name)
      if original_name!=modified_name:
        path_object.rename(path_object.parent/modified_name)

  def rename_target_dir_only(target_dir:Path, conv_dict:dict):
    new_path_name = re_conv.sub(lambda x: conv_dict[x.group()], target_dir.name)
    if target_dir.name==new_path_name:
      new_target_dir = target_dir
    else:
      new_target_dir = target_dir.parent / new_path_name
      if new_target_dir.is_dir():
        remove_directory(new_target_dir)
      target_dir.rename(new_target_dir)
    return new_target_dir

  def rename_sub_dir(parent_dir:Path, conv_dict:dict):
    for path_object in parent_dir.iterdir():
      if path_object.is_dir():
        new_target_dir = rename_target_dir_only(path_object, conv_dict)
        rename_sub_dir(new_target_dir, conv_dict)

  rename_sub_dir(base_path, conv_dict)

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='start rename')
  parser.add_argument('-path', '-p', help='path')
  parser.add_argument('-before', '-b', help='before')
  parser.add_argument('-after', '-a', help='after')
  args = parser.parse_args()

  assert args.path
  assert args.before
  assert args.after

  base_path = Path(args.path).resolve()
  rename_smart(base_path, args.before, args.after)
