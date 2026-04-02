import os
import argparse
import time
import shutil
from pathlib import *
from tqdm.auto import tqdm
import import_util

from re_util import *
from npx_cfg_parser import *

def get_define_list(define_pragma, file_name):
  with open(file_name,'r') as f:
    file_contents = f.read()
  file_contents = remove_comments(file_contents)
  #reexp_define = define_pragma + reexp_blank + memorize(wordize(reexp_identifier)) + exist_or_not(reexp_blank + '\(' + memorize(reexp_anything) + r'\)') + r'$'
  reexp_define = define_pragma + reexp_blank + memorize(wordize(reexp_identifier)) + exist_or_not(reexp_blank + memorize(reexp_exp)) + r'$'
  re_define = re.compile(reexp_define,re.MULTILINE)
  define_list = re_define.findall(file_contents)
  return define_list

def gen_operator_info(app_cfg_path:Path, ssw_info_path:Path, output_dir_path:Path, suffix:str):
  define_list = get_define_list('#define', ssw_info_path)  
  define_dict = {}
  for id, value in define_list:
    define_dict[id] = value
  
  text_parser = NpxCfgParser(app_cfg_path)
  operator_info = NpxCfgParser()
  for i, layer_option in enumerate(text_parser.layer_info_list):
    cfg_section = operator_info.add_network(layer_option.name)
    if layer_option.name == 'Linear':
      if 'USE_DCA' in define_dict:
        cfg_section['operator'] = 'dca'
      elif 'USE_VTA' in define_dict:
        cfg_section['operator'] = 'vta'
      else:
        cfg_section['operator'] = 'cpu'
    elif layer_option.name == 'Conv2d':
      if 'USE_DCA' in define_dict:
        cfg_section['operator'] = 'dca'
      elif 'USE_VTA' in define_dict:
        cfg_section['operator'] = 'vta'
      else:
        cfg_section['operator'] = 'cpu'
    elif layer_option.name == 'MaxPool2d':
      cfg_section['operator'] = 'cpu'
    elif layer_option.name == 'AvgPool2d':
      cfg_section['operator'] = 'cpu'
    elif layer_option.name == 'Flatten':
      cfg_section['operator'] = 'cpu'
    elif layer_option.name == 'Leaky':
      cfg_section['operator'] = 'cpu'
    else:
      assert 0, layer_option.name

  operator_info_path = output_dir_path / (app_cfg_path.stem + suffix + app_cfg_path.suffix)
  operator_info_path.write_text(str(operator_info))
  
if __name__ == '__main__':
  
  parser = argparse.ArgumentParser(description='NPX Framework')
  parser.add_argument('-cfg', '-c', nargs='+', help='app cfg file name')
  parser.add_argument('-ssw_info', '-i', help='ssw_info XML')
  parser.add_argument('-output', '-o', help='output directory')
  parser.add_argument('-suffix', '-s', help='suffix')

  # check args
  args = parser.parse_args()
  assert args.cfg
  assert args.output
  assert args.ssw_info
  assert args.suffix

  app_cfg_list = args.cfg
  #print(app_cfg_list)
  ssw_info_path = Path(args.ssw_info).absolute()
  assert ssw_info_path.is_file(), ssw_info_path
  result_dir_path = Path(args.output).absolute()

  # cfg
  for app_cfg in app_cfg_list:
    app_cfg_path = Path(app_cfg)
    assert app_cfg_path.is_file(), app_cfg_path
    output_dir_path = result_dir_path / app_cfg_path.stem / 'riscv'
    if not output_dir_path.is_dir():
      output_dir_path.mkdir(parents=True, exist_ok=True)
    gen_operator_info(app_cfg_path=app_cfg_path, ssw_info_path=ssw_info_path, output_dir_path=output_dir_path, suffix=args.suffix)
