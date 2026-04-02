import argparse
import os
import shutil

from pathlib import Path
from os_util import *
from gather_files import *

if __name__ =='__main__':
  parser = argparse.ArgumentParser(description='Exporting Files')
  parser.add_argument('-source', '-s', type=str, nargs='*', help='source file or directory')
  parser.add_argument('-output', '-o', help='export directory')
  parser.add_argument('-extension', '-ext', type=str, nargs='*', help='allowed file extension')
  args = parser.parse_args()
  
  assert args.output
  output_directory = Path(args.output).absolute()
  print(args.output)
  print(output_directory)
  if output_directory.is_dir():
    shutil.rmtree(output_directory)
  output_directory.mkdir(parents=True)

  gathered_file_list = gather_file_list(args.source, args.extension)
  
  for source_file in gathered_file_list:
    target_file = output_directory / source_file.name
    copy_file(source_file, target_file)
