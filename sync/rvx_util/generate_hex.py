#-------------------------------------------------------------------------------
# generate_hex.py
#
# A clone of 'generate_hex' in Python, based on the pyelftools library
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
import os, sys
import argparse
import string

# For running from development directory. It should take precedence over the
# installed pyelftools.
sys.path.insert(0, '.')

from elftools.elf.elffile import ELFFile

class MemoryContent(object):
  def __init__(self, elf_class, baseaddr, max_size):
    self.elf_class = elf_class
    self.start_addr = baseaddr
    self.size = max_size
    self.end_addr = self.start_addr + self.size
    self.output = sys.stdout
    self.data_list = []
    self.data_size = 0

  def read(self, stores_bss:bool):
    nonempty_endaddr = self.start_addr
    for section in self.elf_class.iter_sections():
      if not (section['sh_flags'] & 2): # allocated
        continue
      if (not stores_bss) and section.name.endswith('bss'):
        continue
      sh_addr = section['sh_addr']
      sh_size = section['sh_size']
      sh_end = sh_addr + sh_size
      if sh_addr >= self.end_addr:
        continue
      if sh_end <= self.start_addr:
        continue
      assert sh_addr >= self.start_addr
      assert sh_addr >= nonempty_endaddr
      #
      self.data_size += sh_addr-nonempty_endaddr
      self.data_list.append(bytes(sh_addr-nonempty_endaddr))
      #
      self.data_size += sh_size
      if(section['sh_type'] == 'SHT_PROGBITS'):
        self.data_list.append(section.data())
      elif(section['sh_type'] == 'SHT_INIT_ARRAY'):
        self.data_list.append(section.data())
      else:
        self.data_list.append(bytes(sh_size))
      #
      nonempty_endaddr = sh_end
      #
      assert self.data_size <= self.size, (self.data_size, self.size)

  def write(self, hex_file_name):
    outfile = open(hex_file_name,"w")

    cnt_in_a_line = 0
    for data in self.data_list:
      for a_data in data:
        outfile.write('%02x' % a_data)
        cnt_in_a_line += 1
        if cnt_in_a_line == 4:
          outfile.write('\n')
          cnt_in_a_line = 0

    # chunks of 4 bytes per line
    last_line_bytes = self.data_size % 4
    if last_line_bytes > 0:
      for i in range(last_line_bytes, 4): 
        outfile.write('00')

    outfile.close()

  def display_section_headers(self, show_heading=True):
    """ Display the ELF section headers
    """
    elfheader = self.elf_class.header
    if show_heading:
      self._emitline('There are %s section headers, starting at offset %s' % (
        elfheader['e_shnum'], self._format_hex(elfheader['e_shoff'])))

    if self.elf_class.num_sections() == 0:
      self._emitline('There are no sections in this file.')
      return

    self._emitline('\nSection Header%s:' % (
      's' if elfheader['e_shnum'] > 1 else ''))

    # Different formatting constraints of 32-bit and 64-bit addresses
    #
    if self.elf_class.elfclass == 32:
      self._emitline('  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al')
    else:
      self._emitline('  [Nr] Name              Type             Address           Offset')
      self._emitline('       Size              EntSize          Flags  Link  Info  Align')

    # Now the entries
    #
    for nsec, section in enumerate(self.elf_class.iter_sections()):
      self._emit('  [%2u] %-17.17s %-15.15s ' % (
        nsec, section.name, describe_sh_type(section['sh_type'])))

      if self.elf_class.elfclass == 32:
        self._emitline('%s %s %s %s %3s %2s %3s %2s' % (
          self._format_hex(section['sh_addr'], fieldsize=8, lead0x=False),
          self._format_hex(section['sh_offset'], fieldsize=6, lead0x=False),
          self._format_hex(section['sh_size'], fieldsize=6, lead0x=False),
          self._format_hex(section['sh_entsize'], fieldsize=2, lead0x=False),
          describe_sh_flags(section['sh_flags']),
          section['sh_link'], section['sh_info'],
          section['sh_addralign']))
      else: # 64
        self._emitline(' %s  %s' % (
          self._format_hex(section['sh_addr'], fullhex=True, lead0x=False),
          self._format_hex(section['sh_offset'],
            fieldsize=16 if section['sh_offset'] > 0xffffffff else 8,
            lead0x=False)))
        self._emitline('     %s  %s %3s    %2s   %3s   %s' % (
          self._format_hex(section['sh_size'], fullhex=True, lead0x=False),
          self._format_hex(section['sh_entsize'], fullhex=True, lead0x=False),
          describe_sh_flags(section['sh_flags']),
          section['sh_link'], section['sh_info'],
          section['sh_addralign']))

  def _emit(self, s=''): 
    """ Emit an object to output
    """
    self.output.write(str(s))

  def _emitline(self, s=''):
    """ Emit an object to output, followed by a newline
    """
    self.output.write(str(s).rstrip() + '\n')

  def _format_hex(self, addr, fieldsize=None, fullhex=False, lead0x=True,
          alternate=False):
    """ Format an address into a hexadecimal string.

      fieldsize:
        Size of the hexadecimal field (with leading zeros to fit the
        address into. For example with fieldsize=8, the format will
        be %08x
        If None, the minimal required field size will be used.

      fullhex:
        If True, override fieldsize to set it to the maximal size
        needed for the elfclass

      lead0x:
        If True, leading 0x is added

      alternate:
        If True, override lead0x to emulate the alternate
        hexadecimal form specified in format string with the #
        character: only non-zero values are prefixed with 0x.
        This form is used by generate_hex.
    """
    if alternate:
      if addr == 0:
        lead0x = False
      else:
        lead0x = True
        fieldsize -= 2

    s = '0x' if lead0x else ''
    if fullhex:
      fieldsize = 8 if self.elf_class.elfclass == 32 else 16
    if fieldsize==None:
      field = '%x'
    else:
      field = '%' + '0%sx' % fieldsize
    return s + field % addr

