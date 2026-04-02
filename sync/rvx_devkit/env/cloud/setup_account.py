import os, sys
import argparse
from pathlib import Path

sys.path.append(os.environ.get('RVX_UTIL_HOME'))

from os_util import *

if __name__ == '__main__':
  # argument
  parser = argparse.ArgumentParser(description='RVX account setup')
  parser.add_argument('--force', action="store_true", help='setup unconditionally')
  args = parser.parse_args()

  # variable
  shared_path = Path(os.environ.get('RVX_SHARED_HOME'))
  shared_devkit_path = shared_path / 'rvx_devkit'
  home_path = Path.home()
  private_devkit_path = home_path / 'rvx_devkit'
  version_info_filename = 'cloud_info_common.xml'
  shared_devkit_info_path = shared_path / version_info_filename
  private_devkit_info_path = private_devkit_path / version_info_filename

  # check
  setup_is_required = True
  if (not args.force) and private_devkit_info_path.is_file():
    if is_equal_file(private_devkit_info_path, shared_devkit_info_path):
      setup_is_required = False
  
  # setup
  if setup_is_required:
    # bashrc
    bashrc_filename = '.bashrc'
    copy_file(shared_path/bashrc_filename, home_path/bashrc_filename)
    # devkit
    remove_directory(private_devkit_path)
    copy_directory(shared_devkit_path, private_devkit_path)
    # config
    execute_shell_cmd('make config > /dev/null 2>&1', private_devkit_path)
    env_path = private_devkit_path / 'env'
    execute_shell_cmd(f'make cloud.init RVX_DEVKIT_HOME={private_devkit_path} RVX_ENV={env_path}> /dev/null 2>&1', private_devkit_path)
