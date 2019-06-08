import warnings
warnings.filterwarnings('ignore')
import numpy as np
import sys
import json
import indentor as indt

def print_line(code, indt, code_text):
  code.append(indt.to_str() + code_text)

def disclaimer(vsa, config):
  code = []
  code.append('/**\n')
  code.append('  * This file is automatically gneerated by PolySA CodeGen.\n')    
  code.append('  * Author: Jie Wang\n')    
  code.append('  */\n')    
  code.append('\n')    

  return code

def header_include(vsa, config):
  code = []
  code.append('#include "common_header_U%s.h"\n\n' %(vsa['KERNEL_ID']))
  
  return code

def tb(vsa, src_code, config):
  code = src_code

  # Add the top function call in-between the scop pragmas
  top_func_call = "top_kernel("
  for op_str in vsa['OP_NAME']:
    top_func_call += op_str
    top_func_call += ', '
  for res_str in vsa['RES_NAME']:
    top_func_call += res_str
  top_func_call += ');\n'
    
  for line_id in range(len(src_code)):
    line = src_code[line_id]
    if line.find('#pragma scop') >= 0:
      scop_start_line_id = line_id
    if line.find('#pragma endscop') >= 0:
      scop_end_line_id = line_id
  
  remove_line_num = scop_end_line_id - scop_start_line_id + 1
  for idx in range(remove_line_num):
    del code[scop_start_line_id]

  code.insert(scop_start_line_id, "  " + top_func_call)
  code.insert(scop_start_line_id, "  " + "// hardware kernel\n")

  return code

