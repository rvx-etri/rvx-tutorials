import argparse
import os
from pathlib import Path
import import_util

from os_util import *
from gather_files import *


def execute_shell_cmd(cmd_list, ignore_error: bool = False):
    if type(cmd_list) == str:
        cmd = cmd_list
    else:
        cmd = ' '.join(cmd_list)
    exit_code = os.system(cmd)
    if not ignore_error:
        assert exit_code == 0, cmd_list[-1]
    else:
        return (exit_code == 0)


def not_allowed_assertion(simulator):
    assert 0, f'Not allowed behavior for {simulator}'


def execute_shell_cmd_separately(common_cmd_list: list, file_list: list, each: bool):
    max_number_of_file_at_once = 50
    number_of_file_at_once = 1 if each else max_number_of_file_at_once
    for i in range(0, len(file_list), number_of_file_at_once):
        cmd_list = common_cmd_list + \
            [str(x) for x in file_list[i:i+number_of_file_at_once]]
        execute_shell_cmd(cmd_list)


def get_hdlsim_filename(tool: str, action: str) -> str:
    action_list = action.split('.')
    result = None
    assert len(action_list) == 2, action
    if 0:
        pass
    elif tool == 'ncsim' or tool == 'xcelium':
        if 0:
            pass
        elif action_list[0] == 'compile':
            if 0:
                pass
            elif action_list[1] == 'log':
                result = 'xmvlog.log'
            else:
                assert 0
        elif action_list[0] == 'elaborate':
            if 0:
                pass
            elif action_list[1] == 'log':
                result = 'xmelab.log'
            else:
                assert 0
        elif action_list[0] == 'run' or action_list[0] == 'debug':
            if 0:
                pass
            elif action_list[1] == 'log':
                result = 'xmsim.log'
            else:
                assert 0
    elif tool == 'modelsim' or tool == 'questasim':
        if 0:
            pass
        elif action_list[0] == 'compile':
            assert 0
        elif action_list[0] == 'elaborate':
            assert 0
        elif action_list[0] == 'run' or action_list[0] == 'debug':
            if 0:
                pass
            elif action_list[1] == 'log':
                result = 'qtsim.log'
            else:
                assert 0

    assert result, (tool, action)
    return result


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Simulate HDL files')
    parser.add_argument('-source', '-s', type=str, nargs='*',
                        help='source file or directory')
    parser.add_argument('-include', '-i', type=str,
                        nargs='*', help='include directory')
    parser.add_argument('-operation', '-op', type=str, help='operation')
    parser.add_argument('-simulator', '-smt', type=str, help='simulator')
    parser.add_argument('-option', type=str, help='tool option from user')
    parser.add_argument('-define', type=str, nargs='*', help='define')
    parser.add_argument('-top', type=str, help='top module name')
    parser.add_argument('--each', action="store_true", help='compile each')
    args = parser.parse_args()

    assert args.simulator in (
        'ncsim', 'modelsim', 'xcelium', 'questasim'), 'Please check .rvx_tool_config'

    verilog_file_list = gather_file_list(args.source, ('.v', '.vh', '.sv'))
    vhdl_file_list = gather_file_list(args.source, ('.vhd', '.vhdl'))

    pkg_verilog_file_list = []
    normal_verilog_file_list = []
    pkg_vhdl_file_list = []
    normal_vhdl_file_list = []
    for verilog_file in verilog_file_list:
        if verilog_file.stem.endswith('_pkg'):
            pkg_verilog_file_list.append(verilog_file)
        else:
            normal_verilog_file_list.append(verilog_file)

    for vhdl_file in vhdl_file_list:
        if vhdl_file.stem.endswith('_pkg'):
            pkg_vhdl_file_list.append(vhdl_file)
        else:
            normal_vhdl_file_list.append(vhdl_file)

    compile_correctly_file = Path('./compile_correctly')
    compile_wrong_file = Path('./compile_wrong')

    if args.operation == 'compile':
        remove_file(compile_correctly_file)
        compile_wrong_file.touch()
        if args.simulator == 'ncsim' or args.simulator == 'xcelium':
            common_cmd_list = []
            if args.simulator == 'ncsim':
                common_cmd_list.append('ncvlog')
            elif args.simulator == 'xcelium':
                common_cmd_list.append('xmvlog')
            if args.option:
                common_cmd_list.append(args.option)
            else:
                common_cmd_list.append('-sv -lin -message')
            common_cmd_list.append(f'-define USE_{args.simulator.upper()}')
            common_cmd_list.append(f'-define USE_RECORD_COMMAND')
            if args.define:
                common_cmd_list += [f'-define {x}' for x in args.define]
            if args.include:
                common_cmd_list += [f'-INCDIR {x}' for x in args.include]

            if pkg_verilog_file_list:
                execute_shell_cmd_separately(
                    common_cmd_list, pkg_verilog_file_list, False)
            if normal_verilog_file_list:
                execute_shell_cmd_separately(
                    common_cmd_list, normal_verilog_file_list, args.each)
            if pkg_vhdl_file_list:
                common_cmd_list = []
                if args.simulator == 'ncsim':
                    common_cmd_list.append('ncvhdl')
                elif args.simulator == 'xcelium':
                    common_cmd_list.append('xmvhdl')
                common_cmd_list.append('-V200X -RELAX -smartorder -message')
                execute_shell_cmd_separately(
                    common_cmd_list, pkg_vhdl_file_list, False)
            if normal_vhdl_file_list:
                common_cmd_list = []
                if args.simulator == 'ncsim':
                    common_cmd_list.append('ncvhdl')
                elif args.simulator == 'xcelium':
                    common_cmd_list.append('xmvhdl')
                common_cmd_list.append('-V200X -RELAX -smartorder -message')
                execute_shell_cmd_separately(
                    common_cmd_list, normal_vhdl_file_list, args.each)
        elif args.simulator == 'modelsim' or args.simulator == 'questasim':
            work_dir = Path('./work')
            if not work_dir.is_dir():
                execute_shell_cmd(f'vlib {work_dir}')

            common_cmd_list = []
            common_cmd_list.append(f'vlog -work {work_dir} -sv')

            if args.option:
                common_cmd_list.append(args.option)
            common_cmd_list.append(f'+define+USE_{args.simulator.upper()}')
            if args.define:
                common_cmd_list += [f'+define+{x}' for x in args.define]
            if args.include:
                common_cmd_list += [f'+incdir+{x}' for x in args.include]

            if pkg_verilog_file_list:
                execute_shell_cmd_separately(
                    common_cmd_list, pkg_verilog_file_list, False)
            if normal_verilog_file_list:
                execute_shell_cmd_separately(
                    common_cmd_list, normal_verilog_file_list, args.each)

            common_cmd_list = []
            common_cmd_list.append(f'vcom -work {work_dir} -2008')
            if pkg_vhdl_file_list:
                all_correct = True
                candidate_set = set([str(x) for x in pkg_vhdl_file_list])
                for i in range(len(pkg_vhdl_file_list)):
                    for vhdl_file in set(candidate_set):
                        cmd_list = common_cmd_list + [vhdl_file]
                        if execute_shell_cmd(cmd_list, True):
                            candidate_set.remove(vhdl_file)
                    if len(candidate_set) == 0:
                        break
                assert len(candidate_set) == 0
                cmd_list = common_cmd_list + \
                    [str(x) for x in pkg_vhdl_file_list]
                execute_shell_cmd(cmd_list)
            if normal_vhdl_file_list:
                all_correct = True
                candidate_set = set([str(x) for x in normal_vhdl_file_list])
                for i in range(len(normal_vhdl_file_list)):
                    for vhdl_file in set(candidate_set):
                        cmd_list = common_cmd_list + [vhdl_file]
                        if execute_shell_cmd(cmd_list, True):
                            candidate_set.remove(vhdl_file)
                    if len(candidate_set) == 0:
                        break
                assert len(candidate_set) == 0
                cmd_list = common_cmd_list + \
                    [str(x) for x in normal_vhdl_file_list]
                execute_shell_cmd(cmd_list)
        remove_file(compile_wrong_file)
        compile_correctly_file.touch()

    elif args.operation.startswith('elaborate'):
        assert args.top
        if compile_correctly_file.is_file():
            if args.simulator == 'ncsim' or args.simulator == 'xcelium':
                # elaborate
                cmd_list = []
                if args.simulator == 'ncsim':
                    cmd_list.append('ncelab')
                elif args.simulator == 'xcelium':
                    cmd_list.append('xmelab -MCCODEGEN')
                if args.option:
                    cmd_list.append(args.option)
                else:
                    if not args.operation.endswith('run'):
                        cmd_list.append('-access +rwc')
                    cmd_list.append(
                        '-timescale 1ns/100ps -ntcnotchks -message')
                cmd_list.append(f'worklib.{args.top}')
                execute_shell_cmd(cmd_list)
            elif args.simulator == 'modelsim' or args.simulator == 'questasim':
                pass

    elif args.operation == 'run' or args.operation == 'debug':
        assert args.top
        if compile_correctly_file.is_file():
            if args.simulator == 'ncsim' or args.simulator == 'xcelium':
                sim_script = Path('.') / f'{args.operation}.tcl'
                assert sim_script.is_file(), sim_script
                cmd_list = []
                if args.simulator == 'ncsim':
                    cmd_list.append('ncsim')
                elif args.simulator == 'xcelium':
                    cmd_list.append('xmsim -MCDUMP')
                if args.option:
                    cmd_list.append(args.option)
                else:
                    cmd_list.append(
                        f'-UNBUFFERED -message -LICQUEUE -input {sim_script}')
                cmd_list.append(f'worklib.{args.top}')
                execute_shell_cmd(cmd_list)
            elif args.simulator == 'modelsim' or args.simulator == 'questasim':
                # execute_shell_cmd(f'vopt -debugdb +acc {args.top} -o {args.top}_opt')
                sim_script = Path('.') / f'{args.operation}.do'
                assert sim_script.is_file(), sim_script
                cmd_list = []
                if args.simulator == 'modelsim' or args.simulator == 'questasim':
                    cmd_list.append(f'vsim -c')
                if args.operation == 'debug':
                    cmd_list.append('-debugdb')
                cmd_list.append(f'{args.top} -do {sim_script}')
                log_file_path = Path(
                    '.') / get_hdlsim_filename(args.simulator, f'{args.operation}.log')
                cmd_list.append(f'-l {log_file_path}')
                execute_shell_cmd(cmd_list)

    elif args.operation == 'debug_view':
        assert args.top
        if compile_correctly_file.is_file():
            if args.simulator == 'modelsim' or args.simulator == 'questasim':
                sim_script = Path('./debug.do')
                assert sim_script.is_file(), sim_script
                if args.simulator == 'modelsim' or args.simulator == 'questasim':
                    execute_shell_cmd(
                        f'vsim -debugdb {args.top} -do {sim_script}')

    elif args.operation == 'view':
        if args.simulator == 'ncsim' or args.simulator == 'xcelium':
            cmd_list = []
            cmd_list.append('simvision ./wave/*-1-1.trn &')
            execute_shell_cmd(cmd_list)
        elif args.simulator == 'modelsim' or args.simulator == 'questasim':
            cmd_list = []
            if args.simulator == 'modelsim' or args.simulator == 'questasim':
                cmd_list.append('vsim -view dataset=./vsim.wlf')
            execute_shell_cmd(cmd_list)
        else:
            not_allowed_assertion(args.simulator)
    else:
        assert 0, op
