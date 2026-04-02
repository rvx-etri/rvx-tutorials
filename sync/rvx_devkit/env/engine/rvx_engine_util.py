## ****************************************************************************
## ****************************************************************************
## Copyright SoC Design Research Group, All rights reserved.    
## Electronics and Telecommunications Research Institute (ETRI)
##
## THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE 
## WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS 
## TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE 
## REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL 
## SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE, 
## IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE 
## COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.
## ****************************************************************************
## 2020-03-11
## Kyuseung Han (han@etri.re.kr)
## ****************************************************************************
## ****************************************************************************

from pathlib import Path
import import_util
from os_util import *
from re_util import *
from xml_util import *

def gen_script_file(cmd:str, file_path:Path):
  file_path.write_text(cmd)
  make_executable(file_path)

def run_xlaunch(binary_dir:Path):
  if not is_linux:
    execute_shell_cmd(binary_dir/'config.xlaunch', None, True)

def get_define_list(define_pragma:str, path:Path):
  file_contents = path.read_text()
  file_contents = remove_comments(file_contents)
  #reexp_define = define_pragma + reexp_blank + memorize(wordize(reexp_identifier)) + exist_or_not(reexp_blank + '\(' + memorize(reexp_anything) + r'\)') + r'$'
  reexp_define = define_pragma + reexp_blank + memorize(wordize(reexp_identifier)) + exist_or_not(reexp_blank + memorize(reexp_exp)) + r'$'
  re_define = re.compile(reexp_define,re.MULTILINE)
  define_list = re_define.findall(file_contents)
  return define_list
