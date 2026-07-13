import argparse
import os
from pathlib import Path
import import_util

from os_util import *
from re_util import *


def analyze_build_log(contents: str):
    line_list = contents.split('\n')
    start_index = len(line_list)
    for i, line in enumerate(line_list):
        if 'error:' in line:
            start_index = i
            break
    if start_index == len(line_list):
        return ''
    size = 0
    for i, line in enumerate(line_list[start_index+1:]):
        if '|' not in line:
            size = i + 1
            break
    return '\n'.join(line_list[start_index:start_index+size])


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Analyze Build logs')
    parser.add_argument('-input', '-i', type=str, help='input build log file')
    parser.add_argument('-output', '-o', type=str, help='output file')
    parser.add_argument('-op', type=str, nargs='*', help='operation')
    args = parser.parse_args()

    input_file_path = Path(args.input)
    assert input_file_path.is_file(), input_file_path
    input_contents = input_file_path.read_text()

    contents = '\n[[BUILD FAILURE]]\n\n'
    contents += analyze_build_log(input_contents)

    if args.output:
        output_path = Path(args.output)
        assert output_path.parent.is_dir(), output_path
        output_path.write_text(contents)
    else:
        print(contents)
