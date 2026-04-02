# ****************************************************************************
# ****************************************************************************
# Copyright SoC Design Research Group, All rights reserved.
# Electronics and Telecommunications Research Institute (ETRI)
##
# THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE
# WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS
# TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE
# REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL
# SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE,
# IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE
# COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.
# ****************************************************************************
# 2020-03-11
# Kyuseung Han (han@etri.re.kr)
# ****************************************************************************
# ****************************************************************************

import platform
import os
import sys
import subprocess
import shutil
import distro

from urllib import request
from filecmp import cmp
from pathlib import Path

is_linux = (platform.system() == 'Linux')
is_windows = not is_linux
is_centos = 'CentOS' in distro.name() or 'Red Hat' in distro.name()
is_ubuntu = 'Ubuntu' in distro.name() or 'Debian' in distro.name()
is_other_os = (not is_windows) and (not is_centos) and (not is_ubuntu)
prints_shell_cmd = False
encoding = 'utf8' if is_linux else 'cp949'

# depredicated soon


def log_shell_cmd(cmd: str, cwd: Path = None, stderr_as_stdout: bool = False):
    stdout = subprocess.PIPE
    if stderr_as_stdout:
        stderr = subprocess.STDOUT
    else:
        stderr = subprocess.PIPE
    result = subprocess.run(cmd, cwd=cwd, shell=True,
                            stdout=stdout, stderr=stderr, encoding=encoding)
    return result

# depredicated soon


def get_shell_stdout(cmd: str, cwd=None):
    result = log_shell_cmd(cmd, cwd)
    assert not result.stderr, result.stderr
    return result.stdout

# depredicated soon


def get_shell_stderr(cmd: str, cwd=None):
    return log_shell_cmd(cmd, cwd).stderr


def run_shell_cmd(cmd: str, cwd: Path = None, stdout=subprocess.PIPE, stderr=subprocess.PIPE, prints_when_error=True, asserts_when_error=True):
    result = subprocess.run(cmd, cwd=cwd, shell=True,
                            stdout=stdout, stderr=stderr, encoding=encoding)
    if result.returncode != 0:
        if asserts_when_error:
            assert 0, result
        elif prints_when_error:
            print(result)
    return result

# depredicated


def execute_shell_cmd(cmd: str, cwd: Path = None, background: bool = False, prints_cmd: bool = False):
    if background:
        if is_linux:
            cmd = '{0} &'.format(cmd)
        else:
            cmd = 'start {0}'.format(cmd)
    if prints_shell_cmd or prints_cmd:
        print(cwd, cmd)
    if cwd:
        os.chdir(cwd)
    returncode = os.system(cmd)
    return returncode


def get_version(shell_result: str, seperator: str, keyword: str, iter_diff: int):
    assert iter_diff > 0, iter_diff
    version_info_list = shell_result.split(seperator)
    version = None
    for i, info in enumerate(version_info_list):
        if info == keyword:
            version = version_info_list[i+iter_diff].strip()
            break
    assert version, shell_result
    return version


def get_os_version(os_name: str):
    return get_version(platform.platform(), '-', os_name, 1)


def get_gnome_version():
    return get_version(get_shell_stdout('gnome-terminal --version'), ' ', 'Terminal', 1) if is_linux else None


# os_version = get_os_version('centos' if is_centos else ) if is_linux else None
gnome_terminal_version = get_gnome_version()

python_cmd = 'python3' if is_linux else 'python'


def make_nested_string(text: str, quote: str):
    text = text.replace('\\', '\\\\')
    text = text.replace('\'', '\\\'')
    text = text.replace('\"', '\\\"')
    text = f'{quote}{text}{quote}'
    return text


def execute_shell_cmd_with_terminal(cmd: str, cwd: Path, custom_python_cmd: str = None, custom_util_home: Path = None):
    if custom_python_cmd:
        python_cmd = custom_python_cmd
    else:
        python_cmd = 'python3'
    if custom_util_home:
        util_home = custom_util_home
    else:
        util_home = get_path_from_os_env('RVX_UTIL_HOME')
    assert util_home

    cmd = make_nested_string(cmd, '\"')
    cmd = f'{python_cmd} {util_home}/run_cmd.py {cmd}'
    if is_linux:
        if gnome_terminal_version.split('.')[0] == '2':
            cmd = make_nested_string(cmd, '\"')
            cmd = f'gnome-terminal -e {cmd}'
        else:
            cmd = f'gnome-terminal -- {cmd}'
    else:
        cmd = f'start {cmd}'

    execute_shell_cmd(cmd, cwd, background=False)


def make_cmd_sudo(cmd: str, passwd: str):
    assert is_linux
    modified_cmd = f'echo \"{passwd}\" | sudo -S -v; sudo {cmd}' if passwd else f'sudo {cmd}'
    return modified_cmd


def remove(element: Path):
    if element.is_file():
        os.remove(element)
    elif element.is_dir():
        shutil.rmtree(element)
    else:
        assert 0


