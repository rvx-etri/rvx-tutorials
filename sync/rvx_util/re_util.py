import re

def wordize(reexp):
	return r'\b'+reexp+r'\b'

def memorize(reexp):
	return r'('+reexp+')'

def exist_or_not(reexp):
	return r'(?:'+reexp+')?'

reexp_blank = r'[ \n\t]+'
reexp_blank_or_not = exist_or_not(reexp_blank)
reexp_int = r'-?[0-9]+'
reexp_int_extended = r'[0-9-+*/]+'
reexp_float = reexp_int + '\.' + reexp_int
reexp_identifier = r'[A-Za-z_][A-Za-z0-9_]*'
reexp_exp = r'[A-Za-z0-9_!%^&*+-=/:()\[\]"]+'
reexp_anything = r'.+?'
rexp_shrink_multiline = r'\\\n'

re_identifier = re.compile(memorize(wordize(reexp_identifier)))
re_shrink_multiline = re.compile(rexp_shrink_multiline, re.MULTILINE)

# reference: http://stackoverflow.com/questionsi2319019/using-regex-to-remove-comments-from-source-files
def remove_comments(string):
	string = re.sub(re.compile("/\*.*?\*/",re.DOTALL ) ,"" ,string) # remove all occurrence streamed comments (/*COMMENT */) from string
	string = re.sub(re.compile("//.*?\n" ) ,"" ,string) # remove all occurrence singleline comments (//COMMENT\n ) from string
	return string
