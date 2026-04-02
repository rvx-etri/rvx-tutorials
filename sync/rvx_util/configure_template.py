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
# 2020-03-11
# Kyuseung Han (han@etri.re.kr)
# ****************************************************************************
# ****************************************************************************

import argparse
import re

from os_util import *
from re_util import *

def configure_template_text(contents:str, conv_info=None):
  if not conv_info:
    config_line = ''
    conv_dict = {}
  elif type(conv_info)==dict:
    config_line = ' '.join([f'"{x}={y}"' for x, y in conv_info.items()])
    conv_dict = conv_info
  elif type(conv_info)==list or type(conv_info)==tuple:
    config_line = ' '.join([f'"{x}={y}"' for x, y in conv_info])
    conv_dict = {}
    for before_word, after_word in conv_info:
      assert before_word
      conv_dict[before_word] = after_word
  #
  contents = re.sub('\"\${TEMPLATE_CONFIG}\"', config_line, contents)
  #
  reexp_template_var = '\"\${' + memorize(reexp_identifier) + '}\"'
  re_template_var = re.compile(reexp_template_var)
  contents = re_template_var.sub(lambda x: conv_dict[x.group(1)], contents)
  return contents

def configure_template_file(input_file:Path, output_file:Path, conv_dict:dict={}):
  contents = input_file.read_text()
  contents = configure_template_text(contents, conv_dict)
  output_file.write_text(contents)

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Configure..')
  parser.add_argument('-input', '-i', help='input template file')
  parser.add_argument('-output', '-o', help='output file')
  parser.add_argument('-config', '-c', nargs='+', help='configure info')
  args = parser.parse_args()

  assert args.input
  assert args.output

  input_path = Path(args.input).absolute()
  output_path = Path(args.output).absolute()

  conv_dict = {}
  if args.config:
    for config in args.config:
      before_word, after_word = config.split('=')
      assert before_word
      conv_dict[before_word] = after_word
  configure_template_file(input_path, output_path, conv_dict)
