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
# 2019-06-14
# Kyuseung Han (han@etri.re.kr)
# ****************************************************************************
# ****************************************************************************

import argparse
import os
from pathlib import Path

from os_util import *

def update_makefile(dir:Path):
	execute_shell_cmd('make update_makefile &', dir)

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Update Makefile')
	parser.add_argument('-path', '-p', help='path')
	args = parser.parse_args()

	assert args.path
	base_path = Path(args.path).resolve()

	for path in base_path.glob('**/Makefile'):
		makefile_contents = path.read_text()
		if 'template_makefile.mh' in makefile_contents:
			print(path)
			update_makefile(path.parent)
