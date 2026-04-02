import argparse
from pathlib import Path
from os_util import *
from config_file_manager import *
from generate_git_info import *


def add_git_info_to_version_info(version_info, name: str, git_path: Path):
    assert git_path.is_dir(), git_path

    if not name:
        name = get_git_name(git_path)
    version_info.set_attr(f'{name}.path',
                          '/'.join(git_path.parts[-2:]), False)
    git_url = get_git_url(git_path)
    add_url = False
    if 'bitbucket' in git_url:
        add_url = True
    elif 'gitlab' in git_url:
        add_url = True
    elif 'github' in git_url:
        add_url = True
    if add_url:
        version_info.set_attr(f'{name}.url', get_git_url(git_path), False)
    version_info.set_attr(f'{name}.commit', get_git_version(git_path), False)
    version_info.set_attr(f'{name}.date', get_git_date(git_path), False)


def add_name_and_value_info_to_version_info(version_info, name: str, value: str):
    assert name and value
    version_info.set_attr(name, value, False)


def generate_config_from_version_info(vesion_info_file):
    return ConfigFileManager('version_info', vesion_info_file)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Manage Export Info')
    parser.add_argument('-name', '-n', help='name')
    parser.add_argument('-value', '-v', help='value')
    parser.add_argument('-git', '-g', help='git path')
    parser.add_argument('-output', '-o', help='output info file')
    args = parser.parse_args()

    output_file = Path(args.output).resolve()
    assert output_file.parent.is_dir(), output_file
    version_info = ConfigFileManager('version_info', output_file)

    if args.git:
        assert not args.value

        git_path = Path(args.git).resolve()
        add_git_info_to_version_info(version_info, args.name, git_path)

    else:
        assert not args.git
        add_name_and_value_info_to_version_info(
            version_info, args.name, args.value)

    version_info.export_file()
