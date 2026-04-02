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
## 2019-09-24
## Kyuseung Han (han@etri.re.kr)
## ****************************************************************************
## ****************************************************************************

import os
import getpass

from pathlib import Path

from rvx_engine_util import *
from rvx_remote_handler import *
from rvx_config import *
from rvx_engine_log import *

import import_util
from generate_git_info import *
from manage_version_info import *

class RvxDevkit():
  @staticmethod
  def set_if_env_exist(variable_name):
    x = os.environ.get(variable_name)
    if x:
      x = Path(x)
      if not x.is_dir():
        x = None
    return x

  def __init__(self, config:RvxConfig, output_file:str, engine_log:RvxEngineLog, called_by_gui:bool):
    self.config = config
    self.output_file = Path(output_file).resolve() if output_file else None
    self.engine_log = engine_log
    self.called_by_gui = called_by_gui
    self.remote_handler = RemoteHandler(self.config)
    self.is_debug_mode = (self.config.home_path / 'debug').is_file()
    if self.config.is_local and self.is_debug_mode:
      print('Debug On!')

  def get_env_path(self, *args):
    return self.config.env_path.joinpath(*args)
  
  @property
  def get_imp_class_list_common_file(self):
    return self.get_env_path('info','rvx_imp_class_common.xml')

  @property
  def client_sync_config_path(self):
    return self.config.home_path / 'sync' / 'client_info.xml'

  def get_client_sync_config(self):
    if self.client_sync_config_path.is_file():
      client_sync_config = generate_config_from_version_info(self.client_sync_config_path)
    else:
      client_sync_config = None
    return client_sync_config
  
  @property
  def server_log_path(self):
    return self.engine_log.log_path / 'server_log'

  def get_utility_file(self, file_name):
    return self.config.utility_path / file_name

  def set_gitignore(self, type:str, path:Path):
    dst_file = path / '.gitignore'
    if not dst_file.exists():
      src_file = self.get_env_path('git',f'gitignore.{type}.txt')
      assert src_file.is_file()
      copy_file(src_file, dst_file)
    
  def geneate_rvx_each_template(self, type:str, path:Path):
    dst_file = path / 'rvx_each.mh'
    if not dst_file.exists():
      src_file = self.get_env_path('rvx_each',f'rvx_each.{type}.txt')
      assert src_file.is_file()
      copy_file(src_file, dst_file)

  def handle_output(self, contents:str):
    if self.output_file:
      with open(self.output_file,'w') as f:
        f.write(contents)
    else:
      print(contents)

  def get_remote_handler(self):
    if not self.config.server_config.equal_to_file:
      self.config.server_config.set_from_input()
    success = self.remote_handler.test_connect()
    assert success, 'RVX server login FAIL'
    if not self.config.server_config.equal_to_file:
      self.config.server_config.export_file()
    return self.remote_handler

  def get_sudo_passwd(self):
    if not self.config.sudo_config.equal_to_file:
      self.config.sudo_config.set_from_input()
      self.config.sudo_config.export_file()
    sudo_passwd = self.config.sudo_passwd
    return sudo_passwd

  def add_new_job(self, job_name:str, is_local:bool, job_status:str=None):
    self.engine_log.add_new_job(job_name, is_local, job_status)

  def add_log(self, log:str, status:str=None, is_file:bool=False, is_user:bool=False):
    self.engine_log.add_log(log, is_file=is_file, is_user=is_user)
    if status!=None:
      self.engine_log.set_status(status)

  def add_process_log(self, log:subprocess.CompletedProcess, is_user:bool):
    if log.stderr:
      contents = ''
      if log.stdout:
        contents += log.stdout
        contents += '\n'
      contents += log.stderr
      self.engine_log.add_log(contents, is_file=True, is_user=True)
      self.engine_log.set_status('fail')
    else:
      if log.stdout:
        self.engine_log.add_log(log.stdout, is_file=True, is_user=True)
      self.engine_log.set_status('done')

  def check_log(self, print_if_local:bool=False):
    self.engine_log.export_file(self.config.is_server)
    status = self.engine_log.current_job.status
    if status=='error' or status=='fail':
      if self.config.is_local:
        print(self.engine_log.current_job)
      if status=='error':
        exit(11)
      else:
        exit(21)
    else:
      if self.config.is_local and print_if_local:
        print(self.engine_log.current_job)

  def make_at_server(self, make_target:str='', background:bool=False):
    assert self.config.is_client
    remote_cmd_list = []
    remote_cmd_list.append('make --no-print-directory')
    remote_cmd_list.append(make_target)
    #remote_cmd_list.append('> /dev/null 2>&1')
    if background:
      remote_cmd_list.append('&')
    
    remote_cmd = ' '.join(remote_cmd_list)
    result = self.get_remote_handler().request_ssh(remote_cmd, uses_stdout=self.is_debug_mode)
    if result.returncode > 1:
      self.append_server_log()
      self.check_log(True)
      assert 0, f'Never reaches {result.returncode}'
      #assert 0, (make_target, result.returncode, self.engine_log.current_job)

  def append_server_log(self):
    self.download_server_log()
    server_log = RvxEngineLog()
    server_log.import_file(self.server_log_path)
    self.engine_log.job_list += server_log.job_list

  def append_server_user_log(self):
    self.download_server_log()
    server_log = RvxEngineLog()
    server_log.import_file(self.server_log_path)
    for job in server_log.job_list:
      if job.is_user:
        self.engine_log.append_job(job)

  def download_server_log(self):
    assert self.config.is_client
    remove_directory(self.server_log_path)
    self.server_log_path.mkdir()
    self.get_remote_handler().extract_tar_file(self.engine_log.log_tar_file.name, '.', self.server_log_path)

  def studio_cmd(self):
    lib_jar_list = []
    for lib_jar in self.config.gui_path.glob('./lib/*.jar'):
      lib_jar_list.append(str(lib_jar))
    lib_jar_arg = ':'.join(lib_jar_list)
    studio_binary = self.config.gui_path / 'rvx_studio.jar'
    cmd = f'java -cp {lib_jar_arg} -jar {studio_binary}'
    return cmd

  def home_git_info(self):
    git_version = get_git_version(self.config.home_path)
    return git_version
  
  def home_git_name(self):
    git_name = get_git_name(self.config.home_path)
    return git_name

  def devkit_git_info(self):
    if self.config.is_mini:
      client_sync_config = self.get_client_sync_config()
      git_version = client_sync_config.get_attr('rvx_version.commit')
    else:
      git_version = get_git_version(self.config.devkit_path)
    return git_version

  def username(self):
    if self.config.is_mini:
      username = self.config.username
    else:
      username = getpass.getuser()
    return username

  def spec_checker_cmd(self):
    checker_binary = self.get_env_path('spec_checker', 'rvx_spec_checker.jar')
    assert checker_binary.is_file(), checker_binary
    return f'java -jar {checker_binary}'
