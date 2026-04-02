import os
import io
import argparse
from pathlib import *

if __name__ == '__main__':
  # parse argument
  parser = argparse.ArgumentParser(description='Converting Input Files')
  parser.add_argument('-input', '-i', nargs='+', help='input files')
  parser.add_argument('-output', '-o', help='output directory')
  parser.add_argument('-rom', '-r', help='rom name')
  args = parser.parse_args()

  assert args.input
  assert args.output
  assert args.rom
  output_directory = Path(args.output)
  assert output_directory.is_dir()
  rom_name = args.rom.upper()
  if rom_name=='NONE':
    rom_name = ''

  for input_file in args.input:
    input_path = Path(input_file)
    assert input_path.is_file(), input_path
    design_name = input_path.name.replace('.','_') + '_file'

    fakefile_chunk_name = f'{design_name}_fakefile_chunk'

    # header
    line_list = []
    line_list.append(f'#ifndef __{design_name.upper()}_H__')
    line_list.append(f'#define __{design_name.upper()}_H__')
    line_list.append('')
    line_list.append('#include \"ervp_fakefile.h\"')
    line_list.append('')
    #line_list.append(f'extern FAKEFILE* {design_name};')
    line_list.append('')
    line_list.append(f'#endif')

    output_path = output_directory / f'{design_name}.h'
    output_path.write_text('\n'.join(line_list))

    # body
    line_list = []
    line_list.append(f'#include <stdint.h>')
    line_list.append('#include <stddef.h>')
    line_list.append(f'#include \"ervp_variable_allocation.h\"')
    line_list.append(f'#include \"ervp_malloc.h\"')
    line_list.append(f'#include \"{design_name}.h\"')
    line_list.append(f'#ifndef GENERATE_HEX_USING_GCC')
    line_list.append(f'#include \"platform_info.h\"')
    if rom_name:
      line_list.append(f'#ifdef INCLUDE_{rom_name}')
      line_list.append(f'#include \"{rom_name.lower()}_memorymap.h\"')
      line_list.append(f'#endif')
    line_list.append(f'#endif')
    line_list.append('')
    line_list.append(f'const uint8_t {design_name}_raw[] ALIGNED_DATA BIG_DATA = {{')
    line_list.append(','.join([ hex(x) for x in input_path.read_bytes() ] ))
    line_list.append('};')
    line_list.append('')
    line_list.append(f'static char {design_name}_filename[] = \"{input_path.name}\";')
    line_list.append(f'static fakefile_chunk_t {fakefile_chunk_name};')
    line_list.append(f'static FAKEFILE {design_name};')
    line_list.append('')
    line_list.append(f'#ifdef USE_FAKEFILE')
    line_list.append(f'static void __attribute__ ((constructor)) construct_{design_name}()')
    line_list.append('{')
    line_list.append(f'\tfakefile_chunk_init_(&{fakefile_chunk_name});')
    line_list.append(f'\t{fakefile_chunk_name}.data = (uint8_t*){design_name}_raw;')
    line_list.append(f'\t{fakefile_chunk_name}.size = sizeof({design_name}_raw);')
    line_list.append(f'\t{fakefile_chunk_name}.current_size = sizeof({design_name}_raw);')
    #line_list.append(f'\t{design_name} = (FAKEFILE*)malloc(sizeof(FAKEFILE));')
    line_list.append(f'\tfakefile_init_(&{design_name});')
    line_list.append(f'\t{design_name}.status = FILE_STATUS_READ_ONLY;')
    line_list.append(f'\t{design_name}.name = {design_name}_filename;')
    line_list.append(f'\t{design_name}.head = &{fakefile_chunk_name};')
    line_list.append(f'\tfakefile_dict_add(&{design_name});')
    line_list.append('};')

    line_list.append(f'static void __attribute__ ((destructor)) destruct_{design_name}()')
    line_list.append('{')
    #line_list.append(f'\tfree({design_name});')
    line_list.append('};')
    line_list.append('#endif')

    output_path = output_directory / f'{design_name}.c'
    output_path.write_text('\n'.join(line_list))
