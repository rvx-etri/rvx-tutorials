import re
import import_util
from re_util import *

verilog_key_wordset = frozenset(('input','output','inout','wire','reg','define','ifdef','ifndef'))

# reference: http://stackoverflow.com/questionsi2319019/using-regex-to-remove-comments-from-source-files
def remove_comments(string):
	string = re.sub(re.compile("/\*.*?\*/",re.DOTALL ) ,"" ,string) # remove all occurrence streamed comments (/*COMMENT */) from string
	string = re.sub(re.compile(r"^//.*?\n",re.MULTILINE ),"" ,string)
	string = re.sub(re.compile("//.*?\n",re.MULTILINE ),"\n" ,string) # remove all occurrence singleline comments (//COMMENT\n ) from string
	string = re.sub(re.compile(r'\n\n\n+',re.MULTILINE ),"\n\n" ,string)
	return string

reexp_include = r'`(?:include|INCLUDE)' + reexp_blank + r'\"' + memorize(reexp_identifier + r'\.' + reexp_identifier)+ r'\"'
re_include = re.compile(reexp_include)

reexp_define = r'`(?:define|DEFINE)' + reexp_blank + memorize(wordize(reexp_identifier))
re_define = re.compile(reexp_define)

reexp_macro = r'`' + memorize(r'(?:define|DEFINE|ifndef|IFNDEF|ifdef|IFDEF|undef|UNDEF)') + reexp_blank + memorize(wordize(reexp_identifier))
re_macro = re.compile(reexp_macro)

reexp_lpara = r'(?:localparam|LOCALPARAM)'  + exist_or_not(reexp_blank + memorize(reexp_identifier)) + reexp_blank + memorize(wordize(reexp_identifier)) + reexp_blank_or_not + r'='
re_lpara = re.compile(reexp_lpara,re.DOTALL)

reexp_para = r'(?:parameter|PARAMETER)' + reexp_blank + memorize(wordize(reexp_identifier))
re_para = re.compile(reexp_para,re.DOTALL)

reexp_def_signal = memorize(exist_or_not('(?:input|INPUT|output|OUTPUT)')) + reexp_blank + memorize('(?:wire|Wire|reg|REG)') + reexp_blank + \
				memorize(exist_or_not(r'\[' + r'[^\[\]]+' + r'\]')) + \
				memorize(r'[^;]+') + \
				r';'
re_def_signal = re.compile(reexp_def_signal,re.DOTALL)

reexp_def_port = memorize(r'(?:input|INPUT|output|OUTPUT|inout|INOUT)') + reexp_blank + memorize(exist_or_not('(?:wire|Wire|reg|REG)')) + reexp_blank_or_not + \
				memorize(exist_or_not(r'\[' + r'[^\[\]]+' + r'\]')) + reexp_blank_or_not + \
				memorize(r'[^;\[\]]+') + \
				r';'

re_def_port = re.compile(reexp_def_port,re.DOTALL)

reexp_header_prolog = r'`(?:ifndef|IFNDEF)' + reexp_blank + memorize(wordize(reexp_identifier)) + reexp_blank + r'`(?:define|DEFINE)' + reexp_blank + memorize(wordize(reexp_identifier))
re_header_prolog= re.compile(reexp_header_prolog, re.MULTILINE)

reexp_use_identifier = memorize(r'[^`.]') + memorize(wordize(reexp_identifier))
re_use_identifier = re.compile(reexp_use_identifier)

reexp_module_define = wordize(r'(?:module|MODULE)') + reexp_blank + memorize(wordize(reexp_identifier))
re_module_define = re.compile(reexp_module_define, re.DOTALL)

reexp_instance = memorize(wordize(reexp_identifier)) + \
			memorize( \
			reexp_blank_or_not + exist_or_not(r'#\(' + r'[^;]+' + r'\)') + reexp_blank_or_not) + \
			memorize(wordize(reexp_identifier)) + \
			memorize(
			reexp_blank_or_not + r'\(' + \
			reexp_blank_or_not + r'\.' + reexp_blank_or_not + reexp_identifier + reexp_blank_or_not + r'\(' + \
			r'[^;]+' + \
			r'\)' + reexp_blank_or_not + r';')
re_instance = re.compile(reexp_instance, re.DOTALL)
