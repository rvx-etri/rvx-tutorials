import os
import io
import argparse
import tkinter as tk
import tkinter.font as tkFont
import import_util

from pathlib import *
from collections import defaultdict
from os_util import *

def check_if_path_has_brief_gui(path:Path):
  is_engine_path = False
  if path and path.is_dir():
    makefile_path = path / 'Makefile'
    if makefile_path.is_file():
      result = run_shell_cmd(cmd='make --no-print-directory _check_gui', cwd=path, prints_when_error=False, asserts_when_error=False)
      if result.returncode==0:
        is_engine_path = True
  return is_engine_path  

if __name__ == '__main__':
  # parse argument
  parser = argparse.ArgumentParser(description='Converting Input Files')
  parser.add_argument('-path', '-p', help='Makefile path')
  parser.add_argument('-cmd', '-c', nargs='+', help='Command list')
  parser.add_argument('-cmd_para', '-cp', nargs='+', help='Parameter list')
  parser.add_argument('-font', '-f', help='Font')
  parser.add_argument('--adv', action="store_true", help='if advanced')
  args = parser.parse_args()

  assert args.path
  assert args.cmd

  makefile_path = Path(args.path).resolve().absolute()
  assert makefile_path.is_dir(), makefile_path
  
  command_list = args.cmd
  
  cmd_para_dict = defaultdict(list)
  if args.cmd_para:
    for cmd_para in args.cmd_para:
      cmd, para = cmd_para.split(':')
      cmd_para_dict[cmd].append(para)
      
  #print(args.cmd_para)
  #print(cmd_para_dict)
    
  assert len(command_list) > 0, command_list
  if args.font:
    font_size = int(args.font)
  else:
    font_size = 16

  #
  main_window = tk.Tk()
  if args.adv:
    main_window.title('RVX Brief GUI with Advanced Feature')
  else:
    main_window.title('RVX Brief GUI')
  font_style = tkFont.Font(family="Arial", size=font_size)
  
  row_index = 0
  column_size = len(command_list)
  
  #
  label = tk.Label(main_window, text=makefile_path, font=font_style)
  label.grid(row=row_index, column=0, columnspan=column_size, sticky="we", padx=5, pady=1)
  row_index += 1
  
  #
  hline = tk.Frame(main_window, height=2, bd=1, relief="sunken", bg="black")
  hline.grid(row=row_index, column=0, columnspan=column_size, sticky="we", padx=5, pady=1)
  row_index += 1
  
  cmd_para_entry_dict = {}
  #
  def generate_button_command(command:str):
    def execute():
      make_target = command
      make_arg = ''
      for para in cmd_para_dict[command]:
        value = cmd_para_entry_dict[(command,para)].get()
        if value:
          make_arg += f' {para}={value}'
      make_command = f'make --no-print-directory {make_target}{make_arg}'
      exe_path = makefile_path
      print(f'\n[DIR] {exe_path}')
      print(f'[CMD] {make_target}{make_arg}')
      print(f'[START] {command}')
      execute_shell_cmd(cmd=make_command,cwd=exe_path)
      print(f'[END] {command}')
    return execute

  #
  cmd_row_max = 1
  for i, command in enumerate(command_list):
    button = tk.Button(main_window, text=command, command=generate_button_command(command), font=font_style)
    button.grid(row=row_index, column=i, padx=5, pady=1)
    cmd_row = 1 + len(cmd_para_dict[command])*2
    cmd_row_max = max(cmd_row_max,cmd_row)
    for j, para in enumerate(cmd_para_dict[command]):
      label = tk.Label(main_window, text=para, font=font_style)
      label.grid(row=row_index+1+j*2, column=i, sticky="we", padx=5, pady=1)
      entry = tk.Entry(main_window, width=10, font=font_style)
      entry.grid(row=row_index+2+j*2, column=i, sticky="we", padx=5, pady=1)
      cmd_para_entry_dict[(command,para)] = entry
  row_index += cmd_row_max
  
  #
  hline = tk.Frame(main_window, height=5, bd=1, relief="sunken", bg="black")
  hline.grid(row=row_index, column=0, columnspan=column_size, sticky="we", padx=5, pady=1)
  row_index += 1

  #
  def generate_button_open_and_kill(path:Path):
    def open_and_kill():
      if args.adv:
        cmd = 'make --no-print-directory _open_gui_cmd_adv'
      else:
        cmd = 'make --no-print-directory _open_gui_cmd'
      execute_shell_cmd_with_terminal(cmd=cmd, cwd=path)
      main_window.destroy()
    return open_and_kill
  
  open_and_kill_at_makefile_path = generate_button_open_and_kill(makefile_path)
  
  button = tk.Button(main_window, text='Refresh', command=open_and_kill_at_makefile_path, font=font_style)
  button.grid(row=row_index, column=0, columnspan=column_size, sticky="we", padx=5, pady=1)
  row_index += 1
  
  def generate_button_direct_target(target:str):
    def execute():
      print(makefile_path)
      execute_shell_cmd(cmd=f'make {target}',cwd=makefile_path)
      #open_and_kill_at_makefile_path() # bug
    return execute
  
  if args.adv:
    button = tk.Button(main_window, text='Update Makefile', command=generate_button_direct_target('update_makefile'), font=font_style)
    button.grid(row=row_index, column=0, columnspan=column_size, sticky="we", padx=5, pady=1)
    row_index += 1
    
    button = tk.Button(main_window, text='Update Makefile Recursively', command=generate_button_direct_target('update_makefiles'), font=font_style)
    button.grid(row=row_index, column=0, columnspan=column_size, sticky="we", padx=5, pady=1)
    row_index += 1
  
  #
  hline = tk.Frame(main_window, height=5, bd=1, relief="sunken", bg="black")
  hline.grid(row=row_index, column=0, columnspan=column_size, sticky="we", padx=5, pady=1)
  row_index += 1
  
  #
  move_list = []
  
  #
  parent_path = makefile_path.parent
  if check_if_path_has_brief_gui(parent_path):
    button_text = 'Parent: Go'
    button_command = generate_button_open_and_kill(parent_path)
  else:
    button_text = 'Parent: N/A'
    button_command = None
  move_list.append((button_text,button_command))
  
  for subpath in makefile_path.glob('*'):
    if subpath.is_dir() and check_if_path_has_brief_gui(subpath):
      button_text = str(subpath).replace(str(parent_path),'.')
      button_command = generate_button_open_and_kill(subpath)
      move_list.append((button_text,button_command))
  
  #
  for button_text, button_command in move_list:
    button = tk.Button(main_window, text=button_text, command=button_command, font=font_style)
    button.grid(row=row_index, column=0, columnspan=column_size, sticky="we", padx=5, pady=1)
    row_index += 1

  #
  main_window.mainloop()
