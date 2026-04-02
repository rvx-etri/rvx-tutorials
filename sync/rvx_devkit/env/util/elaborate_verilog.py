import argparse
from pathlib import Path
from itertools import *

from rvx_verilog_re_util import *

class VerilogFileList():
  def __init__(self, target_file_list:tuple, include_file_list:tuple):
    self.name2text = {}
    self.target_set = frozenset([ x.name for x in target_file_list ])
    for file in chain(target_file_list,include_file_list):
      if file.name.endswith('.vf'):
        self.name2text[file.name] = [file, remove_comments(file.read_text())]
      elif file.name.endswith('.vb'):
        self.name2text[file.name] = [file, remove_comments(file.read_text())]
      elif file.name.endswith('.vh'):
        self.name2text[file.name] = [file, remove_comments(file.read_text())]
      else:
        self.name2text[file.name] = [file, file.read_text()]

  def elab_all(self):
    self.compelte_set = set()
    self.processing_set = set()
    for file_name in self.name2text.keys():
      self.__elab_all(file_name)
  
  def elab_include(self):
    self.compelte_set = set()
    self.processing_set = set()      
    for file_name in self.name2text.keys():
      if not file_name.endswith('.v'):
        self.__elab_all(file_name)

  def elab_body(self):
    self.elab_include()
    for file_name in self.name2text.keys():
      if file_name.endswith('.v'):
        self.__elab_body(file_name)

  def __elab_text(self, text:str):
    f = lambda x: '// {0} start\n{1}// {0} end\n'.format(x.group(1),self.name2text[x.group(1)][1]) if x.group(1) in self.name2text else x.group(0)
    modified_text = re_include.sub(f, text)
    return modified_text

  def __elab_all(self, target_file_name:str):
    if target_file_name in self.compelte_set:
      pass
    elif target_file_name not in self.name2text:
      pass
    else:
      assert target_file_name not in self.processing_set, target_file_name
      self.processing_set.add(target_file_name)
      for header_file_name in re_include.findall(self.name2text[target_file_name][1]):
        self.__elab_all(header_file_name)
      self.name2text[target_file_name][1] = self.__elab_text(self.name2text[target_file_name][1])
      self.processing_set.remove(target_file_name)
      self.compelte_set.add(target_file_name)

  def __elab_body(self, target_file_name:str):
    assert target_file_name in self.name2text, target_file_name
    module_text = self.name2text[target_file_name][1]
    module_name = target_file_name[:-len('.v')].upper()
    reexp_module_name = r'^(?:module|MODULE)' + reexp_blank_or_not + module_name
    re_module_name = re.compile(reexp_module_name,re.MULTILINE)
    splited_module_text = re_module_name.split(module_text)
    if len(splited_module_text)==2:
      module_body = splited_module_text[1]
      modified_module_body = self.__elab_text(module_body)
      modified_module_text = splited_module_text[0] + f'module {module_name}' + modified_module_body
      self.name2text[target_file_name][1] = modified_module_text

  def export_all(self):
    for file_name, (file, contencts) in self.name2text.items():
      if file_name in self.target_set:
        file.write_text(contencts)

  def export_module(self):
    for file_name, (file, contencts) in self.name2text.items():
      if file_name.endswith('.v') and file_name in self.target_set:
        file.write_text(contencts)

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Elaborating Verilog')
  parser.add_argument('-path', '-p', help='path')
  parser.add_argument('-include_list', '-i', type=str, nargs='*', help='include directory')
  parser.add_argument('-cmd', '-c', help='command')
  args = parser.parse_args()

  assert args.path
  target_path = Path(args.path).resolve()
  assert target_path.is_dir(), target_path

  target_file_list = tuple(target_path.glob('./**/*.v'))
  target_file_list += tuple(target_path.glob('./**/*.v?'))

  include_file_list = []
  if args.include_list:
    for include_dir in args.include_list:
      include_path = Path(include_dir).resolve()
      assert include_path.is_dir(), include_path
      include_file_list += tuple(include_path.glob('./**/*.v?'))

  assert args.cmd
  cmd_list = args.cmd.split('.')
  if len(cmd_list)==0:
    assert 0
  elif len(cmd_list)==1:
    cmd_list.append('all')
  else:
    assert len(cmd_list)==2, cmd_list

  verilog_list = VerilogFileList(target_file_list, include_file_list)

  if 0:
    pass
  elif cmd_list[0]=='elab':
    if 0:
      pass
    elif cmd_list[1]=='all':
      verilog_list.elab_all()
      verilog_list.export_all()
    elif cmd_list[1]=='body':
      verilog_list.elab_body()
      verilog_list.export_module()
  else:
    assert 0, cmd_list[0]
