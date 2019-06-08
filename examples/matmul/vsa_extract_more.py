import json
import argparse
import numpy as np
import os

def code_extract(vsa, code_file):
  type_width = {
    'float': 32,
    'int': 32,
    'char': 8
  }

  vsa['CHANNEL_DEPTH'] = 2  

def vsa_dump(fp, vsa, config):
  with open(fp, 'w') as f:
    json.dump(vsa, f, indent = 2)

def run(vsa_file, code_file):
  config = {}

  with open(vsa_file, 'r') as f:
    vsa = json.loads(f.read())

  vsa = code_extract(vsa, code_file)  

  vsa_dump('vsa_rich.json', vsa, config)

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="Extend the VSA for more features")
  parser.add_argument('-v', '--vsa', metavar="VSA", required=True, help='input VSA')
  parser.add_argument('-c', '--code', metavar='CODE', required=True, help='input code')

  run(args.vsa, args.code) 