def top(vsa, config):
  code = []
  my_indt = indt.Indentor(0)  

  print_line(code, my_indt, 'void %stop_kernel(\n' % (config['PREFIX']))
  my_indt.inc()
  idx = 0
  for op_ref in vsa['OP_REF']:
    print_line(code, my_indt, '%sdata_t%d %s,\n' % (config['PREFIX'], idx, 
      op_ref))
    idx += 1
  for res_ref in vsa['RES_REF']:
    print_line(code, my_indt, '%sdata_t%d %s\n' % (config['PREFIX'], idx, 
      res_ref))
    idx += 1
  my_indt.dec()
  print_line(code, my_indt, ') {\n')
  print_line(code, my_indt, '#pragma HLS DATAFLOW\n')

  # function body
  my_indt.inc()
  print_line(code, my_indt, '// fifo declarations\n')
  # operands and results fifos
  idx = 0
  for op_name in vsa['OP_NAME']:
    for sa_rows in range(vsa['SA_ROWS'] + 1):
      for sa_cols in range(vsa['SA_COLS'] + 1):
        print_line(code, my_indt, 'stream<%sData%dPEChannelType> fifo%d_feed_%d_%d;\n'
          % (config['PREFIX'], idx, idx, sa_rows, sa_cols))
        print_line(code, my_indt, '#pragma HLS STREAM variable=fifo%d_feed_%d_%d depth=%d\n'
          % (idx, sa_rows, sa_cols, vsa['CHANNEL_DEPTH']))
    idx += 1

  for res_name in vsa['RES_NAME']:
    for sa_rows in range(vsa['SA_ROWS'] + 1):
      for sa_cols in range(vsa['SA_COLS'] + 1):
        print_line(code, my_indt, 'stream<%sData%dPEChannelType> fifo%d_collect_%d_%d;\n'
          % (config['PREFIX'], idx, idx, sa_rows, sa_cols))
        print_line(code, my_indt, '#pragma HLS STREAM variable=fifo%d_collect_%d_%d depth=%d\n'
          % (idx, sa_rows, sa_cols, vsa['CHANNEL_DEPTH']))
    idx += 1

  idx = 0
  for op_name in vsa['OP_NAME']:
    feed_num = vsa['OP_ENGINE_NUM'][idx] + 1
    for feed_id in range(feed_num):
      print_line(code, my_indt, 'stream<%sData%dTransferChannelType> fifo%d_transfer_%d;\n'
        % (config['PREFIX'], idx, idx, feed_id))
      print_line(code, my_indt, '#pragma HLS STREAM variable=fifo%d_transfer_%d depth=%d\n'
        % (idx, feed_id, vsa['CHANNEL_DEPTH']))
    idx += 1
  for res_name in vsa['RES_NAME']:
    feed_num = vsa['RES_ENGINE_NUM'][idx - len(vsa['OP_NAME'])] + 1
    for feed_id in range(feed_num):
      print_line(code, my_indt, 'stream<%sData%dTransferChannelType> fifo%d_transfer_%d;\n'
        % (config['PREFIX'], idx, idx, feed_id))
      print_line(code, my_indt, '#pragma HLS STREAM variable=fifo%d_transfer_%d depth=%d\n'
        % (idx, feed_id, vsa['CHANNEL_DEPTH']))  
    idx += 1
  
  # pe fifos
  for sa_rows in range(vsa['SA_ROWS'] + 1):
    for sa_cols in range(vsa['SA_COLS'] + 1):
      idx = 0
      for op_name in vsa['OP_NAME']:
        print_line(code, my_indt, 'stream<%sData%dPEChannelType> PE_%d_%d_fifo%d_local;\n'
          % (config['PREFIX'], idx, sa_rows, sa_cols, idx))
        print_line(code, my_indt, '#pragma HLS STREAM variable=PE_%d_%d_fifo%d_local depth=%d\n'
          % (sa_rows, sa_cols, idx, vsa['CHANNEL_DEPTH']))  
        idx += 1
      for res_name in vsa['RES_NAME']:
        print_line(code, my_indt, 'stream<%sData%dPEChannelType> PE_%d_%d_fifo%d_local;\n'
          % (config['PREFIX'], idx, sa_rows, sa_cols, idx))
        print_line(code, my_indt, '#pragma HLS STREAM variable=PE_%d_%d_fifo%d_local depth=%d\n'
          % (sa_rows, sa_cols, idx, vsa['CHANNEL_DEPTH']))  
        idx += 1      
  print_line(code, my_indt, '\n')
  
  print_line(code, my_indt, '// modules\n')
  idx = 0
  for op_name in vsa['OP_NAME']:
    # loader
    feed_num = vsa['OP_ENGINE_NUM'][idx]
    print_line(code, my_indt, 'DataFeed%dHead(\n' % (idx))
    my_indt.inc()
    print_line(code, my_indt, '%s,\n'% (op_name))
    print_line(code, my_indt, 'fifo%d_transfer_%d\n' % (idx, 0))
    my_indt.dec()
    print_line(code, my_indt, ');\n\n')

    # engine
    for feed_id in range(feed_num):
      feed_id_nxt = feed_id + 1
      if feed_id < feed_num - 1:
        print_line(code, my_indt, '%sDataFeed%dEngine0_wrapper(\n')  
        my_indt.inc()
        print_line(code, my_indt, 'fifo%d_transfer_%d,\n' % (idx, feed_id))
        print_line(code, my_indt, 'fifo%d_transfer_%d,\n' % (idx, feed_id_nxt))
        ch_dir = vsa['OP_CHANNEL_DIR'][idx]
        if ch_dir == 'D':
          print_line(code, my_indt, 'fifo%d_feed_0_%d,\n' % (idx, feed_id))
        elif ch_dir == 'U':
          row_idx = vsa['SA_ROWS'] - 1
          print_line(code, my_indt, 'fifo%d_feed_%d_%d,\n' % (idx, row_idx, feed_id))
        elif ch_dir == 'R':
          print_line(code, my_indt, 'fifo%d_feed_%d_0,\n' % (idx, feed_id))
        elif ch_dir == 'L':
          col_idx = vsa['SA_COLS'] - 1
          print_line(code, my_indt, 'fifo%d_feed_%d_%d,\n' % (idx, feed_id, col_idx))
        print_line(code, my_indt, '%d\n' % (feed_id))
        my_indt.dec()
        print_line(code, my_indt, ');\n\n')
      else:
        print_line(code, my_indt, '%sDataFeed%dEngineLast(\n')  
        my_indt.inc()
        print_line(code, my_indt, 'fifo%d_transfer_%d,\n' % (idx, feed_id))
        # print_line(code, my_indt, 'fifo%d_transfer_%d,\n' % (idx, feed_id_nxt))
        ch_dir = vsa['OP_CHANNEL_DIR'][idx]
        if ch_dir == 'D':
          print_line(code, my_indt, 'fifo%d_feed_0_%d,\n' % (idx, feed_id))
        elif ch_dir == 'U':
          row_idx = vsa['SA_ROWS'] - 1
          print_line(code, my_indt, 'fifo%d_feed_%d_%d,\n' % (idx, row_idx, feed_id))
        elif ch_dir == 'R':
          print_line(code, my_indt, 'fifo%d_feed_%d_0,\n' % (idx, feed_id))
        elif ch_dir == 'L':
          col_idx = vsa['SA_COLS'] - 1
          print_line(code, my_indt, 'fifo%d_feed_%d_%d,\n' % (idx, feed_id, col_idx))
        print_line(code, my_indt, '%d\n' % (feed_id))
        my_indt.dec()
        print_line(code, my_indt, ');\n\n')        
    idx += 1

  # PE modules
  print_line(code, my_indt, '// PE modules\n')  
  for row in range(vsa['SA_ROWS']):
    for col in range(vsa['SA_COLS']):
      # op modules
      idx = 0
      for op_name in vsa['OP_NAME']:
        ch_dir = vsa['OP_CHANNEL_DIR'][idx]
        if (ch_dir == 'D' and row == vsa['SA_ROWS'] - 1) or \
           (ch_dir == 'U' and row == 0) or \
           (ch_dir == 'R' and col == vsa['SA_COLS'] - 1) or \
           (ch_dir == 'L' and col == 0):
          print_line(code, my_indt, '%sop%d_transfer_last_wrapper(\n' % (config['PREFIX'], idx))
          my_indt.inc()
          print_line(code, my_indt, 'fifo%d_feed_%d_%d,\n' % (idx, row, col))
          print_line(code, my_indt, 'PE_%d_%d_fifo%d_local);\n\n' % (row, col, idx))          
          my_indt.dec()
        else:
          print_line(code, my_indt, '%sop%d_transfer_wrapper(\n' % (config['PREFIX'], idx))
          my_indt.inc()
          print_line(code, my_indt, 'fifo%d_feed_%d_%d,\n' % (idx, row, col))                                
          if ch_dir == 'R':
            col_nxt = col + 1
            print_line(code, my_indt, 'fifo%d_feed_%d_%d,\n' % (idx, row, col_nxt))            
          elif ch_dir == 'L':
            col_prv = col - 1
            print_line(code, my_indt, 'fifo%d_feed_%d_%d,\n' % (idx, row, col_prv))                        
          elif ch_dir == 'U':
            row_prv = row - 1
            print_line(code, my_indt, 'fifo%d_feed_%d_%d,\n' % (idx, row_prv, col))                                    
          elif ch_dir == 'D':
            row_nxt = row + 1
            print_line(code, my_indt, 'fifo%d_feed_%d_%d,\n' % (idx, row_nxt, col))            
          print_line(code, my_indt, 'PE_%d_%d_fifo%d_local);\n\n' % (row, col, idx))                                  
          my_indt.dec()
        idx += 1

      # compute
      print_line(code, my_indt, '%scompute_wrapper(\n' % (config['PREFIX']))        
      my_indt.inc()
      idx = 0
      for op_name in vsa['OP_NAME']:        
        print_line(code, my_indt, 'PE_%d_%d_fifo%d_local,\n' % (row, col, idx))          
        idx += 1
      for res_name in vsa['RES_NAME']:
        ch_dir = vsa['RES_CHANNEL_DIR'][idx - len(vsa['OP_NAME'])]
        if ((ch_dir == 'D' or ch_dir == 'U') and (vsa['SA_ROWS'] == 1) or \
            (ch_dir == 'L' or ch_dir == 'R') and (vsa['SA_COLS'] == 1)):
          print_line(code, my_indt, 'fifo%d_collect_%d_%d\n' % (idx, row, col))          
        else:
          print_line(code, my_indt, 'PE_%d_%d_fifo%d_local\n' % (row, col, idx))          
        idx += 1
      my_indt.dec()
      print_line(code, my_indt, ');\n\n')      

      # res_transfer   
      idx = len(vsa['OP_NAME'])
      for res_name in vsa['RES_NAME']:
        ch_dir = vsa['RES_CHANNEL_DIR'][idx - len(vsa['OP_NAME'])]

        if ((ch_dir == 'D' or ch_dir == 'U') and (vsa['SA_ROWS'] == 1) or \
            (ch_dir == 'L' or ch_dir == 'R') and (vsa['SA_COLS'] == 1)) is False:
          if (ch_dir == 'D' and row == 0) or \
             (ch_dir == 'U' and row == vsa['SA_ROWS'] - 1) or \
             (ch_dir == 'R' and col == 0) or \
             (ch_dir == 'L' and col == vsa['SA_COLS'] - 1):
            print_line(code, my_indt, '%sres_transfer_first_wrapper(\n' % (config['PREFIX']))
            my_indt.inc()
            print_line(code, my_indt, 'PE_%d_%d_fifo%d_local,\n' % (row, col, idx))
            print_line(code, my_indt, 'fifo%d_collect_%d_%d,\n' % (idx, row, col))
            print_line(code, my_indt, '%d,\n' % (row))
            print_line(code, my_indt, '%d,\n' % (col))
            my_indt.dec()
            print_line(code, my_indt, ');\n\n')            
          else:
            print_line(code, my_indt, '%sres_transfer_wrapper(\n' % (config['PREFIX']))
            my_indt.inc()
            print_line(code, my_indt, 'PE_%d_%d_fifo%d_local,\n' % (row, col, idx))            
            if ch_dir == 'R':
              col_prv = col - 1
              print_line(code, my_indt, 'fifo%d_collect_%d_%d,\n' % (idx, row, col_prv))              
            elif ch_dir == 'L':
              col_nxt = col + 1
              print_line(code, my_indt, 'fifo%d_collect_%d_%d,\n' % (idx, row, col_nxt))              
            elif ch_dir == 'U':
              row_nxt = row + 1
              print_line(code, my_indt, 'fifo%d_collect_%d_%d,\n' % (idx, row_nxt, col))              
            elif ch_dir == 'D':
              row_prv = row - 1
              print_line(code, my_indt, 'fifo%d_collect_%d_%d,\n' % (idx, row_prv, col))
            
            print_line(code, my_indt, 'fifo%d_collect_%d_%d,\n' % (idx, row, col))
            print_line(code, my_indt, '%d,\n' % (row))
            print_line(code, my_indt, '%d\n' % (col))            
            my_indt.dec()

            print_line(code, my_indt, ');\n\n')            
        idx += 1      

  # data collectors
  idx = len(vsa['OP_NAME'])  
  for ref_name in vsa['RES_NAME']:
    # engines
    feed_num = vsa['RES_ENGINE_NUM'][idx - len(vsa['OP_NAME'])]
    for feed_id in range(feed_num - 1, -1, -1):
      if feed_id == feed_num - 1:
        fifo_out_id = feed_num - 1 - feed_id
        print_line(code, my_indt, '%sDataCollect%dEngineLast(\n')
        my_indt.inc()
        print_line(code, my_indt, 'fifo%d_transfer_%d,\n' % (idx, fifo_out_id))                      
        ch_dir = vsa['RES_CHANNEL_DIR'][idx - len(vsa['OP_NAME'])]
        if ch_dir == 'D':
          row = vsa['SA_ROWS'] - 1
          col = feed_num - 1 - feed_id
          print_line(code, my_indt, 'fifo%d_collect_%d_%d,\n' % (idx, row, col))          
        elif ch_dir == 'U':
          row = 0
          col = feed_num - 1 - feed_id
          print_line(code, my_indt, 'fifo%d_collect_%d_%d,\n' % (idx, row, col))          
        elif ch_dir == 'R':
          row = feed_num - 1 - feed_id
          col = vsa['SA_COLS'] - 1
          print_line(code, my_indt, 'fifo%d_collect_%d_%d,\n' % (idx, row, col))          
        elif ch_dir == 'L':
          row = feed_num - 1 - feed_id
          col = 0
          print_line(code, my_indt, 'fifo%d_collect_%d_%d,\n' % (idx, row, col))          
        print_line(code, my_indt, '%d\n' % (feed_id))        
        my_indt.dec()
        print_line(code, my_indt, ');\n')
      else:
        fifo_out_id = feed_num - 1 - feed_id
        fifo_in_id = fifo_out_id - 1
        print_line(code, my_indt, '%sDataCollect%dEngine0_wrapper(\n' % (config['PREFIX'], idx))
        my_indt.inc()

        print_line(code, my_indt, 'fifo%d_transfer_%d,\n' % (idx, fifo_in_id))
        print_line(code, my_indt, 'fifo%d_transfer_%d,\n' % (idx, fifo_out_id))                    
        ch_dir = vsa['RES_CHANNEL_DIR'][idx - len(vsa['OP_NAME'])]
        if ch_dir == 'D':
          row = vsa['SA_ROWS'] - 1
          col = feed_num - 1 - feed_id
          print_line(code, my_indt, 'fifo%d_collect_%d_%d,\n' % (idx, row, col))          
        elif ch_dir == 'U':
          row = 0
          col = feed_num - 1 - feed_id
          print_line(code, my_indt, 'fifo%d_collect_%d_%d,\n' % (idx, row, col))          
        elif ch_dir == 'R':
          row = feed_num - 1 - feed_id
          col = vsa['SA_COLS'] - 1
          print_line(code, my_indt, 'fifo%d_collect_%d_%d,\n' % (idx, row, col))          
        elif ch_dir == 'L':
          row = feed_num - 1 - feed_id
          col = 0
          print_line(code, my_indt, 'fifo%d_collect_%d_%d,\n' % (idx, row, col))          
        print_line(code, my_indt, '%d\n' % (feed_id))        

        my_indt.dec()
        print_line(code, my_indt, ');\n\n')

    # loaders
    idx = len(vsa['OP_NAME'])  
    for res_name in vsa['RES_NAME']:      
      print_line(code, my_indt, '%sDataCollect%dHead(\n' % (config['PREFIX'], idx))
      my_indt.inc()
      print_line(code, my_indt, '%s,\n' % (res_name))
      feed_id = vsa['RES_ENGINE_NUM'][idx - len(vsa['OP_NAME'])] - 1
      print_line(code, my_indt, 'fifo%d_transfer_%d\n' % (idx, feed_id))
      my_indt.dec()        
      print_line(code, my_indt, ');\n\n')        

    idx += 1    

  my_indt.dec()
  print_line(code, my_indt, '}\n')

  return code

