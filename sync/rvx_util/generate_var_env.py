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
# 2019-04-15
# Kyuseung Han (han@etri.re.kr)
# ****************************************************************************
# ****************************************************************************

import argparse
import os

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Configure..')
  parser.add_argument('-output', '-o', help='output file')
  parser.add_argument('-variable', '-v', nargs='+', help='variable name list')
  parser.add_argument('-lang', '-l', help='language')
  args = parser.parse_args()

  assert args.output
  assert args.variable
  assert args.lang

  line_list = []
  for assignment in args.variable:
    if '=' in assignment:
      variable, value = assignment.split('=')
    else:
      variable = assignment
      value = os.environ.get(variable)
    if value:
      value = os.path.normpath(value)
      value = '/'.join(value.split(os.sep))
      if args.lang=='tcl':
        line_list.append('set {0} {1}'.format(variable,value))
      elif args.lang=='makefile':
        line_list.append('{0}={1}'.format(variable,value))
      else:
        assert 0, args.lang
    else:
      pass
  
  output_directory = os.path.split(args.output)[0]
  if not os.path.isdir(output_directory):
    os.makedirs(output_directory)
  with open(args.output,'w') as f:
    f.write('\n'.join(line_list))
