import argparse
import re
import os
from pathlib import Path

import import_util
from os_util import *
from generate_git_info import *

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Server Info Generation')
  parser.add_argument('-cmd', help='command')
  parser.add_argument('-common', help='common info file')
  parser.add_argument('-output', '-o', help='output file')
  parser.add_argument('-history', help='history file')
  
  args = parser.parse_args()
  
  assert args.cmd
  
  assert args.common
  common_info_path = Path(args.common)
  assert common_info_path.is_file(), common_info_path
  
  assert args.output
  private_info_path = Path(args.output)
  assert private_info_path.is_absolute(), private_info_path
  assert private_info_path.parent.is_dir(), private_info_path
  
  assert args.history
  history_path = Path(args.history)
  
  if not args.cmd:
    assert 0
  elif args.cmd=='check':
    common_info_list = [ x.split(':') if ':' in x else ['',x] for x in common_info_path.read_text().split('\n') ]
    common_info_name_list = ('rvx_version','rvx_server_manager','rvx_platform_example','rvx_binary','rvx_install')
    for i, common_info_name in enumerate(common_info_name_list):
      if len(common_info_list) > i and (not common_info_list[i][0]):
        common_info_list[i][0] = common_info_name
      
    private_info_list = []
    for common_info in common_info_list:
      if common_info[0]:
        private_info_list.append(common_info)
    
    if history_path.is_file():
      private_info_list.append(['synced_before','true'])
    else:
      private_info_list.append(['synced_before','false'])
    
    line_list = [ ':'.join(x) for x in private_info_list ]
    private_info_path.write_text('\n'.join(line_list))
  else:
    assert 0, args.cmd
    
  
  
