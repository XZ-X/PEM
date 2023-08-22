import re

def parse_dyn_cfg(filename, key=""):
  SEP = "######"
  file = open(filename)
  data_str = ' '.join(file.readlines())
  data_str = data_str.strip("{").strip("}")
  items = data_str.split(SEP)
  data_str = None
  key = key.strip()
  check = key.strip() != ""  
  for f in items:
    if check:
      if key not in f:
        continue
    func = eval("{" + f + "}")
    if len(func) == 0:
      continue
    yield func

invisible_patterns = re.compile('[^!-~]+')

def parse_sem(filename, key=""):
  SEP = "######"
  file = open(filename, errors='ignore', encoding='ascii')
  data_str = ' '.join(file.readlines())
  data_str = data_str.strip("{").strip("}")
  items = data_str.split(SEP)
  data_str = None
  key = key.strip()
  check = key.strip() != ""  
  for f in items:
    if check:
      if key not in f:
        continue
    f = invisible_patterns.sub(' ', f).strip()
    func = eval("{" + f + "}")
    if len(func) == 0:
      continue
    yield func