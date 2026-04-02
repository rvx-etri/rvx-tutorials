import os, sys

def get_util_path():
  util_path = os.environ.get('RVX_MINI_HOME')
  if util_path:
    util_path = f'{util_path}/rvx_util'
  else:
    util_path = os.environ.get('RVX_UTIL_HOME')
  assert util_path
  return util_path

sys.path.append(get_util_path())