def header(vsa, config):
  code = []
  my_indt = indt.Indentor(0)  

  var_prefix = config['PREFIX']

  # headers
  code.append('// vendor headers\n')
  code.append('#include "hls_stream.h"\n')
  code.append('#include "ap_int.h"\n')
  code.append('#include "ap_fixed.h"\n\n')
  code.append('// common headers\n')
  code.append('#include <stdio.h>\n')
  code.append('#include <string.h>\n\n')

  code.append('using namespace hls;\n\n')

  code.append('#define cal_aligned_size(x, y) ((x + y - 1) / y * y)\n')

  code.append('// Data types\n')
  idx = 0
  for data_type in vsa['DATA_TYPE']:
    code.append('typedef ' + data_type + ' U%s' % (vsa['KERNEL_ID']) + '_data_t' + str(idx) + ';\n')
    code.append('typedef ap_uint<' + str(vsa['BUS_WIDTH'][idx]) + '> U%s' %(vsa['KERNEL_ID']) + '_bus_t' + str(idx) + ';\n')
    code.append('#define U%s' %(vsa['KERNEL_ID']) + '_DATA' + str(idx) + '_WIDTH ' + str(vsa['DATA_WIDTH'][idx]) + '\n')
    code.append('#define U%s' %(vsa['KERNEL_ID']) + '_DATA' + str(idx) + '_PACK_FACTOR (' + str(vsa['BUS_WIDTH'][idx]) + '/U%s'%(vsa['KERNEL_ID']) + '_DATA' + str(idx) + '_WIDTH)\n')
    idx += 1
  code.append('typedef unsigned int uint;\n')
  code.append('union ufloat{\n')
  code.append(' ' * 2 + 'float f;\n')
  code.append(' ' * 2 + 'unsigned int u;\n')
  code.append('};\n\n')

  code.append('// Macros\n')
  for param in vsa['PARAMETERS']:
    code.append('#define U%s' %(vsa['KERNEL_ID']) + '_' + param + ' ' + str(vsa['PARAMETERS'][param]) + '\n')
  # idx = 0
  # for op_name in desp['OP_NAME']:
  #   code.append('#define U%s' %(desp['KERNEL_ID']) + '_DATA' + str(idx) + '_SIZE ' + str(desp['ARRAY_SIZE'][idx]) + '\n')
  #   code.append('#define U%s' %(desp['KERNEL_ID']) + '_DATA' + str(idx) + '_SIZE_ALIGNED (cal_aligned_size(' + str(desp['ARRAY_SIZE'][idx]) + ', U%s' %(desp['KERNEL_ID']) + '_DATA' + str(idx) + '_PACK_FACTOR))\n')
  #   idx += 1
  # for res_name in desp['RES_NAME']:
  #   code.append('#define U%s' %(desp['KERNEL_ID']) + '_DATA' + str(idx) + '_SIZE ' + str(desp['ARRAY_SIZE'][idx]) + '\n')
  #   code.append('#define U%s' %(desp['KERNEL_ID']) + '_DATA' + str(idx) + '_SIZE_ALIGNED (cal_aligned_size(' + str(desp['ARRAY_SIZE'][idx]) + ', U%s' %(desp['KERNEL_ID']) + '_DATA' + str(idx) + '_PACK_FACTOR))\n')
  #   idx += 1

  code.append('\n')

  code.append('#define U%s' % (vsa['KERNEL_ID']) + '_ROW_IL_FACTOR ' + str(vsa['ROW_IL_FACTOR']) + '\n')
  code.append('#define U%s' % (vsa['KERNEL_ID']) + '_COL_IL_FACTOR ' + str(vsa['COL_IL_FACTOR']) + '\n')
  code.append('#define U%s' % (vsa['KERNEL_ID']) + '_SA_ROWS ' + str(vsa['SA_ROWS']) + '\n')
  code.append('#define U%s' % (vsa['KERNEL_ID']) + '_SA_COLS ' + str(vsa['SA_COLS']) + '\n')
  # code.append('#define U%s' %(desp['KERNEL_ID']) + '_LOCAL_REG_NUM ' + str(desp['LOCAL_REG_NUM']) + '\n')
  # code.append('#define U%s' %(desp['KERNEL_ID']) + '_LOCAL_ACCUM_NUM ' + str(desp['LOCAL_ACCUM_NUM']) + '\n')

  code.append('#define U%s' % (vsa['KERNEL_ID']) + '_SIMD_FACTOR ' + str(vsa['SIMD_FACTOR']) + '\n')
  idx = 0
  for op_name in vsa['OP_NAME']:
    code.append('#define U%s' %(vsa['KERNEL_ID']) + '_DATA' + str(idx) + '_FC_SIMD_FACTOR ' + str(vsa['FC_SIMD_FACTOR'][idx]) + '\n')
    code.append('#define U%s' %(vsa['KERNEL_ID']) + '_DATA' + str(idx) + '_FC_GROUP_FACTOR ' + str(vsa['FC_GROUP_FACTOR'][idx]) + '\n')
    code.append('#define U%s' %(vsa['KERNEL_ID']) + '_DATA' + str(idx) + '_FC_SPLIT_FACTOR ' + str(vsa['FC_SPLIT_FACTOR'][idx]) + '\n')
    idx += 1
  for res_name in vsa['RES_NAME']:
    code.append('#define U%s' %(vsa['KERNEL_ID']) + '_DATA' + str(idx) + '_FC_SIMD_FACTOR ' + str(vsa['FC_SIMD_FACTOR'][idx]) + '\n')
    code.append('#define U%s' %(vsa['KERNEL_ID']) + '_DATA' + str(idx) + '_FC_GROUP_FACTOR ' + str(vsa['FC_GROUP_FACTOR'][idx]) + '\n')
    code.append('#define U%s' %(vsa['KERNEL_ID']) + '_DATA' + str(idx) + '_FC_SPLIT_FACTOR ' + str(vsa['FC_SPLIT_FACTOR'][idx]) + '\n')  
    idx += 1

  code.append('\n')
  # idx = 0
  # for op_name in desp['OP_NAME']:
  #   code.append('#define U%s' %(desp['KERNEL_ID']) + '_DATA' + str(idx) + '_BUF_SIZE ' + str(desp['DFC_BUF_SIZE'][idx]) + '\n')
  #   code.append('#define ' + var_prefix + 'DATA' + str(idx) + '_HEAD_BUF_SIZE ' + str(desp['DFC_HEAD_BUF_SIZE'][idx]) + '\n')
  #   idx += 1
  # for res_name in desp['RES_NAME']:
  #   code.append('#define U%s' %(desp['KERNEL_ID']) + '_DATA' + str(idx) + '_BUF_SIZE ' + str(desp['DFC_BUF_SIZE'][idx]) + '\n')
  #   code.append('#define ' + var_prefix + 'DATA' + str(idx) + '_HEAD_BUF_SIZE ' + str(desp['DFC_HEAD_BUF_SIZE'][idx]) + '\n')
  #   idx += 1

  # code.append('\n')
  code.append('// Functions and structs\n')
  idx = 0
  for op_name in vsa['OP_NAME']:
    code.append('struct U%s' %(vsa['KERNEL_ID']) + '_Data' + str(idx) + 'TransferChannelType{\n')
    code.append(' '*2 + config['PREFIX'] + 'Data' + str(idx) + 'TransferChannelType(){}\n')
    code.append(' '*2 + var_prefix + 'Data' + str(idx) + 'TransferChannelType(\n')
    code.append(' '*4 + 'ap_uint<' + var_prefix + 'DATA' + str(idx) + '_WIDTH*' + var_prefix + \
        'DATA' + str(idx) + '_FC_SIMD_FACTOR> data_t\n')
    #code.append(' '*4 + 'unsigned int feeder_id_t,\n')
    #code.append(' '*4 + 'bool new_pair_t,\n')
    #code.append(' '*4 + 'bool last_pair_t,\n')
    #code.append(' '*4 + 'unsigned int filter_s_t\n')
    code.append(' '*2 + '){\n')
    code.append(' '*4 + 'data = data_t;\n')
    #code.append(' '*4 + 'feeder_id = feeder_id_t;\n')
    #code.append(' '*4 + 'new_pair = new_pair_t;\n')
    #code.append(' '*4 + 'last_pair = last_pair_t;\n')
    #code.append(' '*4 + 'FILTER_S = filter_s_t;\n')
    code.append(' '*2 + '}\n')

    code.append(' '*2 + 'ap_uint<' + var_prefix + 'DATA' + str(idx) + '_WIDTH*' + var_prefix + 'DATA' + str(idx) \
        + '_FC_SIMD_FACTOR> data;\n')
    #code.append(' '*2 + 'unsigned int feeder_id;\n')
    #code.append(' '*2 + 'bool new_pair;\n')
    #code.append(' '*2 + 'bool last_pair;\n')
    #code.append(' '*2 + 'unsigned int FILTER_S;\n')
    code.append('};\n\n')

    idx += 1

  for res_name in vsa['RES_NAME']:
    code.append('struct U%s' %(vsa['KERNEL_ID']) + '_Data' + str(idx) + 'TransferChannelType{\n')
    code.append(' '*2 + var_prefix + 'Data' + str(idx) + 'TransferChannelType(){}\n')
    code.append(' '*2 + var_prefix + 'Data' + str(idx) + 'TransferChannelType(\n')
    code.append(' '*4 + 'ap_uint<' + var_prefix + 'DATA' + str(idx) + '_WIDTH*' + var_prefix + \
        'DATA' + str(idx) + '_FC_SIMD_FACTOR> data_t){\n')
    code.append(' '*4 + 'data = data_t;\n')
    code.append(' '*2 + '}\n')

    code.append(' '*2 + 'ap_uint<' + var_prefix + 'DATA' + str(idx) + '_WIDTH*' + var_prefix + 'DATA' + str(idx) \
        + '_FC_SIMD_FACTOR> data;\n')
    code.append('};\n\n')

    idx += 1

  idx = 0
  for op_name in vsa['OP_NAME']:
    code.append('struct U%s' %(vsa['KERNEL_ID']) + '_Data' + str(idx) + 'PEChannelType{\n')
    code.append(' '*2 + var_prefix + 'Data' + str(idx) + 'PEChannelType(){}\n')

    code.append(' '*2 + var_prefix + 'Data' + str(idx) + 'PEChannelType(\n')
    code.append(' '*4 + 'ap_uint<' + str(vsa['OP_PE_SIMD_WIDTH'][idx]) + '> data_t\n')
    code.append(' '*2 + '){\n')
    code.append(' '*4 + 'data = data_t;\n')
    code.append(' '*2 + '}\n')

    # code.append(' '*2 + var_prefix + 'Data' + str(idx) + 'PEChannelType(\n')
    # code.append(' '*4 + 'ap_uint<' + str(vsa['OP_PE_SIMD_WIDTH'][idx]) + '> data_t,\n')
    # code.append(' '*4 + 'bool new_pair_t,\n')
    # code.append(' '*4 + 'unsigned int filter_s_t\n')
    # code.append(' '*2 + '){\n')
    # code.append(' '*4 + 'data = data_t;\n')
    # code.append(' '*4 + 'new_pair = new_pair_t;\n')
    # code.append(' '*4 + 'FILTER_S = filter_s_t;\n')
    # code.append(' '*2 + '}\n')

    # code.append(' '*2 + var_prefix + 'Data' + str(idx) + 'PEChannelType(\n')
    # code.append(' '*4 + 'ap_uint<' + str(desp['OP_PE_SIMD_WIDTH'][idx]) + '> data_t,\n')
    # code.append(' '*4 + 'bool new_pair_t,\n')
    # code.append(' '*4 + 'bool last_pair_t,\n')
    # code.append(' '*4 + 'unsigned int filter_s_t\n')
    # code.append(' '*2 + '){\n')
    # code.append(' '*4 + 'data = data_t;\n')
    # code.append(' '*4 + 'new_pair = new_pair_t;\n')
    # code.append(' '*4 + 'last_pair = last_pair_t;\n')
    # code.append(' '*4 + 'FILTER_S = filter_s_t;\n')
    # code.append(' '*2 + '}\n')

    code.append(' '*2 + 'ap_uint<' + str(vsa['OP_PE_SIMD_WIDTH'][idx]) + '> data;\n')
    # code.append(' '*2 + 'bool new_pair;\n')
    # code.append(' '*2 + 'bool last_pair;\n')
    # code.append(' '*2 + 'unsigned int FILTER_S;\n')
    code.append('};\n\n')

    code.append('typedef ap_uint<' + str(vsa['OP_PE_SIMD_WIDTH'][idx]) + '> ' + var_prefix + 'Data' + str(idx) + 'SIMDType;\n\n')
    idx += 1

  for res_name in vsa['RES_NAME']:
    code.append('struct U%s' %(vsa['KERNEL_ID']) + '_Data' + str(idx) + 'PEChannelType{\n')
    code.append(' '*2 + var_prefix + 'Data' + str(idx) + 'PEChannelType(){}\n')
    code.append(' '*2 + var_prefix + 'Data' + str(idx) + 'PEChannelType(\n')
    code.append(' '*4 + var_prefix + 'data_t' + str(idx) + ' data_t){\n')
    code.append(' '*4 + 'data = data_t;\n')
    code.append(' '*2 + '}\n')

    code.append(' '*2 + var_prefix + 'data_t' + str(idx) + ' data;\n')
    code.append('};\n\n')

    idx += 1

  # function declarations
  idx = 0
  for op_name in vsa['OP_NAME']:
    code.append('void ' + var_prefix + 'DataFeed' + str(idx) + 'Head(\n')
    code.append(' '*2 + var_prefix + 'data_t' + str(idx) + '* ' + vsa['OP_NAME'][idx] + ',\n')
    # code.append(' '*2 + 'bool init,\n')
    # code.append(' '*2 + 'unsigned int FILTER_S,\n')
    # for feed_id in range(desp['FC_SPLIT_FACTOR'][idx]):
    #   if feed_id < desp['FC_SPLIT_FACTOR'][idx] - 1:
    #     code.append(' '*2 + 'stream<' + var_prefix + 'Data' + str(idx) + 'TransferChannelType> &fifo_transfer_out' + str(feed_id) + ',\n')
    #   else:
    #     code.append(' '*2 + 'stream<' + var_prefix + 'Data' + str(idx) + 'TransferChannelType> &fifo_transfer_out' + str(feed_id) + '\n')
    code.append(' '*2 + 'stream<%sData%dTransferChannelType> &fifo_transfer_out\n' 
        % (var_prefix, idx))
    code.append(');\n\n')
    idx += 1

  for res_name in vsa['RES_NAME']:
    code.append('void ' + var_prefix + 'DataCollect' + str(idx) + 'Head(\n')
    code.append(' '*2 + var_prefix + 'data_t' + str(idx) + '* ' + vsa['RES_NAME'][idx - len(vsa['OP_NAME'])] + ',\n')
    # for feed_id in range(desp['FC_SPLIT_FACTOR'][idx]):
    #   if feed_id < desp['FC_SPLIT_FACTOR'][idx] - 1:
    #     code.append(' '*2 + 'stream<' + var_prefix + 'Data' + str(idx) + 'TransferChannelType> &fifo_transfer_in' + str(feed_id) + ',\n')
    #   else:
    #     code.append(' '*2 + 'stream<' + var_prefix + 'Data' + str(idx) + 'TransferChannelType> &fifo_transfer_in' + str(feed_id) + '\n')
    code.append(' '*2 + 'stream<%sData%dTransferChannelType> &fifo_transfer_in\n' 
        % (var_prefix, idx))
    code.append(');\n\n')
    idx += 1

  idx = 0
  for op_name in vsa['OP_NAME']:
    code.append('void ' + var_prefix + 'DataFeed' + str(idx) + 'Engine0_wrapper(\n')
    code.append(' '*2 + 'stream<' + var_prefix + 'Data' + str(idx) + 'TransferChannelType> &fifo_transfer_in,\n')
    code.append(' '*2 + 'stream<' + var_prefix + 'Data' + str(idx) + 'TransferChannelType> &fifo_transfer_out,\n')
    # for gs in range(desp['FC_GROUP_FACTOR'][idx]):
    #   code.append(' '*2 + 'stream<' + var_prefix + 'Data' + str(idx) + 'PEChannelType> &fifo_feed_' + str(gs) + ',\n')
    code.append(' '*2 + 'stream<%sData%dPEChannelType> &fifo_feed,\n' % (var_prefix, idx))
    code.append(' '*2 + 'unsigned int engine_id\n')
    code.append(');\n\n')

    code.append('void ' + var_prefix + 'DataFeed' + str(idx) + 'EngineLast(\n')
    code.append(' '*2 + 'stream<' + var_prefix + 'Data' + str(idx) + 'TransferChannelType> &fifo_transfer_in,\n')
    # for gs in range(desp['FC_GROUP_FACTOR'][idx]):
    #   code.append(' '*2 + 'stream<' + var_prefix + 'Data' + str(idx) + 'PEChannelType> &fifo_feed_' + str(gs) + ',\n')
    code.append(' '*2 + 'stream<%sData%dPEChannelType> &fifo_feed,\n' % (var_prefix, idx))
    code.append(' '*2 + 'unsigned int engine_id\n')
    code.append(');\n\n')

    idx += 1

  for res_name in vsa['RES_NAME']:
    code.append('void ' + var_prefix + 'DataCollect' + str(idx) + 'Engine0_wrapper(\n')
    code.append(' '*2 + 'stream<' + var_prefix + 'Data' + str(idx) + 'TransferChannelType> &fifo_transfer_in,\n')
    code.append(' '*2 + 'stream<' + var_prefix + 'Data' + str(idx) + 'TransferChannelType> &fifo_transfer_out,\n')
    # for gs in range(desp['FC_GROUP_FACTOR'][idx]):
    #   code.append(' '*2 + 'stream<' + var_prefix + 'Data' + str(idx) + 'PEChannelType> &fifo_collect_' + str(gs) + ',\n')
    code.append(' '*2 + 'stream<%sData%dPEChannelType> &fifo_collect,\n' % (var_prefix, idx))
    code.append(' '*2 + 'unsigned int engine_id\n')
    code.append(');\n\n')

    code.append('void ' + var_prefix + 'DataCollect' + str(idx) + 'EngineLast(\n')
    code.append(' '*2 + 'stream<' + var_prefix + 'Data' + str(idx) + 'TransferChannelType> &fifo_transfer_out,\n')
    # for gs in range(desp['FC_GROUP_FACTOR'][idx]):
    #   code.append(' '*2 + 'stream<' + var_prefix + 'Data' + str(idx) + 'PEChannelType> &fifo_collect_' + str(gs) + ',\n')
    code.append(' '*2 + 'stream<%sData%dPEChannelType> &fifo_collect,\n' % (var_prefix, idx))
    code.append(' '*2 + 'unsigned int engine_id\n')
    code.append(');\n\n')

    idx += 1

  idx = 0
  code.append('void ' + var_prefix + 'top_kernel(\n')
  for name in vsa['OP_NAME']:
    code.append(' '*2 + var_prefix + 'data_t' + str(idx) + '* ' + name + ',\n')
    idx += 1
  for name in vsa['RES_NAME']:
    code.append(' '*2 + var_prefix + 'data_t' + str(idx) + '* ' + name + '\n')
    idx += 1
  # code.append(' '*2 + 'bool init,\n')
  # code.append(' '*2 + 'unsigned int FILTER_S\n')
  code.append(');\n\n')

  code.append('template<typename To, typename From>\n')
  code.append('inline To Reinterpret(const From& val){\n')
  code.append(' '*2 + 'return reinterpret_cast<const To&>(val);\n')
  code.append('}\n\n')

  code.append('template<class data_t, class bus_t, int WIDTH>\n')
  code.append('data_t data_select(\n')
  code.append(' '*2 + 'bus_t bus_data,\n')
  code.append(' '*2 + 'uint offset\n')
  code.append('){\n')
  code.append(' '*2 + 'data_t ret;\n')
  code.append(' '*2 + 'ret = Reinterpret<data_t>((ap_uint<WIDTH>)bus_data(WIDTH-1 + offset*WIDTH, offset*WIDTH));\n')
  code.append(' '*2 + 'return ret;\n')
  code.append('}\n')

  return code

def df_loader(vsa, config):
  code = []
  my_indt = indt.Indentor(0)  

  idx = 0
  for op_name in vsa['OP_NAME']:
    # 

    print_line(code, my_indt, 'void %sDataFeed%dHead(\n' % (config['PREFIX'], idx))
    my_indt.inc()
    print_line(code, my_indt, '%sdata_t%d *%s,\n' % (config['PREFIX'], idx, op_name))
    print_line(code, my_indt, 'stream<%sData%dTransferChannelType> &fifo_transfer_out\n' 
      % (config['PREFIX'], idx))   
    my_indt.dec()
    print_line(code, my_indt, '){\n')
    print_line(code, my_indt, '#pragma HLS INLINE off\n')
    print_line(code, my_indt, '#pragma HLS DATA_PACK variable=fifo_transfer_out\n')

    

    print_line(code, my_indt, '}\n\n')

    idx += 1

  return code