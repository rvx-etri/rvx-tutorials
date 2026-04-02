## ****************************************************************************
## ****************************************************************************
## Copyright SoC Design Research Group, All rights reserved.    
## Electronics and Telecommunications Research Institute (ETRI)
##
## THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE 
## WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS 
## TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE 
## REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL 
## SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE, 
## IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE 
## COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.
## ****************************************************************************
## 2019-01
## Kyuseung Han (han@etri.re.kr)
## ****************************************************************************
## ****************************************************************************

import os
import argparse
import shutil
from pathlib import Path

if __name__ == '__main__':

	parser = argparse.ArgumentParser(description='Gathering Header Files')
	parser.add_argument('-output', '-o', help='an output directory')

	args = parser.parse_args()
	assert args.output

	header_list_file = Path('./header_list.txt').resolve()
	os.system('make header_list HEADER_LIST_FILE={0}'.format(header_list_file))
	include_path_list = header_list_file.read_text().split(' ')
	header_file_list = []
	for include_path in include_path_list:
		for (path, dir_list, file_list) in os.walk(include_path[2:]):
			for file_name in file_list:
				if file_name[-2:]=='.h':
					header_file_list.append(os.path.join(path, file_name))

	if os.path.isdir(args.output):
		shutil.rmtree(args.output)
	os.makedirs(args.output)
	for header_file in header_file_list:
		shutil.copy2(header_file,args.output)
	os.remove(header_list_file)
