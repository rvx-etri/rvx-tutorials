import os
import argparse

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description='calculate hex file size')
	parser.add_argument('-input', '-i', nargs='+', help='input hex files')
	parser.add_argument('-output', '-o', help='output file name')

	args = parser.parse_args()
	#print(args)

	assert args.input
	assert args.output

	# input 
	hex_size_list = []
	max_hex_size = 0

	for input_file_name in args.input:
		if os.path.isfile(input_file_name):
			file_contents = open(input_file_name,'r').read()
			file_contents = file_contents.replace('\r','')
			line_list = file_contents.split('\n')
		
			hex_size  = 0
			for line in line_list:
				if line:
					hex_size = hex_size + 1
			if max_hex_size < hex_size:
				max_hex_size = hex_size
		else:
			hex_size = 0
		prefix = os.path.splitext(os.path.basename(input_file_name))[0]
		hex_size_list.append((prefix,hex_size))
	
	# output
	output_dir, output_filename = os.path.split(args.output)

	if not os.path.isdir(output_dir):
		os.makedirs(output_dir)

	with open(args.output,'w') as f:
		for name, hex_size in hex_size_list:
			if hex_size!=0:
				f.write('\n`define USE_%s' % name.upper())
			f.write('\n`define %s_HEX_SIZE %d' % (name.upper(),hex_size))
		f.write('\n`define HEX_SIZE %d' % max_hex_size)
