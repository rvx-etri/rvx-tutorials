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
# 2021-02-24
# Kyuseung Han (han@etri.re.kr)
# ****************************************************************************
# ****************************************************************************

import os
import argparse
import random
from pathlib import Path

from os_util import *

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Makefile Helper')
  parser.add_argument('-cmd', '-c', help='command')
  parser.add_argument('-parameter', '-p', nargs='+', help='parameter list')
  args = parser.parse_args()

  if args.cmd=='toupper':
    result = ' '.join([x.upper() for x in args.parameter])
    print(result)
  elif args.cmd=='pyinstaller_key':
    candidate_list = ['a','b','c','d','e','f']
    candidate_list += range(0,10)
    result = ''
    for i in range(0,16):
      result += str(random.choice(candidate_list))
    print(result)
  elif args.cmd=='distclean':
    cwd = Path('.').resolve()
    original_set = frozenset([str(cwd/x) for x in args.parameter])
    for element in cwd.iterdir():
      if str(element) not in original_set:
        if element.is_file():
          remove_file(element)
        elif element.is_dir():
          remove_directory(element)
  else:
    assert 0, args.cmd  
