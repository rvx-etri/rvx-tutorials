import os
import argparse
import math

def change_endian(value):
	byte_list = [value[i:i+2] for i in range(0, len(value), 2)]
	byte_list.reverse()
	result = ''.join(byte_list)
	return result

if __name__ == '__main__':

	parser = argparse.ArgumentParser(description='generate verilog ROM header from a hex file')
	parser.add_argument('-hex_file', '-hex', help='a hex file')
	parser.add_argument('-width', '-w', help='memory width')

	parser.add_argument('-prefix', '-p', type=str, help='prefix of output files')
	parser.add_argument('-output_directory', '-od', help='output directory')
	parser.add_argument('-start_addr', help='start memory addr')
	parser.add_argument('-start_index', help='start memory index')
	parser.add_argument('-hex_be', action='store_true', help='input hex is big endian type')
	parser.add_argument('-be', action='store_true', help='stored as big endian type')

	args = parser.parse_args()
	#print(args)

	assert args.hex_file
	assert args.width

	width = int(args.width)
	if not args.start_index and not args.start_addr:
		index = 0
	elif start_addr:
		bw_byte_offset = int(math.log(float(args.width), 2));
		assert math.exp(2,bw_byte_offset)==args.width
		index = args.start_addr >> bw_byte_offset
	else:
		index = args.start_index

	if args.prefix:
		prefix = args.prefix
	else:
		prefix = 'rom'

	if args.output_directory:
		output_directory = args.output_directory
	else:
		output_directory = '.'

	diffrent_endian = args.hex_be ^ args.be

	capacity_file_name = '%s_capacity' % prefix
	contents_file_name = '%s_contents' % prefix

	##############################################################

	with open(args.hex_file,'r') as f:
		file_contents = f.read()
	assert file_contents

	file_contents = file_contents.replace('\r','')
	line_list = file_contents.split('\n')

	if os.path.isdir(output_directory)==False:
		os.makedirs(output_directory)
	depth  = 0
	with open(os.path.join(output_directory,contents_file_name)+'.vh','w') as f:
		f.write('always@(*)')
		f.write('\nbegin : gen_%s_contents' % prefix)
		f.write('\nrom_data = 0;')
		f.write('\ncase(rom_index)')
		for line in line_list:
			if line:
				if diffrent_endian:
					line = change_endian(line)
				f.write('\n%d: rom_data = %d\'h %s;' % (index, width, line))
				index  = index + 1
				depth = depth + 1
		f.write('\nendcase')
		f.write('\nend')

	capacity = math.ceil(depth*int(args.width)/8)

	with open(os.path.join(output_directory,capacity_file_name)+'.vh','w') as f:
		f.write('`ifndef __%s_H__' % capacity_file_name.upper())
		f.write('\n`define __%s_H__' % capacity_file_name.upper())
		f.write('\n\n`define %s %d' % (capacity_file_name.upper(),capacity))
		f.write('\n\n`endif')
