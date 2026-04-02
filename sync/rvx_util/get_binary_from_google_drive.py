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
## 2020-03-17
## Kyuseung Han (han@etri.re.kr)
## ****************************************************************************
## ****************************************************************************

import os
import argparse
from pathlib import Path

from os_util import *

class RvxBinary():
	def __init__(self, info_dir:Path):
		self.info_dir = info_dir.resolve()
		assert self.info_dir.is_dir(), self.info_dir
		self.script_file = Path(os.environ['RVX_UTIL_HOME']).resolve() / 'fget_google_drive.sh'
		assert self.script_file.is_file()

	def __download_google_drive_file(self, gd_id:str, file_path:Path):
		execute_shell_cmd(f'{self.script_file} {gd_id} {file_path}')

	def get_binary_from_goole_drive(self, target:str, output_dir:Path):
		new_info = None
		if is_windows:
			new_info = self.info_dir / f'{target}_win_info.txt'
			if not new_info.is_file():
				new_info = self.info_dir / f'{target}_all_info.txt'
		elif is_linux:
			if is_centos:
				new_info = self.info_dir / f'{target}_centos_info.txt'
			elif is_ubuntu:
				new_info = self.info_dir / f'{target}_ubuntu_info.txt'
			if not new_info.is_file():
				new_info = self.info_dir / f'{target}_linux_info.txt'
			if not new_info.is_file():
				new_info = self.info_dir / f'{target}_all_info.txt'
		assert new_info.is_file()
		current_info = output_dir / new_info.name
		#
		is_update_required = False
		if not current_info.is_file():
			is_update_required = True
		else:
			get_version = lambda x: x.read_text().split('\n')[0]
			new_version = get_version(new_info)
			current_version = get_version(current_info)
			if new_version!=current_version:
				is_update_required = True
		
		if is_update_required:
			remove_directory(output_dir)
			output_dir.mkdir(parents=True)
			filename, gd_id = new_info.read_text().split('\n')[1:3]
			self.__download_google_drive_file(gd_id, output_dir / filename)
			#
			if filename.endswith('.tar.xz'):
				execute_shell_cmd(f'tar -xf ./{filename}', output_dir)
			elif filename.endswith('.tar.gz'):
				execute_shell_cmd(f'tar -xzf ./{filename}', output_dir)
			elif filename.endswith('.tar'):
				execute_shell_cmd(f'tar -xf ./{filename}', output_dir)
			else:
				assert 0, f'Unknown compressed type: {filename}'
			copy_file(new_info, current_info)

if __name__ =='__main__':
	parser = argparse.ArgumentParser(description='Exporting Files')
	parser.add_argument('-info', '-i', help='info directory')
	parser.add_argument('-target', '-t', help='target binary name')
	parser.add_argument('-output', '-o', help='output directory')
	args = parser.parse_args()

	assert args.info
	assert args.output
	assert args.target
	
	binary = RvxBinary(Path(args.info))
	binary.get_binary_from_goole_drive(args.target, Path(args.output).resolve())
