#import warnings
warnings.filterwarnings('ignore')
import numpy as np
import sys
import jason
import argparse
import os

def run(vsa):
  config = {}

  pwd_dir  = os.path.dirname(os.path.realpath(__file__))
  if not os.path.exists(pwd_dir + '/output'):
    os.makedirs(pwd_dir + '/output')

  # top.cpp
  generate_tb(pwd_dir + 'output/top.cpp', vsa, config)

  # common_header.h
  generate_header(pwd_dir + 'output/common_header_U%s.h' %(vsa['KERNEL_ID']), vsa, config)

  # PE.cpp
  generate_PE(pwd_dir + 'output/PE_U%s.cpp' %(vsa['KERNEL_ID']), vsa, config)
  
  # DF_engine.cpp
  generate_DF(pwd_dir + 'output/DF_U%s.cpp' %(vsa['KERNEL_ID']), vsa, config)

  # DC_engine.cpp
  generate_DC(pwd_dir + 'output/DC_U%s.cpp' %(vsa['KERNEL_ID']), vsa, config)

  # DF_loader.cpp
  generate_DF_loader(pwd_dir + 'output/DF_loader_U%s.cpp' %(vsa['KERNEL_ID']), vsa, config)

  # DC_loader.cpp
  generate_DC_loader(pwd_dir + 'output/DC_loader_U%s.cpp' %(vsa['KERNEL_ID']), vsa, config)

if __name__ == "__main__":

  parser = argparse.ArgumentParser(description="Generate HLS C code from VSA")
  parser.add_argument('-i', '--input', metavar='INPUT', required=True, help='input virtual systolic array descriptor')

  args = parser.parse_args()

  run(args.input)