def remove_file(file: Path):
    file.unlink(missing_ok=True)


def remove_files(dir: Path, pattern: str):
    if pattern and dir.is_dir():
        if is_linux:
            run_shell_cmd('rm -rf {0}'.format(pattern), dir)
        else:
            run_shell_cmd('del /s /f /q {0}'.format(pattern), dir)


def remove_directory(dir: Path):
    if dir.is_dir():
        shutil.rmtree(dir)


def copy_directory(src_dir: Path, dst_dir: Path):
    assert src_dir.is_dir()
    shutil.copytree(str(src_dir), str(dst_dir), dirs_exist_ok=True)


# overwrite
def copy_file(src_file: Path, dst_file: Path):
    assert src_file.is_file(), src_file
    shutil.copy(str(src_file), str(dst_file))


def move_files(src_path: Path, dst_path: Path):
    if src_path.is_file():
        if is_linux:
            run_shell_cmd(f'mv {src_path} {dst_path}')
        else:
            run_shell_cmd(f'move /y {src_path} {dst_path}')
    elif src_path.is_dir():
        assert dst_path.is_dir(), dst_path
        for element in src_path.glob('**/*'):
            relative_path = element.relative_to(src_path)
            dst_hier_path = dst_path / relative_path
            if is_linux:
                run_shell_cmd(f'mv {element} {dst_hier_path}')
            else:
                run_shell_cmd(f'move /y {element} {dst_hier_path}')


def move_directory(src_dir: Path, dst_dir: Path):
    copy_directory(src_dir, dst_dir)
    remove_directory(src_dir)


def is_equal_file(a_file: Path, b_file: Path):
    return cmp(a_file, b_file)


def make_executable(path: Path):
    if path.is_file():
        if is_linux:
            run_shell_cmd(f'chmod +x {path.name}', path.parent)


def extract_file(target_file: Path):
    assert target_file.is_file(), target_file
    output_dir = target_file.parent

    target_filename = target_file.name
    if target_filename.endswith('.gz') or target_filename.endswith('.tgz'):
        run_shell_cmd(f'tar -xzf ./{target_filename}', output_dir)
    elif target_filename.endswith('.tar') or target_filename.endswith('.xz'):
        run_shell_cmd(f'tar -xf ./{target_filename}', output_dir)
    else:
        assert 0, target_file


def get_path_from_os_env(os_var_name: str, default_value=None, must: bool = False):
    x = os.environ.get(os_var_name)
    if x:
        x = Path(x)
    else:
        assert not must, os_var_name
        x = default_value
    return x


def download_url(url: str, output_dir: Path = None, output_file=None):
    assert (not output_dir) or (not output_file)
    assert not (output_dir and output_file)

    if output_file:
        result_file = output_file
    else:
        result_file = output_dir / url.split('/')[-1]

    assert result_file.parent.is_dir(), result_file
    request.urlretrieve(url, result_file)


def get_dir_list(path: Path):
    dir_list = []
    if path.is_dir():
        for child in path.iterdir():
            if not child.is_dir():
                continue
            if child.name.startswith('.'):
                continue
            dir_list.append(child.name)
    return dir_list


def is_process_running(process_name: str) -> bool:
    assert is_linux
    result = subprocess.run(["pgrep", process_name], stdout=subprocess.DEVNULL)
    return result.returncode == 0


def get_makefile_var(var: str, cwd: Path):
    out = subprocess.check_output(
        ["make", f"print-{var}"], cwd=cwd, text=True).strip()
    return out.split("=", 1)[1]


def check_argument_number(argv: list, num: int):
    assert len(argv) == (num+2), argv


if __name__ == '__main__':
    cmd = sys.argv[1]
    if cmd == 'append_text':
        assert len(sys.argv) == 4, sys.argv
        file = sys.argv[2]
        text_to_append = sys.argv[3]
        text_to_append = text_to_append.replace('\\n', '\n')
        path = Path(file).resolve()
        assert path.is_file(), path
        original_text = path.read_text()
        extended_text = original_text + text_to_append
        path.write_text(extended_text)
    elif cmd == 'dir_list':
        check_argument_number(sys.argv, 1)
        dir_list = get_dir_list(Path(sys.argv[2]))
        print(' '.join(dir_list))
    elif cmd == 'cp_file':
        check_argument_number(sys.argv, 2)
        copy_file(Path(sys.argv[2]), Path(sys.argv[3]))
    elif cmd == 'mkdir':
        check_argument_number(sys.argv, 1)
        path = Path(sys.argv[2])
        path.mkdir(parents=True, exist_ok=True)
    elif cmd == 'rm_file':
        check_argument_number(sys.argv, 1)
        remove_file(Path(sys.argv[2]))
    elif cmd == 'rm_files':
        check_argument_number(sys.argv, 2)
        remove_files(Path(sys.argv[2]), sys.argv[3])
    elif cmd == 'rm_dir':
        check_argument_number(sys.argv, 1)
        remove_directory(Path(sys.argv[2]))
    else:
        assert 0, sys.argv[1]
