import math

def get_tab(num):
  return '\t' * num

def list2str(list, seperator='\n', prefix='', suffix=''):
  merged_str = ''
  for element in list:
    if element==None:
      continue
    sub_list = element.split('\n')
    for sub_element in sub_list:
      if not merged_str:
        merged_str = prefix + sub_element + suffix
      else:
        merged_str += seperator + prefix + sub_element + suffix
  return merged_str

def generate_rtl_from_list(list, tab, seperator):
  rtl_list = [x.generate_rtl(tab) for x in list]
  return list2str(rtl_list,seperator,'','')

concatenator = '/'

def concat_hname(*args):
  return concatenator.join(args)

def split_hname(hname):
  return hname.split(concatenator)

def id2str(index:int):
  return f'{index:03}'

def get_percore_name(original:str, index:int):
  return f'{original}_{id2str(index)}'

def get_partial_access(list_variable_name, element_size, index):
  return '{0}[{1}*({2}+1)-1 -:{1}]'.format(list_variable_name,element_size,index)

def is_integer(text):
  try: 
    int(text)
    return True
  except ValueError:
     return False

def get_required_bitwidth_unsigend(value):
  return math.floor(math.log(value,2))+1 if value>0 else 1

def name_generator_decorator(prefix:str,num_digit:int):
  def name_generator():
    index = 0
    while 1:
      yield f'{prefix}{str(index).zfill(num_digit)}'
      index += 1
  return name_generator

def answer2bool(answer:str):
  true_list = ('true', 't', 'yes', 'y')
  false_list = ('false', 'f', 'no', 'n')
  answer = answer.lower()
  if answer in true_list:
    value = True
  elif answer in false_list:
    value = False
  else:
    assert 0
  return value

def round_up_int(value: int, step:int):
  result = value
  if step > 1:
    residue = value % step
    if residue:
      result += (step-residue)
  return result

def round_up_power2(value: int):
  result = 1
  while result < value:
    result <<=1
  return result

def check_if_para_value_is_fixed(value:str):
  result = False
  try:
    eval(value)
    result = True
  except:
    pass
  if '(' in value and ')' in value:
    result = True
  return result
        
class ConsistencyChecker():
  def __init__(self):
    self.db = {}
    
  def check(self, key, value):
    stored = self.db.get(key)
    is_new = (stored==None)
    if is_new:
      self.db[key] = value
    else:
      assert (value==stored), (value, stored)
    return is_new
