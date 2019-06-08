import warnings
warnings.filterwarnings('ignore')
import numpy as np
import sys
import json
import argparse
import os
import code_template as tpl

def generate_tb(path, vsa, src_code, config):
  code = []
  code.extend(tpl.disclaimer(vsa, config))
  code.extend(tpl.header_include(vsa, config))

  code.extend(tpl.tb(vsa, src_code, config))

  with open(path, 'w') as f:
    for codeline in code:
      f.write(codeline)

def generate_top(path, vsa, config):
  code = []
  code.extend(tpl.disclaimer(vsa, config))
  code.extend(tpl.header_include(vsa, config))

  code.extend(tpl.top(vsa, config))

  with open(path, 'w') as f:
    for codeline in code:
      f.write(codeline)

def generate_header(path, vsa, config):
  code = []
  code.extend(tpl.disclaimer(vsa, config))

  code.extend(tpl.header(vsa, config))

  with open(path, 'w') as f:
    for codeline in code:
      f.write(codeline)

def generate_DF_loader(path, vsa, config):
  code = []
  code.extend(tpl.disclaimer(vsa, config))
  code.extend(tpl.header_include(vsa, config))
  
  code.extend(tpl.df_loader(vsa, config))

  with open(path, 'w') as f:
    for codeline in code:
      f.write(codeline)

def run(vsa_file, input_file, code_dir):
  config = {}
  config['CODE_DIR'] = code_dir  

  with open(vsa_file, 'r')  as f:
    vsa = json.loads(f.read())

  src_code = []
  with open(input_file, 'r') as f:
    for line in f.readlines():
      src_code.append(line)    

  pwd_dir  = os.path.dirname(os.path.realpath(__file__))
  if not os.path.exists(pwd_dir + '/output'):
    os.makedirs(pwd_dir + '/output')

  config['PREFIX'] = 'U%s_' % (vsa['KERNEL_ID'])

  # tb.cpp  
  generate_tb(pwd_dir + '/output/tb.cpp', vsa, src_code, config)

  # top.cpp
  generate_top(pwd_dir + '/output/top.cpp', vsa, config)

  # common_header.h
  generate_header(pwd_dir + '/output/common_header_U%s.h' %(vsa['KERNEL_ID']), vsa, config)

  # # PE.cpp
  # generate_PE(pwd_dir + 'output/PE_U%s.cpp' %(vsa['KERNEL_ID']), vsa, config)
  
  # # DF_engine.cpp
  # generate_DF(pwd_dir + 'output/DF_U%s.cpp' %(vsa['KERNEL_ID']), vsa, config)

  # # DC_engine.cpp
  # generate_DC(pwd_dir + 'output/DC_U%s.cpp' %(vsa['KERNEL_ID']), vsa, config)

  # DF_loader.cpp
  generate_DF_loader(pwd_dir + '/output/DF_loader_U%s.cpp' %(vsa['KERNEL_ID']), vsa, config)

  # # DC_loader.cpp
  # generate_DC_loader(pwd_dir + 'output/DC_loader_U%s.cpp' %(vsa['KERNEL_ID']), vsa, config)

if __name__ == "__main__":

  parser = argparse.ArgumentParser(description="Generate HLS C code from VSA")
  parser.add_argument('-v', '--vsa', metavar='VSA', required=True, help='input virtual systolic array descriptor')
  parser.add_argument('-i', '--input', metavar='INPUT', required=True, help='input C code')
  parser.add_argument('-m', '--module', metavar='MODULE_CODE', required=True, help='module code directory')

  args = parser.parse_args()

  run(args.vsa, args.input, args.module)