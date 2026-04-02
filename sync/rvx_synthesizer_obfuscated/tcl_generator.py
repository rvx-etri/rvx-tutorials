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
## 2017-02
## Kyuseung Han (han@etri.re.kr)
## ****************************************************************************
## ****************************************************************************

import os

class TclGenerator:
  @staticmethod
  def gen_header_prologue(file_name):
    return ''

  @staticmethod
  def gen_header_epilogue(file_name):
    return ''

  @staticmethod
  def gen_define(name, value, format=None):
    assert type(value)==str, type(value)
    line = None
    if format=='dec':
      line = 'set ::{0} {1}'.format(name,value)
    elif format=='hex':
      line = 'set ::{0} {1}'.format(name,value)
    elif format=='text':
      line = 'set ::{0} {1}'.format(name,value)
    elif format=='exp':
      line = 'set ::{0} {1}'.format(name,value)
    elif format=='str':
      line = 'set ::{0} \"{1}\"'.format(name,value)
    elif format=='bool':
      if value=='True':
        line = 'set ::{0} 1'.format(name)
      else:
        line = '# set ::{0}'.format(name)
    elif format=='lang_tcl':
      line = f'set ::{name} {value}'
    elif format[0:5]=='lang_':
      pass
    else:
      assert 0, format
    return line

  @staticmethod
  def gen_short_comments(text):
    return '# {0}'.format(text)
  
  @staticmethod
  def use_define(name):
    return '${{{0}}}'.format(name)
    #return ('$::%s' % (name))
  
  @staticmethod
  def convert_hex(value,bitwitdh):
    return ('0x%x' % (value))

  @staticmethod
  def gen_include(include_name):
    return 'source {0}.tcl'.format(include_name)
