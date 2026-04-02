import argparse
import os
from pathlib import Path

from os_util import *
from re_util import *

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Analyze DesignCompiler logs')
  parser.add_argument('-input', '-i', type=str, help='input path')
  parser.add_argument('-output', '-o', type=str, help='output directory')
  parser.add_argument('-op', type=str, nargs='*', help='operation')
  args = parser.parse_args()

  output_path = Path(args.output)
  assert output_path.is_dir(), output_path
  input_path = Path(args.input)
  assert input_path.exists(), input_path
  if input_path.is_file():
    input_file_path = input_path
  elif input_path.is_dir():
    input_file_path_list = tuple(input_path.glob('*.rpt'))
    assert len(input_file_path_list)==1, input_file_path_list
    input_file_path = input_file_path_list[0]

  input_contents = input_file_path.read_text()

  reexp_error = memorize(r'^Error:' + reexp_anything + '\(' + memorize(r'[A-Za-z0-9-]+') + r'\)$')
  re_error = re.compile(reexp_error,re.MULTILINE)
  error_result = re_error.findall(input_contents)

  reexp_warning = memorize(r'^Warning:' + reexp_anything + '\(' + memorize(r'[A-Za-z0-9-]+') + r'\)$')
  re_warning = re.compile(reexp_warning,re.MULTILINE)
  warning_result = re_warning.findall(input_contents)

  error_result_known = []
  error_result_unknown = []
  for statement, code in error_result:
    if code=='CMD-036':
      error_result_known.append(statement)
      continue
    error_result_unknown.append(statement)

  warning_result_known = []
  warning_result_critical = []
  warning_result_unknown = []
  for statement, code in warning_result:
    if code=='VER-318':
      warning_result_known.append(statement)
      continue
    if code=='VER-540':
      warning_result_known.append(statement)
      continue
    if code=='LINK-26':
      warning_result_known.append(statement)
      continue
    if code=='VO-12':
      warning_result_known.append(statement)
      continue

    if code=='VER-1004':
      warning_result_critical.append(statement)
      continue
    if code=='UID-95':
      warning_result_critical.append(statement)
      continue
    warning_result_unknown.append(statement)

  op_list =[]
  for op in args.op:
    if op=='all':
      op_list.append('error.all')
      op_list.append('error.known')
      op_list.append('error.unknown')
      op_list.append('warning.all')
      op_list.append('warning.known')
      op_list.append('warning.critical')
      op_list.append('warning.unknown')
    else:
      op_list.append(op)

  for op in op_list:
    if None:
      pass
    elif op=='error.all':
      contents = '\n'.join([ x[0] for x in error_result ])
      output_file_path = input_file_path.parent / f'{input_file_path.stem}.{op}.txt'
      output_file_path.write_text(contents)
    elif op=='error.known':
      contents = '\n'.join(error_result_known)
      output_file_path = input_file_path.parent / f'{input_file_path.stem}.{op}.txt'
      output_file_path.write_text(contents)
    elif op=='error.unknown':
      contents = '\n'.join(error_result_unknown)
      output_file_path = input_file_path.parent / f'{input_file_path.stem}.{op}.txt'
      output_file_path.write_text(contents)
    elif op=='warning.all':
      contents = '\n'.join([ x[0] for x in warning_result ])
      output_file_path = input_file_path.parent / f'{input_file_path.stem}.{op}.txt'
      output_file_path.write_text(contents)
    elif op=='warning.known':
      contents = '\n'.join(warning_result_known)
      output_file_path = input_file_path.parent / f'{input_file_path.stem}.{op}.txt'
      output_file_path.write_text(contents)
    elif op=='warning.critical':
      contents = '\n'.join(warning_result_critical)
      output_file_path = input_file_path.parent / f'{input_file_path.stem}.{op}.txt'
      output_file_path.write_text(contents)
    elif op=='warning.unknown':
      contents = '\n'.join(warning_result_unknown)
      output_file_path = input_file_path.parent / f'{input_file_path.stem}.{op}.txt'
      output_file_path.write_text(contents)
