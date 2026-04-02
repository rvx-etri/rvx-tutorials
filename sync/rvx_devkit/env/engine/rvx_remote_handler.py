from pathlib import Path, PurePosixPath

from rvx_engine_util import *
import os_util
from rvx_config import *

class RemoteHandler:
  def __init__(self, rvx_config:RvxConfig):
    self.config = rvx_config

  @property
  def ip_address(self):
    return self.config.ip_address

  @property
  def ssh_port(self):
    return int(self.config.ssh_port)

  @property
  def use_sshkey(self):
    return self.config.use_sshkey

  @property
  def username(self):
    return self.config.username
  
  @property
  def passwd(self):
    return self.config.passwd

  @property
  def remote_home_directory(self):
    return PurePosixPath('/home') / self.username / 'rvx_devkit'

  @property
  def plink(self):
    return self.config.windows_binary_path / 'plink.exe'

  def test_connect(self):
    try:
      self.make_ssh_cmd('exit')
      success = True
    except:
      success = False
    return success

  def print_login_info(self):
    print(self.ip_address, self.ssh_port, self.username, self.passwd)

  def make_ssh_cmd(self, cmd:str):
    if is_linux:
      local_cmd = 'sshpass -p {0} ssh -p{1} {2}@{3} \"bash -l -c \'{4}\'\"'.format(self.passwd, self.ssh_port, self.username, self.ip_address, cmd)
    else:
      local_cmd = '{0} {1}@{2} -pw {3} -P {4} \"export DISPLAY=localhost:10.0;bash -l -c \'{5}\'\"'.format(self.plink, self.username, self.ip_address, self.passwd, self.ssh_port, cmd)
    return local_cmd
    
  def store_key_in_cache(self):
    if is_linux:
      local_cmd = 'sshpass -p {0} ssh -p{1} -oStrictHostKeyChecking=no {2}@{3} \"exit\"'.format(self.passwd, self.ssh_port, self.username, self.ip_address)
    else:
      assert self.plink.is_file(), self.plink
      local_cmd = 'echo y | {0} {1}@{2} -pw {3} -P {4} \"exit\"'.format(self.plink, self.username, self.ip_address, self.passwd, self.ssh_port)
    run_shell_cmd(local_cmd)
  
  def request_ssh(self, cmd:str, uses_stdout:bool=False):
    remote_cmd = f'cd {self.remote_home_directory}; source ./rvx_setup.sh; {cmd}'
    local_cmd = self.make_ssh_cmd(remote_cmd)
    result = run_shell_cmd(local_cmd, asserts_when_error=False)
    if uses_stdout:
      print(result.stdout.decode())
      print(result.stderr.decode())
    return result
      
  def request_sftp_get(self, filename:str, remote_dir:str, local_dir:Path):
    assert local_dir.is_absolute(), local_dir
    assert local_dir.is_dir(), local_dir
    remote_file = self.remote_home_directory / remote_dir / filename
    local_file = local_dir / filename
    if is_linux:
      remote_cmd = f'get {remote_file}'
      sshpass_cmd = 'sshpass -p {0} sftp -P {1} {2}@{3} -b'.format(self.passwd, self.ssh_port, self.username, self.ip_address)
      local_cmd = f'echo \"{remote_cmd}\" | {sshpass_cmd}'
      run_shell_cmd(local_cmd, local_dir, stderr=subprocess.STDOUT)
    else:
      remote_cmd = f'get {remote_file}'
      script_file = local_dir / 'rvx_sftp_script.sh'
      script_file.write_text(remote_cmd)

      sshpass_cmd = 'psftp -pw {0} -P {1} {2}@{3} -b'.format(self.passwd, self.ssh_port, self.username, self.ip_address)
      local_cmd = f'{sshpass_cmd} {script_file}'
      run_shell_cmd(local_cmd, local_dir, stderr=subprocess.STDOUT)
    return local_file

  def request_sftp_put(self, filename:str, local_dir:Path, remote_dir:str):
    assert local_dir.is_absolute(), local_dir
    assert local_dir.is_dir(), local_dir
    local_file = local_dir / filename
    remote_file = self.remote_home_directory / remote_dir / filename
    if is_linux:
      remote_cmd = f'put {filename} {remote_file}'
      sshpass_cmd = 'sshpass -p {0} sftp -P {1} {2}@{3} -b'.format(self.passwd, self.ssh_port, self.username, self.ip_address)
      local_cmd = f'echo \"{remote_cmd}\" | {sshpass_cmd}'
      run_shell_cmd(local_cmd, local_dir, stderr=subprocess.STDOUT)
    else:
      remote_cmd = f'put {filename} {remote_file}'
      script_file = local_dir / 'rvx_sftp_script.sh'
      script_file.write_text(remote_cmd)

      sshpass_cmd = 'psftp -pw {0} -P {1} {2}@{3} -b'.format(self.passwd, self.ssh_port, self.username, self.ip_address)
      local_cmd = f'{sshpass_cmd} {script_file}'
      run_shell_cmd(local_cmd, local_dir, stderr=subprocess.STDOUT)

  def extract_tar_file(self, filename:str, remote_dir:str, local_dir:Path):
    assert local_dir.is_absolute()
    local_file = self.request_sftp_get(filename, remote_dir, local_dir)
    extract_file(local_file)
    remove_file(local_file)

  def is_file(self, filename:str):
    remote_file = self.remote_home_directory / filename
    remote_cmd = f'cd {self.remote_home_directory}; source ./rvx_setup.sh; stat {remote_file}'
    local_cmd = self.make_ssh_cmd(remote_cmd)
    try:
      run_shell_cmd(local_cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
      result = True
    except:
      result = False
    return result

  def remove_file(self, remote_dir:str, filename:str):
    self.request_ssh(f'rm -rf {remote_dir}/{filename}')
