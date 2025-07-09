import os, sys, time
from pathlib import Path

sys.path.append(str(Path(os.environ.get('RVX_MINI_HOME'))/'rvx_install'))

from os_util import *

if __name__ == '__main__':
  home_path = Path('.').resolve()
  sync_path = home_path / 'sync'
  manual_path = home_path / 'manual'
  is_sync_fail = False

  # remove
  for dir in (home_path/'rvx_devkit', home_path/'rvx_util', home_path/'rvx_hwlib', manual_path):
    remove_directory(dir)
  time.sleep(0.2)
  for dir in (home_path/'rvx_devkit', home_path/'rvx_util', home_path/'rvx_hwlib', manual_path):
    if dir.is_dir():
      is_sync_fail = True

  # extract
  tar_list = ('rvx_devkit','rvx_util','rvx_hwlib','rvx_ssw')
  for tar_name in tar_list:
    extracted_path = sync_path/tar_name
    target_path = home_path/tar_name
    remove_directory(extracted_path)
    remove_directory(target_path)
    extract_file(sync_path/f'{tar_name}.tar.gz')
    move_directory(extracted_path, target_path)

  # misc
  manual_path.mkdir()
  for manual in (home_path/'rvx_devkit'/'env'/'manual').glob('*'):
    copy_file(manual, home_path/'manual'/manual.name)
  for manual in (home_path/'rvx_install').glob('*_manual*.pdf'):
    if not (home_path/'manual'/manual.name).exists():
      copy_file(manual, home_path/'manual'/manual.name)

  if is_linux and (home_path/'rvx_binary'/'rvx_setup.sh').is_file():
    pass
  else:
    for body_file in (home_path/'rvx_ssw').glob('**/*.c*'):
      remove_file(body_file)

  run_shell_cmd('git checkout ./rvx_hwlib', home_path, prints_when_error=False, asserts_when_error=False)

  for dir in (home_path/'rvx_devkit', home_path/'rvx_util', home_path/'rvx_hwlib', manual_path):
    if not dir.is_dir():
      is_sync_fail = True
      break

  # local_setup
  execute_shell_cmd(f'make --no-print-directory local_setup', home_path)
  time.sleep(0.2)

  if not (home_path/'local_setup').is_dir():
    is_sync_fail = True

  # fpga_component
  for fpga_component in home_path.glob('**/fpga_component'):
    remove_directory(fpga_component)

  #  
  if is_sync_fail:
    remove_directory(home_path/'rvx_devkit')

  '''
    if sync_is_required:
      # env
      remove_directory(self.devkit.config.env_path)
      self.devkit.make_remotely('cloud.init', silent=True, logging=True)
      self.devkit.get_remote_handler().extract_tar_file(remote_env_filename, '.', self.devkit.config.devkit_path)
      move_directory(self.devkit.config.devkit_path/'env'/'manual', self.devkit.config.devkit_path / 'manual')
      # eclipse
      if is_linux:
        execute_shell_cmd('find -type f -name * -exec sed -i \"s/python/${PYTHON3_CMD}/g\" {} \;', self.devkit.config.eclipse_solution_template_path)
        execute_shell_cmd('find -type f -name * -exec sed -i \"s/python/${PYTHON3_CMD}/g\" {} \;', self.devkit.config.eclipse_project_template_path)
        #
        execute_shell_cmd('sudo cp -f ./env/minicom/* /etc/', self.devkit.config.devkit_path)
        execute_shell_cmd('sudo cp -rf ./env/minicom /etc/', self.devkit.config.devkit_path)
        new_rules_path = Path('./env/rules.d/')
        old_rules_path = Path('/etc/udev/rules.d/')
        there_is_new_rule = False
        for rules_file in new_rules_path.glob('*'):
          old_rules_file = old_rules_path / rules_file.name
          if not old_rules_file.is_file():
            there_is_new_rule = True
          elif not is_equal_file(rules_file, old_rules_file):
            there_is_new_rule = True
        if there_is_new_rule:
          execute_shell_cmd(f'sudo cp -f {rules_file} {old_rules_path}', self.devkit.config.devkit_path)
          if is_centos:
            execute_shell_cmd('sudo service network restart', self.devkit.config.devkit_path)
          elif is_ubuntu:
            execute_shell_cmd('sudo service udev restart', self.devkit.config.devkit_path)
          else:
            assert 0

    if gdb_script_is_required:
      # gdb script
      self._make_gdb_script_file(remote_info[1])
      # update eclipse template
      app_debug_config_path = self.devkit.config.eclipse_solution_template_path / '.metadata' / '.plugins' / 'org.eclipse.debug.core' / '.launches'
      template_file = app_debug_config_path / '{0}.template'.format(app_debug_config_file_name)
      output_file = app_debug_config_path / app_debug_config_file_name
      config_list = []
      if is_linux:
        config_list.append(('GDB_BINARY','rvx_binary/centos/riscv64-unknown-elf-gdb'))
      else:
        config_list.append(('GDB_BINARY','rvx_binary/centos/riscv64-unknown-elf-gdb.exe'))
      config_list.append(('GDB_SERVER', f'{self.devkit.get_remote_handler().address}:{remote_info[1]}'))
      configure_template_file(template_file, output_file, config_list)

   def _make_gdb_script_file(self, gdb_port):
    script = 'target remote {0}:{1}'.format(self.devkit.get_remote_handler().address, gdb_port)
    script += '\nload'
    script += '\nset riscv use_compressed_breakpoint off'

    gdb_script_file = self.devkit.get_env_path('gdb_script')
    with open(gdb_script_file,'w') as f:
      f.write(script)
    '''
