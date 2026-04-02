import os
import io
import argparse
from pathlib import *
from PIL import Image

def convert_rgb565(r:int, g:int, b:int):
  return (((r>>3)<<11) | ((g>>2)<<5) | (b>>3))

if __name__ == '__main__':
  # parse argument
  parser = argparse.ArgumentParser(description='Converting Images')
  parser.add_argument('-input', '-i', nargs='+', help='input image files')
  parser.add_argument('-output', '-o', help='output directory')
  parser.add_argument('-width', help='width')
  parser.add_argument('-height', help='height')
  parser.add_argument('-format', '-f', help='format')
  parser.add_argument('-rom', '-r', help='rom name')
  args = parser.parse_args()

  assert args.input
  assert args.output
  assert args.width
  assert args.height
  assert args.rom
  assert args.format
  output_directory = Path(args.output)
  assert output_directory.is_dir()
  image_width = int(args.width)
  image_height = int(args.height)
  rom_name = args.rom.upper()
  if rom_name=='NONE':
    rom_name = ''

  image_format = args.format.upper()
  assert image_format=='RGB_565_PACKED' or image_format=='JPEG', image_format

  for input_file in args.input:
    pixel_array = []
    input_path = Path(input_file)
    assert input_path.is_file(), input_path
    image_name = input_path.stem

    image = Image.open(input_path)
    image = image.resize((image_width,image_height))
    if image_format=='RGB_565_PACKED':
      image = image.convert('RGB')
      bmp_path = output_directory / (input_path.stem + '_temp.bmp')
      image.save(bmp_path)
      for y in range(image.height):
        for x in range(image.width):
          r,g,b = image.getpixel((x,y))
          rgb565 = convert_rgb565(r,g,b)
          pixel_array.append(rgb565&0xff)
          pixel_array.append(rgb565>>8)
    elif image_format=='JPEG':
      jpeg_path = output_directory / (input_path.stem + '_temp.jpg')
      image.save(jpeg_path)
      contents = jpeg_path.read_text()
      assert 0
    # var
    ervp_image_name = f'{image_name}_image'
    # header
    line_list = []
    line_list.append(f'#ifndef __{ervp_image_name.upper()}_H__')
    line_list.append(f'#define __{ervp_image_name.upper()}_H__')
    line_list.append('')
    line_list.append('#include \"ervp_image.h\"')
    line_list.append('')
    line_list.append(f'extern const unsigned char* {ervp_image_name}_array;')
    line_list.append(f'extern const int {ervp_image_name}_width;')
    line_list.append(f'extern const int {ervp_image_name}_height;')
    line_list.append(f'extern const int {ervp_image_name}_byte_size;')
    line_list.append(f'#ifndef GENERATE_HEX_USING_GCC')
    line_list.append(f'extern ErvpImage* {ervp_image_name};')
    line_list.append(f'#endif')

    line_list.append('')
    line_list.append(f'#endif')

    output_path = output_directory / f'{ervp_image_name}.h'
    output_path.write_text('\n'.join(line_list))

    # c
    mmap_name = f'MMAP_{rom_name}_{ervp_image_name.upper()}'
    use_rom_name = f'USE_{rom_name}'
    line_list = []
    line_list.append('#include <stddef.h>')
    line_list.append(f'#include \"ervp_variable_allocation.h\"')
    line_list.append(f'#include \"ervp_malloc.h\"')
    line_list.append(f'#include \"{ervp_image_name}.h\"')
    line_list.append(f'#ifndef GENERATE_HEX_USING_GCC')
    line_list.append(f'#include \"platform_info.h\"')
    if rom_name:
      line_list.append(f'#ifdef INCLUDE_{rom_name}')
      line_list.append(f'#include \"{rom_name.lower()}_memorymap.h\"')
      line_list.append(f'#endif')
    line_list.append(f'#endif')
    line_list.append('')
    line_list.append(f'const unsigned char {ervp_image_name}_raw[] ALIGNED_DATA BIG_DATA = {{')
    line_list.append(','.join([ str(x) for x in pixel_array ] ))
    line_list.append('};')
    line_list.append(f'const unsigned char* {ervp_image_name}_array = {ervp_image_name}_raw;')
    line_list.append(f'const int {ervp_image_name}_width = {image_width};')
    line_list.append(f'const int {ervp_image_name}_height = {image_height};')
    line_list.append(f'const int {ervp_image_name}_byte_size = sizeof({ervp_image_name}_raw);')
    line_list.append(f'#ifndef GENERATE_HEX_USING_GCC')
    line_list.append(f'ErvpImage* {ervp_image_name} = NULL;')
    line_list.append(f'static void __attribute__ ((constructor)) construct_{ervp_image_name}()')
    line_list.append('{')
    #line_list.append(f'\t{ervp_image_name} = (ErvpImage*)malloc(sizeof(ErvpImage));')
    #line_list.append(f'\timage_init_structure({ervp_image_name}, {ervp_image_name}_width, {ervp_image_name}_height, IMAGE_FMT_{args.format});')
    line_list.append(f'\t{ervp_image_name} = image_alloc_wo_internals({ervp_image_name}_width, {ervp_image_name}_height, IMAGE_FMT_{args.format});')
    line_list.append(f'\t{ervp_image_name}->is_window = 1;')
    line_list.append(f'\timage_decide_stride({ervp_image_name}, 0);')
    if rom_name:
      line_list.append(f'#ifdef {use_rom_name}')
      line_list.append(f'\t{ervp_image_name}->addr[0] = (void*){mmap_name};')
      line_list.append(f'#else')
      line_list.append(f'\t{ervp_image_name}->addr[0] = (void*){ervp_image_name}_raw;')
      line_list.append(f'#endif')
    else:
      line_list.append(f'\t{ervp_image_name}->addr[0] = (void*){ervp_image_name}_raw;')
    line_list.append('};')
    line_list.append(f'static void __attribute__ ((destructor)) destruct_{ervp_image_name}()')
    line_list.append('{')
    line_list.append(f'\tfree({ervp_image_name});')
    line_list.append('};')
    line_list.append(f'#endif')

    output_path = output_directory / f'{ervp_image_name}.c'
    output_path.write_text('\n'.join(line_list))
