import os
from pathlib import Path

is_allowed_extension = None

def gather_file_list(source_list, extension:list=None, with_base:bool=False):
  if extension:
    allowed_extension_set = frozenset(extension)
    is_allowed_extension = lambda x: x.suffix in allowed_extension_set
  else:
    disallowed_extension_set = frozenset(('.swp',))
    is_allowed_extension = lambda x: x.suffix not in disallowed_extension_set

  if not source_list:
    source_list = []
  elif type(source_list)==str:
    source_list = (source_list,)
  gathered_file_list = []
  for element in source_list:
    element_path = Path(os.path.expandvars(element)).resolve()
    if element_path.is_file():
      if is_allowed_extension(element_path):
        if with_base:
          gathered_file_list.append((element_path, element_path.parent))
        else:
          gathered_file_list.append(element_path)
    elif element_path.is_dir():
      for subelement in element_path.glob('**/*'):
        if subelement.is_file():
          if is_allowed_extension(subelement):
            if with_base:
              gathered_file_list.append((subelement, element_path))
            else:
              gathered_file_list.append(subelement)
  return gathered_file_list
