import json
import argparse
import numpy as np
import os
import re

def run(input_code, vsa_file, app_name):
  '''Generate the vsa after the C code pass.

  This function takes in C code and the initial VSA generated from the polyhedral pass.
  '''

  with open(vsa_file, 'r') as f:
    vsa = json.loads(f.read())

  # Parse the C file and fulfil the required fields
  vsa_c_pass(input_code, vsa, app_name)

  # Write out the VSA
  vsa_dump('design_desp.json', vsa)

def vsa_dump(output_file, vsa):
  '''Dump out the vsa'''
  with open(output_file, 'w') as f:
    json.dump(vsa, f, indent = 2)

def get_params(content):
  '''Get the parameter list in the C code'''
  params = {}
  for line_id in range(len(content)):
    line = content[line_id]
    if line.find('// global parameters - start') >= 0:
      start_line_id = line_id
    if line.find('// global parameters - end') >= 0:
      end_line_id = line_id
      break
  # Parse the defined macros
  for line_id in range(start_line_id + 1, end_line_id):
    line = content[line_id]
    line_split = line.split(' ')
    param_name = line_split[1]
    param_val = int(line_split[2])
    params[param_name] = param_val
  return params

def get_data_types(content):
  '''Get the data types in the C code'''
  data_types = []
  for line_id in range(len(content)):
    line = content[line_id]
    if line.find('// type definition - start') >= 0:
      start_line_id = line_id
    if line.find('// type definition - end') >= 0:
      end_line_id = line_id
  # Parse the data types
  for line_id in range(start_line_id + 1, end_line_id):
    line = content[line_id]
    data_t = line.split(' ')[1]
    data_types.append(data_t)

  return data_types

def data_type_to_width(data_types):
  '''calculate data widths for the data types in the list'''

  data_widths = []
  data_width_dict = {
    "int": 32,
    "float": 32,
      }
  for data_t in data_types:
    if data_width_dict[data_t] != None:
      data_widths.append(data_width_dict[data_t])
    else:
      p = re.compile(r'ap_uint<|ap_uint<')
      m = p.search(data_t)
      if m == None:
        sys.exit("[ERROR] Unsupported data types! Supported types: float, int, ap_[u]int\n")
      new_s = data_t[m.end():-1]
      new_s_split = new_s.split(',')
      data_widths.append(new_s_split[0])

  return data_widths

def get_sw_kernel_code(content):
  '''get the code in the main function'''
  sw_kernel_code = []
  for line_id in range(len(content)):
    line = content[line_id].strip()
    if line.find('int main()') >= 0:
      start_line_id = line_id
    if line.find('return 0;') >= 0:
      end_line_id = line_id
      break
  for line_id in range(start_line_id + 1, end_line_id):
    sw_kernel_code.append(content[line_id].strip())

  return sw_kernel_code

def get_arr_decls(content):
  '''Get the array declerations in the C code'''
  arr_decls = []
  for line_id in range(len(content)):
    line = content[line_id]
    if line.find('// buffers - start') >= 0:
      start_line_id = line_id
    if line.find('// buffers - end') >= 0:
      end_line_id = line_id
  # Parse the array declerations
  for line_id in range(start_line_id + 1, end_line_id):
    line = content[line_id].strip(';\n').strip()
    p = re.compile(r'\w+')
    m = p.match(line)
    new_s = line[m.end():]
    data_t = new_s # data_type

    p = re.compile(r'[\w\[\]]+')
    m = p.search(new_s)
    arr_ref = m.group() # arr_ref
    arr_name = m.group().split('[')[0] # arr_name

    arr_decl = {}
    arr_decl['DATA_TYPE'] = data_t
    arr_decl['ARR_REF'] = arr_ref
    arr_decl['ARR_NAME'] = arr_name
    arr_decls.append(arr_decl)

  return arr_decls

def vsa_c_pass(input_f, vsa, app):
  '''Parse the input C code and complete the VSA fields

  These fields are to be completed in this function:
  - DATA_TYPE
  - DATA_WIDTH
  - BUS_WIDTH
  - KERNEL_ID
  - APP_NAME
  - INIT_VALUE
  - SW_KERNEL_CODE
  - FIXED_EN
  - OP_REF
  - RES_REF
  - PARAMETERS
  - CHANNEL_DEPTH
  - OP_PE_SIMD_WIDTH
  '''

  '''Parse the C code'''
  content = []
  with open(input_f, 'r') as f:
    for line in f.readlines():
      content.append(line)
    params = get_params(content)
    data_types = get_data_types(content)

  # CHANNEL_DEPTH
  vsa['CHANNEL_DEPTH'] = 2 # Use the default value

  # KERNEL_ID
  vsa['KERNEL_ID'] = 1 # Use the default value

  # BUS_WIDTH
  vsa['BUS_WIDTH'] = [512 for i in range(len(data_types))] # Use the default value

  # FIXED_EN
  fixed_en = 1
  for data_t in data_types:
    if data_t == "float" or data_t == "int":
      fixed_en = 0
      break
  vsa['FIXED_EN'] = fixed_en

  # DATA_TYPE
  vsa['DATA_TYPE'] = []
  for data_t in data_types:
    vsa['DATA_TYPE'].append(data_t)

  # DATA_WIDTH
  vsa['DATA_WIDTH'] = []
  data_widths = data_type_to_width(data_types)
  for data_w in data_widths:
    vsa['DATA_WIDTH'].append(data_w)

  # APP_NAME
  vsa['APP_NAME'] = app

  # INIT_VALUE
  vsa['INIT_VALUE'] = 0 # TODO: temporary solution

  # SW_KERNEL_CODE
  sw_kernel_code = get_sw_kernel_code(content)
  vsa['SW_KERNEL_CODE'] = sw_kernel_code

  # PARAMETERS
  vsa['PARAMETERS'] = {}
  for param_name in params:
    vsa['PARAMETERS'][param_name] = params[param_name]

  # OP_REF
  vsa['OP_REF'] = []
  arr_decls = get_arr_decls(content)
  for op_name in vsa['OP_NAME']:
    for arr_decl in arr_decls:
      if arr_decl['ARR_NAME'] == op_name:
        vsa['OP_REF'].append(arr_decl['ARR_REF'])

  # RES_REF
  vsa['RES_REF'] = []
  for res_name in vsa['RES_NAME']:
    for arr_decl in arr_decls:
      if arr_decl['ARR_NAME'] == res_name:
        vsa['RES_REF'].append(arr_decl['ARR_REF'])

  # OP_PE_SIMD_WIDTH
  vsa['OP_PE_SIMD_WIDTH'] = []
  for idx in range(len(vsa['OP_NAME'])):
    op_pe_simd_width = int(vsa['DATA_WIDTH'][idx]) * int(vsa['SIMD_FACTOR'])
    vsa['OP_PE_SIMD_WIDTH'].append(op_pe_simd_width)

if __name__ == "__main__":

  parser = argparse.ArgumentParser(description='Generate VSA descriptors for applications.')
  parser.add_argument('-i', '--input', metavar='INPUT', required=True, help='input C file to be analyzed')
  parser.add_argument('-f', '--feature', metavar='FEATURE', required=True, help='VSA generated from the polyhedral analysis pass')
  parser.add_argument('-a', '--app', metavar='APP', required=True, help='application name (to be depricated soon)')

  args = parser.parse_args()

  run(args.input, args.feature, args.app)
