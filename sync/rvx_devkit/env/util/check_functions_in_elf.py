import os
import sys
import argparse
import string

from pathlib import *
from elftools.elf.elffile import ELFFile


def generate_function_set_from_txt(path: Path) -> frozenset:
    assert path.is_file(), f"File not found: {path}"

    with path.open("r", encoding="utf-8") as f:
        lines = [line.strip() for line in f if line.strip()]
    return frozenset(lines)


def generate_function_set_from_elf(elf_file: str) -> frozenset:
    with open(args.elf, 'rb') as file:
        elf_class = ELFFile(file)


def generate_function_set_from_elf(elf_file: str) -> frozenset:
    function_list = []
    with open(elf_file, 'rb') as file:
        elf = ELFFile(file)

        # .symtab과 .dynsym 둘 다 확인
        for section_name in ['.symtab', '.dynsym']:
            symtab = elf.get_section_by_name(section_name)
            if symtab:
                for symbol in symtab.iter_symbols():
                    if symbol['st_info']['type'] == 'STT_FUNC' and symbol.name:
                        function_list.append(symbol.name)

        # DWARF 정보에서도 함수 이름 추가 (있을 경우)
        if elf.has_dwarf_info():
            dwarf_info = elf.get_dwarf_info()
            for CU in dwarf_info.iter_CUs():
                for DIE in CU.iter_DIEs():
                    if DIE.tag == 'DW_TAG_subprogram':
                        name_attr = DIE.attributes.get('DW_AT_name')
                        if name_attr:
                            name = name_attr.value.decode(
                                'utf-8', errors='ignore')
                            function_list.append(name)

    return frozenset(function_list)


if __name__ == '__main__':
    # parse the command-line arguments and invoke MemoryContent
    parser = argparse.ArgumentParser(description='Generating Hex File')
    parser.add_argument('-elf', help='elf file')
    parser.add_argument('-error', '-e', help='error function list file')
    parser.add_argument('-warning', '-w', help='warning function list file')
    parser.add_argument('-output', '-o', help='output file')
    args = parser.parse_args()

    assert args.elf

    if args.error:
        error_path = Path(args.error)
        assert error_path.is_file(), error_path
        predefined_error_set = generate_function_set_from_txt(error_path)
    else:
        predefined_error_set = frozenset()

    if args.warning:
        warning_path = Path(args.warning)
        assert warning_path.is_file(), warning_path
        predefined_warning_set = generate_function_set_from_txt(warning_path)
    else:
        predefined_warning_set = frozenset()

    assert args.output
    output_path = Path(args.output).resolve().absolute()
    if output_path.is_dir():
        output_path = output_path / 'check_result.txt'
    else:
        assert output_path.parent.is_dir(), output_path
        output_path.unlink(missing_ok=True)
    # print(output_path)

    used_set = generate_function_set_from_elf(args.elf)
    found_error_set = used_set & predefined_error_set
    found_warning_set = used_set & predefined_warning_set

    line_list = []
    if found_error_set:
        line_list.append('List of functions that cause errors if called:')
        line_list += found_error_set
    if found_warning_set:
        line_list.append('List of functions that require caution if called:')
        line_list += found_warning_set
    if line_list:
        output_path.write_text('\n'.join(line_list))
    else:
        output_path.touch()
