import argparse
import os
from pathlib import Path

from os_util import *
from re_util import *

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Analyze Vivado logs')
  parser.add_argument('-input', '-i', type=str, help='input vivado log file')
  parser.add_argument('-output', '-o', type=str, help='output directory')
  parser.add_argument('-op', type=str, nargs='*', help='operation')
  args = parser.parse_args()

  output_path = Path(args.output)
  assert output_path.is_dir(), output_path
  input_file_path = Path(args.input)
  assert input_file_path.is_file(), input_file_path
  input_contents = input_file_path.read_text()

  reexp_error = r'^ERROR:' + reexp_anything + r'$'
  re_error = re.compile(reexp_error,re.MULTILINE)
  error_result = re_error.findall(input_contents)

  reexp_critical = r'^CRITICAL WARNING:' + reexp_anything + r'$'
  re_critical = re.compile(reexp_critical,re.MULTILINE)
  critical_result = re_critical.findall(input_contents)

  reexp_warning = r'^WARNING:' + reexp_anything + r'$'
  re_warning = re.compile(reexp_warning,re.MULTILINE)
  warning_result = re_warning.findall(input_contents)

  critical_result_known = []
  critical_result_unknown = []
  for statement in critical_result:
    known = False
    if '.xdc' in statement:
      known = True

    if known:
      critical_result_known.append(statement)
    else:
      critical_result_unknown.append(statement)

  warning_result_known = []
  warning_result_unknown = []
  for statement in warning_result:
    known = False
    if '.xdc' in statement:
      known = True
    elif 'munoc_' in statement:
      known = True
    elif 'MUNOC_' in statement:
      known = True
    elif 'already exists in the project' in statement:
      known = True

    if known:
      warning_result_known.append(statement)
    else:
      warning_result_unknown.append(statement)

  op_list =[]
  for op in args.op:
    if op=='all':
      op_list.append('error.all')
      op_list.append('critical.all')
      op_list.append('critical.known')
      op_list.append('critical.unknown')
      op_list.append('warning.all')
      op_list.append('warning.known')
      op_list.append('warning.unknown')
    else:
      op_list.append(op)

  for op in op_list:
    if op=='error.all':
      contents = '\n'.join(error_result)
      output_file_path = input_file_path.parent / f'{input_file_path.stem}.{op}{input_file_path.suffix}'
      output_file_path.write_text(contents)
    elif op=='critical.all':
      contents = '\n'.join(critical_result)
      output_file_path = input_file_path.parent / f'{input_file_path.stem}.{op}{input_file_path.suffix}'
      output_file_path.write_text(contents)
    elif op=='critical.known':
      contents = '\n'.join(critical_result_known)
      output_file_path = input_file_path.parent / f'{input_file_path.stem}.{op}{input_file_path.suffix}'
      output_file_path.write_text(contents)
    elif op=='critical.unknown':
      contents = '\n'.join(critical_result_unknown)
      output_file_path = input_file_path.parent / f'{input_file_path.stem}.{op}{input_file_path.suffix}'
      output_file_path.write_text(contents)
    elif op=='warning.all':
      contents = '\n'.join(warning_result)
      output_file_path = input_file_path.parent / f'{input_file_path.stem}.{op}{input_file_path.suffix}'
      output_file_path.write_text(contents)
    elif op=='warning.known':
      contents = '\n'.join(warning_result_known)
      output_file_path = input_file_path.parent / f'{input_file_path.stem}.{op}{input_file_path.suffix}'
      output_file_path.write_text(contents)
    elif op=='warning.unknown':
      contents = '\n'.join(warning_result_unknown)
      output_file_path = input_file_path.parent / f'{input_file_path.stem}.{op}{input_file_path.suffix}'
      output_file_path.write_text(contents)