#-------------------------------------------------------------------------------
if __name__ == '__main__':
  # parse the command-line arguments and invoke MemoryContent
  parser = argparse.ArgumentParser(description='Generating Hex File')
  parser.add_argument('-elf', help='elf file name')
  parser.add_argument('-sram', type=str, nargs='+', help='SRAM address size')
  parser.add_argument('-dram', type=str, nargs='+', help='DRAM address size')
  parser.add_argument('-output', '-o', help='Output directory')
  parser.add_argument('-prefix', '-p', help='Filename prefix')
  parser.add_argument('--stores_bss', action="store_true", help='if bss is included or not')
  args = parser.parse_args()

  assert args.elf  

  if args.output:
    output_directory = args.output
  else:
    output_directory = '.'

  #elf_file_name = os.path.basename(args.elf)
  #file_prefix = elf_file_name[0:len(elf_file_name)-4]
  #sram_file_name = os.path.join(output_directory,'{0}.sram.hex'.format(file_prefix))
  #dram_file_name = os.path.join(output_directory,'{0}.dram.hex'.format(file_prefix))
  
  if args.prefix:
    prefix = args.prefix
  else:
    prefix = ''
  sram_file_name = os.path.join(output_directory, prefix+'sram.hex')
  dram_file_name = os.path.join(output_directory, prefix+'dram.hex')

  with open(args.elf, 'rb') as file:
    elf_class = ELFFile(file)

    # sram
    if args.sram:
      sram_baseaddr = int(args.sram[0], 16)
      sram_size = int(args.sram[1], 16)
      sram_memory_content = MemoryContent(elf_class, sram_baseaddr, sram_size)
      sram_memory_content.read(args.stores_bss)
      sram_memory_content.write(sram_file_name)

    # dram 
    if args.dram:
      dram_baseaddr = int(args.dram[0], 16)
      dram_size = int(args.dram[1], 16)
      dram_memory_content = MemoryContent(elf_class, dram_baseaddr, dram_size)
      dram_memory_content.read(args.stores_bss)
      dram_memory_content.write(dram_file_name)
