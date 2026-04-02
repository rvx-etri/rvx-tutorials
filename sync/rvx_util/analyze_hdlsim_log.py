import argparse
from pathlib import Path

from re_util import *
from get_hdlsim_info import *

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Analyze HDL logs')
  parser.add_argument('-tool', '-t', type=str, help='tool')
  parser.add_argument('-input', '-i', type=str, help='input path')
  args = parser.parse_args()

  input_path = Path(args.input).resolve()
  assert input_path.is_dir(), input_path

  while 1:
    if args.tool=='ncsim' or args.tool=='xcelium':
      verilog_compile_log_path = input_path / get_hdlsim_filename(args.tool, 'compile.log')
      if not verilog_compile_log_path.is_file():
        break
      verilog_compile_log = verilog_compile_log_path.read_text()
      reexp_error = memorize(r'^' + reexp_anything) + r'$\n' + \
          memorize(r'^' + reexp_anything) + r'$\n' + \
          r'^xmvlog:' + reexp_blank_or_not + '\*E,' + memorize(reexp_identifier) + reexp_blank_or_not + r'\(' + \
          memorize(r'[^\(^\)]*') + '\):' + reexp_blank_or_not + \
          memorize(reexp_anything) + r'$'
      re_error = re.compile(reexp_error,re.MULTILINE)
      error_list = re_error.findall(verilog_compile_log)
      if len(error_list):
        for index, (pre1, pre2, error_type, filename, log) in enumerate(error_list):
          if index>10:
            break
          print(f'\n[ERROR] {error_type}')
          print(filename)
          print(log)
          print(pre1)
          print(pre2)
        break
      elab_log_path = input_path / get_hdlsim_filename(args.tool, 'elaborate.log')
      if not elab_log_path.is_file():
        assert 0, 'Bug?'
      elab_log = elab_log_path.read_text()
      reexp_warning = memorize(r'^' + reexp_anything) + r'$\n' + \
          memorize(r'^' + reexp_anything) + r'$\n' + \
          r'^xmelab:' + reexp_blank_or_not + '\*[EW],' + memorize(reexp_identifier) + reexp_blank_or_not + r'\(' + \
          memorize(r'[^\(^\)]*') + '\):' + reexp_blank_or_not + \
          memorize(reexp_anything) + r'$'
      re_warning = re.compile(reexp_warning,re.MULTILINE)
      warning_list = re_warning.findall(elab_log)
      if len(warning_list):
        for index, (pre1, pre2, warning_type, filename, log) in enumerate(warning_list):
          if index>10:
            break
          print(f'\n[WARNING] {warning_type}')
          print(filename)
          print(log)
          print(pre1)
          print(pre2)
      break
    elif args.tool=='modelsim' or args.tool=='questa':
      break
    else:
      assert 0, args.tool
