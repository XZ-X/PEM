import sem_features_pb2
import glob
import branch_info_pb2

def get_results_filename(results_file):
  files = glob.glob(results_file+"*.pb")
  return files

def load_results_large(file_list):
  ret = {}
  for file in file_list:
    fin = open(file, 'rb')
    results = sem_features_pb2.Result()
    results.ParseFromString(fin.read())
    for f in results.funcs:
      ret[f.addr] = f
    fin.close()
    print("\rLoading %s" % file)
  return ret

def load_results(results_file):  
  files = glob.glob(results_file+"*.pb")
  ret = {}
  for file in files:
    fin = open(file, 'rb')
    results = sem_features_pb2.Result()
    results.ParseFromString(fin.read())
    for f in results.funcs:
      ret[f.addr] = f
    fin.close()
    print("\rLoading %s" % file)
  return ret

def load_fname(fname_file):
  fin = open(fname_file, 'r')
  names = eval(" ".join(fin.readlines()))
  fin.close()
  return names

def load_ruleout(ruleout_file):
  fin = open(ruleout_file, 'r')
  names = eval(" ".join(fin.readlines()))
  fin.close()
  return names

def load_branch_info(branch_info_file):
  fin = open(branch_info_file, 'rb')
  branch = branch_info_pb2.Result()
  branch.ParseFromString(fin.read())
  fin.close()
  ret = {}
  for f in branch.funcs:
    ret[f.addr] = f
  return ret
