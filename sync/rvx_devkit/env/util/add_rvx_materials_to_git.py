import argparse
import re

from pathlib import Path

import import_util
import import_engine

from os_util import *
from rvx_config import *


def does_makefile_use_rvx_engine(path: Path):
    makefile_path = path / 'Makefile'
    if not makefile_path.is_file():
        return None

    content = makefile_path.read_text(encoding='utf-8', errors='ignore')
    return 'rvx_make2engine.mh' in content


def extract_makefile_type(path: Path):
    makefile_path = path / 'Makefile'
    if not makefile_path.is_file():
        return None

    content = makefile_path.read_text(encoding='utf-8', errors='ignore')
    pattern = re.compile(r'Makefile\.([^.]+)\.template[ \t]*?$', re.MULTILINE)
    matches = pattern.findall(content)

    if len(matches) > 1:
        raise ValueError(
            f"Multiple Makefile.XXXXX.template patterns found: {makefile_path}")
    elif len(matches) == 1:
        return matches[0]
    else:
        return None


class RvxGitadd():
    @staticmethod
    def execute_add(materials: str, is_force_all: bool, cwd: Path):
        git_option = '-A'
        if is_force_all:
            git_option += 'f'
        execute_shell_cmd(cmd=f'git add {git_option} {materials}', cwd=cwd)

    @staticmethod
    def execute_remove(materials: str, cwd: Path):
        execute_shell_cmd(cmd=f'git rm -rf {materials}', cwd=cwd)

    @staticmethod
    def add_materials(target_path: Path, rvx_config: RvxConfig, makefile_type: str):
        temp_set = frozenset((None, 'local_setup',))
        is_terminal = True
        # print(makefile_type)
        if makefile_type in temp_set:
            return is_terminal

        print(target_path, ':', makefile_type)
        material_list = []
        if makefile_type == 'home':
            material_list.append(('Makefile rvx_init.mh README.md', True, False, False))
            material_list.append(('.gitignore .gitmodules', True, False, True))
            if rvx_config.is_mini:
                material_list.append(('rvx_config.mh', True, False, False))
                material_list.append(('imp_class_info', True, False, False))
                material_list.append(('rvx_special_ip', True, True, False))
                if rvx_config.is_frozen:
                    material_list.append(('sync', True, False, True))
                    material_list.append(
                        (rvx_config.freeze_tag_path, True, False, False))
            else:
                pass
            is_terminal = False
        elif makefile_type == 'platform_base':
            material_list.append(('Makefile', True, False, False))
            is_terminal = False
        elif makefile_type == 'platform':
            material_list.append(('Makefile', True, False, False))
            material_list.append(('*.xml', True, False, False))
            if rvx_config.is_frozen:
                material_list.append(('arch', True, False, False))
            is_terminal = False
        elif makefile_type == 'app_base':
            material_list.append(('.', True, False, False))
        elif makefile_type == 'imp_fpga':
            if rvx_config.is_frozen:
                material_list.append(('.', True, False, False))
        else:
            assert 0, (target_path, makefile_type)

        for materials, is_add, if_exist, is_force_all in material_list:
            if not is_add:
                RvxGitadd.execute_remove(materials, target_path)
            elif not if_exist:
                RvxGitadd.execute_add(materials, is_force_all, target_path)
            else:
                for material in materials.split(' '):
                    if (target_path/material).exists():
                        RvxGitadd.execute_add(material, is_force_all, target_path)
        return is_terminal


if __name__ == '__main__':
    # argument
    parser = argparse.ArgumentParser(description='RVX gitadd')
    parser.add_argument('-home', help='home directory')
    parser.add_argument('-type', '-t', help='directory type')
    parser.add_argument('-path', '-p', help='target path')
    parser.add_argument('--recursively', action="store_true")

    args = parser.parse_args()

    target_path = Path(args.path).resolve().absolute()
    assert target_path.is_dir(), target_path

    makefile_type = extract_makefile_type(target_path)
    if not makefile_type:
        makefile_type = args.type

    if makefile_type:
        home_path = Path(args.home).resolve().absolute()
        assert home_path.is_dir(), home_path
        rvx_config = RvxConfig(home_path)

        is_terminal = RvxGitadd.add_materials(
            target_path, rvx_config, makefile_type)

        if args.recursively and not is_terminal:
            subdir_list = []
            for subdir in target_path.iterdir():
                if subdir.is_dir() and does_makefile_use_rvx_engine(subdir):
                    subdir_list.append(subdir)
            # print(subdir_list)
            for subdir in subdir_list:
                returncode = execute_shell_cmd(
                    cmd='make --no-print-directory gitadd_rc', cwd=subdir)
                if returncode != 0:
                    break
                # returncode = execute_shell_cmd(cmd='make --no-print-directory gitadd_rc', cwd=subdir)
                # assert returncode!=0, returncode
