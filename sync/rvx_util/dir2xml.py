# ****************************************************************************
# ****************************************************************************
# Copyright SoC Design Research Group, All rights reserved.
# Electronics and Telecommunications Research Institute (ETRI)
# 
# THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE
# WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS
# TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE
# REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL
# SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE,
# IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE
# COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.
# ****************************************************************************
# 2018-12-14
# Kyuseung Han (han@etri.re.kr)
# ****************************************************************************
# ****************************************************************************

import argparse
import os
from xml.sax.saxutils import escape as xml_escape

def get_tab(num):
	return '\t' * num

def list2str(list, seperator='\n', prefix='', suffix=''):
	merged_str = ''
	for element in list:
		sub_list = element.split('\n')
		for sub_element in sub_list:
			if not merged_str:
				merged_str = prefix + sub_element + suffix
			else:
				merged_str += seperator + prefix + sub_element + suffix
	return merged_str

def dir2xml(path):
	line_list = []
	line_list.append('<{0}>'.format(os.path.basename(path)))
	for item in os.listdir(path):
		subdir = os.path.join(path, item)
		if os.path.isdir(subdir):
			sub_result = dir2xml(subdir)
			line_list.append(sub_result)
	
	line_list.append('</{0}>'.format(os.path.basename(path)))

	result = list2str(line_list,'\n',get_tab(1),'')
	return result


if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Directory Structure to XML')
	parser.add_argument('-directory', '-d', help='base directory')
	parser.add_argument('-output', '-o', help='output file')
	
	args = parser.parse_args()
	assert args.directory
	assert args.output

	xml_contents = '<ds>\n' + dir2xml(os.path.abspath(args.directory)) + '\n</ds>'

	with open(args.output,'w') as f:
		f.write(xml_contents)
