import os
import io
import argparse
from pathlib import *

def round_up_int(value:int, step:int):
  residue = value % step;
  result = value;
  if residue!=0:
    result += (step-residue);
  return result;

if __name__ == '__main__':
  # parse argument
  parser = argparse.ArgumentParser(description='Converting Input Files')
  parser.add_argument('-input', '-i', help='input files')
  parser.add_argument('-output', '-o', help='output directory')
  args = parser.parse_args()

  assert args.input
  assert args.output

  input_path = Path(args.input)
  assert input_path.is_file()
  output_directory = Path(args.output)
  assert output_directory.is_dir()

  concatenated_contents = input_path.read_bytes()
  total_size = len(concatenated_contents)
  i = 0
  while i<total_size:
    file_size = concatenated_contents[i+3]
    file_size = (file_size<<8) + concatenated_contents[i+2]
    file_size = (file_size<<8) + concatenated_contents[i+1]
    file_size = (file_size<<8) + concatenated_contents[i+0]
    i += 4
    file_name = ''
    while 1:
      value = concatenated_contents[i]
      i += 1
      if value:
        file_name += chr(value)
      else:
        break
        
    contents = concatenated_contents[i:i+file_size]
    #print(contents)
    output_path = output_directory / file_name
    output_path.write_bytes(contents)
    print(f'{output_path.relative_to(os.getcwd())}: {file_size} bytes')
    i += file_size
    i = round_up_int(i,4)
