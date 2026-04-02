import re

def wordize(reexp):
  return r'\b'+reexp+r'\b'

def memorize(reexp):
  return r'('+reexp+')'

def exist_or_not(reexp):
  return r'(?:'+reexp+')?'

def reexp_or(*exp_list):
  return r'(?:{0})'.format('|'.join(exp_list))

reexp_blank = r'[ \n\t]+'
reexp_blank_or_not = exist_or_not(reexp_blank)
reexp_int = r'-?[0-9]+'
reexp_int_extended = r'[0-9-+*/]+'
reexp_identifier = r'[A-Za-z_][A-Za-z0-9_]*'
reexp_exp = r'[A-Za-z0-9_!%^&*+-=/:()\[\]"]+'
reexp_anything = r'.+?'

re_identifier = re.compile(reexp_identifier)

# reference: http://stackoverflow.com/questionsi2319019/using-regex-to-remove-comments-from-source-files
def remove_comments(string):
  string = re.sub(re.compile("/\*.*?\*/",re.DOTALL ) ,"" ,string) # remove all occurrence streamed comments (/*COMMENT */) from string
  string = re.sub(re.compile("//.*?\n" ) ,"" ,string) # remove all occurrence singleline comments (//COMMENT\n ) from string
  return string

def get_define_list(define_pragma, file_name):
  with open(file_name,'r') as f:
    file_contents = f.read()
  file_contents = remove_comments(file_contents)
  #reexp_define = define_pragma + reexp_blank + memorize(wordize(reexp_identifier)) + exist_or_not(reexp_blank + '\(' + memorize(reexp_anything) + r'\)') + r'$'
  reexp_define = define_pragma + reexp_blank + memorize(wordize(reexp_identifier)) + exist_or_not(reexp_blank + memorize(reexp_exp)) + r'$'
  re_define = re.compile(reexp_define,re.MULTILINE)
  define_list = re_define.findall(file_contents)
  return define_list

def update_expr(text:str, identifier_dict:dict):
  reexp_variable = memorize(exist_or_not(r'`')) + memorize(wordize(reexp_identifier))
  re_variable = re.compile(reexp_variable)
  def __convert(x):
    is_define, identifier = x.groups()
    if is_define:
      result = is_define + identifier
    else:
      after = identifier_dict.get(identifier)
      result = after if after else identifier
    return result
  
  result = text
  while 1:
    before = result
    result = re_variable.sub(__convert, before)
    if result==before:
      break
  return result
