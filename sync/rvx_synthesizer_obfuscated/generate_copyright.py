import datetime

def gen_copyright(line_conmment_indicator, version):
  text = ''
  text +=   '%s ****************************************************************************' % line_conmment_indicator
  text += '\n%s ****************************************************************************' % line_conmment_indicator
  text += '\n%s Copyright SoC Design Research Group, All rights reserved.' % line_conmment_indicator
  text += '\n%s Electronics and Telecommunications Research Institute (ETRI)' % line_conmment_indicator
  text += '\n%s ' % line_conmment_indicator
  text += '\n%s THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE' % line_conmment_indicator
  text += '\n%s WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS' % line_conmment_indicator
  text += '\n%s TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE' % line_conmment_indicator
  text += '\n%s REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL' % line_conmment_indicator
  text += '\n%s SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE,'  % line_conmment_indicator
  text += '\n%s IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE' % line_conmment_indicator
  text += '\n%s COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.' % line_conmment_indicator
  text += '\n%s ****************************************************************************' % line_conmment_indicator
  text += '\n%s %s' % (line_conmment_indicator, version)
  text += '\n%s Kyuseung Han (han@etri.re.kr)' % line_conmment_indicator
  text += '\n%s ****************************************************************************' % line_conmment_indicator
  text += '\n%s ****************************************************************************' % line_conmment_indicator
  return text;

def gen_copyright_in_verilog(version=datetime.date.today().isoformat()):
  return gen_copyright('//',version)

def gen_copyright_in_c(version=datetime.date.today().isoformat()):
  return gen_copyright('//',version)

def gen_copyright_in_tcl(version=datetime.date.today().isoformat()):
  return gen_copyright('#',version)
