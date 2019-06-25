# Functions:
# gen_new_arr_ref
# code_parse
# df_loader_code_modify
# extract_df_loader_misc_from_code

# dc_loader_code_modify
# extract_dc_loader_misc_from_code

# df_engine_feed_code_modify
# df_engine_read_code_modify
# df_engine_read_last_code_modify
# extract_df_engine_misc_from_code

# dc_engine_collect_code_modify
# dc_engine_write_code_modify
# dc_engine_write_last_code_modify
# extract_dc_engine_misc_from_code

# op_transfer_code_modify
# op_transfer_last_code_modify
# extract_op_transfer_misc_from_code

# res_transfer_code_modify
# res_transfer_first_code_modify
# extract_res_transfer_misc_from_code


# extract_compute_misc_from_code

def df_loader_code_modify(cloog_code, stmt_macros, acc_id, acc_name, vsa, config):
  gen_code = []

  arr_lb = []

  stmts = {}
  #print(len(stmt_macros))
  for stmt_macro in stmt_macros:
    #print(stmt_macro)
    #print(stmt_macro.strip('\n').split(' '))
    stmt_info = {}
    stmt_decl = stmt_macro.strip('\n').split(' ')[1]
    stmt_text = stmt_macro.strip('\n').split(' ')[2]

    # get stmt_name
    stmt_name = stmt_decl[:stmt_decl.find('(')]
    #print(stmt_name)
    # get stmt_iters
    stmt_iters = stmt_decl[stmt_decl.find('(') + 1 : stmt_decl.find(')')].split(',')

    stmts[stmt_name] = {}
    stmts[stmt_name]['ITERS'] = stmt_iters
    stmts[stmt_name]['TEXT'] = stmt_text

  # build the map from stmt_iters to loop_iters
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      iter_map = {}
      loop_iters = code_line[code_line.find('(') + 1:code_line.find(')')].split(',')
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      for stmt_iter_id in range(len(stmts[stmt_name]['ITERS'])):
        stmt_iter = stmts[stmt_name]['ITERS'][stmt_iter_id]
        iter_map[stmt_iter] = "(" + loop_iters[stmt_iter_id] + ")"
      stmts[stmt_name]['ITER_MAP'] = iter_map

  # modify the statements in the cloog code
  loop_lbs = []
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('for') >= 0:
      lb = code_line[code_line.find('=') + 1: code_line.find(';')]
      loop_lbs.append(lb)

    if code_line.find('S') >= 0:
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      if stmt_name == 'S1':
        # load from DRAM

        # get the array lower bounds
        arr_lb = loop_lbs[vsa['ARRAY_PART_BAND_WIDTH']:]
        #print(arr_lb)

        # get the array reference
        stmt_text = stmts[stmt_name]['TEXT']
        array_ref = []
        stmt_text = stmt_text[stmt_text.find('['):stmt_text.find('=')].strip(' ')
        stmt_text = stmt_text.split('[')[1:]
        for ref in stmt_text:
          array_ref.append(ref.strip(']'))
        # generate the new array reference with the loop iterators
        new_array_ref = []
        #print(array_ref)
        for ref in array_ref:
          new_ref = ""
          ext_ch_id = -1
          for ch_id in range(len(ref)):
            if ch_id <= ext_ch_id:
              continue
            #print(ch_id)
            if ref[ch_id] == 'd':
              tmp_iter = 'd'
              for ext_ch_id in range(ch_id + 1, len(ref)):
                if ref[ext_ch_id].isdigit() == True:
                  tmp_iter += ref[ext_ch_id]
                else:
                  ext_ch_id = ext_ch_id - 1
                  break

              #print(ext_ch_id)
              loop_iter = stmts[stmt_name]['ITER_MAP'][tmp_iter]
              new_ref += loop_iter
            else:
              new_ref += ref[ch_id]
          #print(new_ref)
          new_array_ref.append(new_ref)
        new_code_line = code_line[:code_line.find('S')]
        new_code_line += "%s_buf" % (acc_name)
        idx = 0
        for new_ref in new_array_ref:
          new_code_line += '['
          new_code_line += new_ref
          new_code_line += ' - %s' % (arr_lb[idx])
          new_code_line += ']'
          idx += 1
        new_code_line += ' = %s' % (acc_name)
        for new_ref in new_array_ref:
          new_code_line += '['
          new_code_line += new_ref
          new_code_line += ']'
        new_code_line += ';\n'
        gen_code.append(new_code_line)
      else:
        # write to FIFO

        # get the array reference
        stmt_text = stmts[stmt_name]['TEXT']
        array_ref = []
        stmt_text = stmt_text[stmt_text.find('['):stmt_text.find(')')]
        stmt_text = stmt_text.split('[')[1:]
        for ref in stmt_text:
          array_ref.append(ref.strip(']'))
        # generate the new array reference with the loop iterators
        new_array_ref = []
        for ref in array_ref:
          new_ref = ""
          ext_ch_id = -1
          for ch_id in range(len(ref)):
            if ch_id <= ext_ch_id:
              continue
            if ref[ch_id] == 'd':
              tmp_iter = 'd'
              for ext_ch_id in range(ch_id + 1, len(ref)):
                if ref[ext_ch_id].isdigit() == True:
                  tmp_iter += ref[ext_ch_id]
                else:
                  ext_ch_id = ext_ch_id - 1
                  break

              loop_iter = stmts[stmt_name]['ITER_MAP'][tmp_iter]
              new_ref += loop_iter

#              ch_id = ext_ch_id
            else:
              new_ref += ref[ch_id]
          new_array_ref.append(new_ref)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += '%sData%dTransferChannelType fifo_data;\n' \
          % (config['PREFIX'], acc_id)
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'fifo_data.data = %s_buf' % (acc_name)

        # new_code_line += "fifo_%s.write(%s_buf" % (acc_name, acc_name)
        idx = 0
        for new_ref in new_array_ref:
          new_code_line += '['
          new_code_line += new_ref
          new_code_line += ' - %s' %(arr_lb[idx])
          new_code_line += ']'
          idx += 1
        new_code_line += ';\n'
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'fifo_transfer_out.write(fifo_data);\n'
        gen_code.append(new_code_line)
    else:
      gen_code.append(code_line)

  return gen_code

def extract_df_loader_misc_from_code(vsa, config, acc_id, acc_name):
  misc_info = {}

  file_name = config['CODE_DIR'] + '/' + acc_name + "_df_loader_code.c"
  content = []
  with open(file_name, 'r') as f:
    for line in f.readlines():
      content.append(line)

  array_decls = []
  stmt_macros = []
  iter_decls = []
  cloog_code = []

  first_define = 0
  for line_id in range(len(content)):
    line = content[line_id]
    if line.find('#define') >= 0:
      if first_define == 0:
        array_decls_end_line_id = line_id
        first_define = 1
        stmt_macros_start_line_id = line_id
      stmt_macros_end_line_id = line_id + 1
    if line.find('int') >= 0:
      iter_decls_line_id = line_id
    if line.find('/* Start of CLooG code */') >= 0:
      cloog_code_start_line_id = line_id + 1
    if line.find('/* End of CLooG code */') >= 0:
      cloog_code_end_line_id = line_id

  # parse array declarations
  for line_id in range(0, array_decls_end_line_id):
    array_decls.append(content[line_id])

  misc_info['ARRAY_DECLS'] = {}
  for array_decl in array_decls:
    array_decl = array_decl.strip('\n').strip(';').split(' ')[1]
    # get arary_name
    array_name = array_decl[:array_decl.find('[')]
    array_name = array_name.split('_')[-1]
    # get array_extent
    array_extent = array_decl[array_decl.find('['):]

    misc_info['ARRAY_DECLS'][array_name] = array_extent
    # print(array_name, array_extent)

  # parse statement macros
  for line_id in range(stmt_macros_start_line_id, stmt_macros_end_line_id):
    stmt_macros.append(content[line_id])

  # parse iterator declarations
  iter_decls.append(content[iter_decls_line_id])
  misc_info['ITER_DECLS'] = iter_decls

  # parse generated code
  for line_id in range(cloog_code_start_line_id, cloog_code_end_line_id):
    cloog_code.append(content[line_id])

  # modify the cloog generated code:
  # - replace the statement macros
  # - add array extent shifting code
  gen_code = df_loader_code_modify(cloog_code, stmt_macros, acc_id, acc_name, vsa, config)

  misc_info['GEN_CODE'] = gen_code

  return misc_info

def gen_new_arr_ref(refs, iter_map):
  new_refs = []
  for ref in refs:
    # print(ref)
    new_ref = ''
    ext_ch_id = -1
    for ch_id in range(len(ref)):
      if ch_id <= ext_ch_id:
        continue
      if ref[ch_id] == 'd':
        tmp_iter = 'd'
        for ext_ch_id in range(ch_id + 1, len(ref)):
          if ref[ext_ch_id].isdigit() == True:
            tmp_iter += ref[ext_ch_id]
          else:
            ext_ch_id = ext_ch_id - 1
            break
        loop_iter = iter_map[tmp_iter]
        new_ref += loop_iter
      else:
        new_ref += ref[ch_id]
    new_refs.append(new_ref)

  return new_refs

def dc_loader_code_modify(cloog_code, stmt_macros, acc_id, acc_name, acc_nrows, vsa, config):
  gen_code = []

  stmts = {}
  for stmt_macro in stmt_macros:
    stmt_info = {}
    stmt_decl = stmt_macro.strip('\n').split(' ')[1]
    stmt_text = stmt_macro.strip('\n').split(' ')[2]

    # get stmt_name
    stmt_name = stmt_decl[:stmt_decl.find('(')]
    # get stmt_iter
    stmt_iters = stmt_decl[stmt_decl.find('(') + 1:stmt_decl.find(')')].split(',')

    stmts[stmt_name] = {}
    stmts[stmt_name]['ITERS'] = stmt_iters
    stmts[stmt_name]['TEXT'] = stmt_text

  # build the map from stmt_iters to loop_iters
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      iter_map = {}
      loop_iters = code_line[code_line.find('(') + 1: code_line.find(')')].split(',')
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      for stmt_iter_id in range(len(stmts[stmt_name]['ITERS'])):
        stmt_iter = stmts[stmt_name]['ITERS'][stmt_iter_id]
        iter_map[stmt_iter] = '(' + loop_iters[stmt_iter_id] + ')'
      stmts[stmt_name]['ITER_MAP'] = iter_map

  # modify the statements in the cloog code
  arr_lb = []
  loop_lbs = []
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('for') >= 0:
      lb = code_line[code_line.find('=') + 1: code_line.find(';')]
      loop_lbs.append(lb)

    if code_line.find('S1') >= 0:
      # get the array lower bounds
      start_idx = -acc_nrows
      #print(start_idx)
      arr_lb = loop_lbs[start_idx:]

  #print(acc_nrows)
  #print(loop_lbs)
  #print(arr_lb)

  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      if stmt_name == 'S1':
        # write to DRAM

        # get the array reference
        stmt_text = stmts[stmt_name]['TEXT']
        array_ref = []
        stmt_text = stmt_text[stmt_text.find('['):stmt_text.find('=')].strip(' ')
        stmt_text = stmt_text.split('[')[1:]
        for ref in stmt_text:
          array_ref.append(ref.strip(']'))

        # generate the new array reference with the loop iterators
        new_array_ref = []
        for ref in array_ref:
          new_ref = ""
          ext_ch_id = -1
          for ch_id in range(len(ref)):
            if ch_id <= ext_ch_id:
              continue
            if ref[ch_id] == 'd':
              tmp_iter = 'd'
              for ext_ch_id in range(ch_id+1, len(ref)):
                if ref[ext_ch_id].isdigit() == True:
                  tmp_iter += ref[ext_ch_id]
                else:
                  ext_ch_id = ext_ch_id - 1
                  break
              loop_iter = stmts[stmt_name]['ITER_MAP'][tmp_iter]
              new_ref += loop_iter
            else:
              new_ref += ref[ch_id]
          new_array_ref.append(new_ref)
        new_code_line = code_line[:code_line.find('S')]
        new_code_line += '%s' % (acc_name)
        idx = 0
        for new_ref in new_array_ref:
          new_code_line += '['
          new_code_line += new_ref
          new_code_line += ']'
        new_code_line += ' = %s_buf' % (acc_name)
        for new_ref in new_array_ref:
          new_code_line += '['
          new_code_line += new_ref
          new_code_line += ' - %s' % (arr_lb[idx])
          new_code_line += ']'
          idx += 1
        new_code_line += ';\n'
        gen_code.append(new_code_line)
      else:
        # read from FIFO

        # get the array references
        stmt_text = stmts[stmt_name]['TEXT']
        array_ref = []
        stmt_text = stmt_text[stmt_text.find('['):stmt_text.find('=')]
        stmt_text = stmt_text.split('[')[1:]
        for ref in stmt_text:
          array_ref.append(ref.strip(']'))
        # generate the new array reference with the loop iterators
        new_array_ref = gen_new_arr_ref(array_ref, stmts[stmt_name]['ITER_MAP'])

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += '%sData%dTransferChannelType fifo_data = fifo_transfer_in.read();\n' \
          % (config['PREFIX'], acc_id)
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += '%s_buf' % (acc_name)
        idx = 0
        for new_ref in new_array_ref:
          new_code_line += '['
          new_code_line += new_ref
          new_code_line += ' - %s' % (arr_lb[idx])
          new_code_line += ']'
          idx += 1
        new_code_line += ' = fifo_data.data;\n'
        gen_code.append(new_code_line)
    else:
      gen_code.append(code_line)

  return gen_code

def extract_dc_loader_misc_from_code(vsa, config, acc_id, acc_name, acc_nrows):
  misc_info = {}

  file_name = config['CODE_DIR'] + '/' + acc_name + '_dc_loader_code.c'
  content = []
  with open(file_name, 'r') as f:
    for line in f.readlines():
      content.append(line)

  array_decls = []
  stmt_macros = []
  iter_decls = []
  cloog_code = []

  first_define = 0
  for line_id in range(len(content)):
    line = content[line_id]
    if line.find('#define') >= 0:
      if first_define == 0:
        array_decls_end_line_id = line_id
        first_define = 1
        stmt_macros_start_line_id = line_id
      stmt_macros_end_line_id = line_id + 1
    if line.find('int') >= 0:
      iter_decls_line_id = line_id
    if line.find('/* Start of CLooG code */') >= 0:
      cloog_code_start_line_id = line_id + 1
    if line.find('/* End of CLooG code */') >= 0:
      cloog_code_end_line_id = line_id

  # parse array declerations
  for line_id in range(0, array_decls_end_line_id):
    array_decls.append(content[line_id])

  misc_info['ARRAY_DECLS'] = {}
  for array_decl in array_decls:
    array_decl = array_decl.strip('\n').strip(';').split(' ')[1]
    # get array_name
    array_name = array_decl[:array_decl.find('[')]
    array_name = array_name.split('_')[-1]
    # get array_extent
    array_extent = array_decl[array_decl.find('['):]

    misc_info['ARRAY_DECLS'][array_name] = array_extent

  # parse statement macros
  for line_id in range(stmt_macros_start_line_id, stmt_macros_end_line_id):
    stmt_macros.append(content[line_id])

  # parse iterator declerations
  iter_decls.append(content[iter_decls_line_id])
  misc_info['ITER_DECLS'] = iter_decls

  # parse generate_code
  for line_id in range(cloog_code_start_line_id, cloog_code_end_line_id):
    cloog_code.append(content[line_id])

  # modify the cloog generated code
  # - replace the statement macros
  # - add array extent shifting code
  gen_code = dc_loader_code_modify(cloog_code, stmt_macros, acc_id, acc_name, acc_nrows, vsa, config)

  misc_info['GEN_CODE'] = gen_code
  #misc_info['GEN_CODE'] = cloog_code

  return misc_info

def code_parse(content):
  array_decls = []
  stmt_macros = []
  iter_decls = []
  cloog_code = []

  first_define = 0
  for line_id in range(len(content)):
    line = content[line_id]
    if line.find('#define') >= 0:
      if first_define == 0:
        array_decls_end_line_id = line_id
        first_define = 1
        stmt_macros_start_line_id = line_id
      stmt_macros_end_line_id = line_id + 1
    if line.find('int') >= 0:
      iter_decls_line_id = line_id
    if line.find('/* Start of CLooG code */') >= 0:
      cloog_code_start_line_id = line_id + 1
    if line.find('/* End of CLooG code */') >= 0:
      cloog_code_end_line_id = line_id

  # prase array declerations
  for line_id in range(0, array_decls_end_line_id):
    array_decls.append(content[line_id])

  # parse stmt macros
  for line_id in range(stmt_macros_start_line_id, stmt_macros_end_line_id):
    stmt_macros.append(content[line_id])

  # parse iter declarations
  # iter_decls.append(content[iter_decls_line_id])
  iter_decls = content[iter_decls_line_id]

  # parse cloog code
  for line_id in range(cloog_code_start_line_id, cloog_code_end_line_id):
    cloog_code.append(content[line_id])

  return [array_decls, stmt_macros, iter_decls, cloog_code]

def extract_df_engine_misc_from_code(vsa, config, acc_id, acc_name, acc_nrows):
  misc_info = {}

  file_name = config['CODE_DIR'] + '/' + acc_name + '_df_engine_read_code.c'
  content = []
  with open(file_name, 'r') as f:
    for line in f.readlines():
      content.append(line)

  array_decls = []
  stmt_macros = []
  iter_decls = []
  cloog_code = []

  [array_decls, stmt_macros, iter_decls, cloog_code] = code_parse(content)

  # ARRAY_DECLS
  # e.g., "A": [32][32]
  misc_info['ARRAY_DECLS'] = {}
  for array_decl in array_decls:
    array_decl = array_decl.strip('\n').strip(';').split(' ')[1]
    # get array_name
    array_name = array_decl[:array_decl.find('[')]
    array_name = array_name.split('_')[-1]
    # get array_extent
    array_extent = array_decl[array_decl.find('['):]

    misc_info['ARRAY_DECLS'][array_name] = array_extent

  # ARRAY_PART_LOOPS
  # e.g.,
  # for (t1=0;t1<...) {
  #   for (t2=0;t2<...) {

  # LAST_ARRAY_PART_TILE_COND
  # e.g., t1 == 31 && t2 == 31 && t3 == 31

  # FIRST_ARRAY_PART_TILE_COND
  # e.g., t1 == 0 && t2 == 0 && t3 == 0

  # ARRAY_PART_ITER_DECLS
  # e.g., int t1, int t2, ...
  array_part_iters = []
  engine_iters = []
  # other_iters = []
  array_part_iter_ubs = []
  array_part_iter_lbs = []
  array_part_band_width = vsa['ARRAY_PART_BAND_WIDTH']
  engine_band_width = vsa['ENGINE_BAND_WIDTH'][acc_id]

  array_part_loops = []

  loop_cnt = 0
  for code_line in cloog_code:
    if loop_cnt < array_part_band_width and code_line.find('for') >= 0:
      array_part_loops.append(code_line)
      for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip(' ')
      iter_name = for_cond[:for_cond.find('=')]
      array_part_iters.append(iter_name)
      tmp_cond = for_cond[for_cond.find('<=')+2:]
      ub = tmp_cond[:tmp_cond.find(';')]
      array_part_iter_ubs.append(ub)

      lb = for_cond[for_cond.find('=')+1:for_cond.find(';')]
      array_part_iter_lbs.append(lb)
      loop_cnt += 1
    elif loop_cnt >= array_part_band_width and loop_cnt < array_part_band_width + \
      engine_band_width:
      for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip(' ')
      iter_name = for_cond[:for_cond.find('=')]
      engine_iters.append(iter_name)
      loop_cnt += 1
    # else:
    #   if code_line.find('for') >= 0:
    #     for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip(' ')
    #     iter_name = for_cond[:for_cond.find('=')]
    #     other_iters.append(iter_name)
    #     loop_cnt += 1

  # get the lower bounds for local buffer
  arr_lbs = []
  loop_cnt = 0
  for code_line in cloog_code:
    if code_line.find('for') >= 0:
      if loop_cnt >= array_part_band_width + engine_band_width:
        for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip(' ')
        lb = for_cond[for_cond.find('=')+1:for_cond.find(';')]
        arr_lbs.append(lb)
    loop_cnt += 1

  misc_info['ARR_LBS'] = arr_lbs
  misc_info['ARRAY_PART_LOOPS'] = array_part_loops
  misc_info['ARRAY_PART_ITER_UBS'] = array_part_iter_ubs
  misc_info['ARRAY_PART_ITER_LBS'] = array_part_iter_lbs

  last_array_part_tile_cond = ""
  for iter_idx in range(len(array_part_iters)):
    iter_name = array_part_iters[iter_idx]
    iter_ub = array_part_iter_ubs[iter_idx]
    last_array_part_tile_cond += '%s == %s' % (iter_name, iter_ub)
    if iter_idx < len(array_part_iters) - 1:
      last_array_part_tile_cond += ' && '
  misc_info['LAST_ARRAY_PART_TILE_COND'] = last_array_part_tile_cond

  first_array_part_tile_cond = ""
  for iter_idx in range(len(array_part_iters)):
    iter_name = array_part_iters[iter_idx]
    iter_lb = array_part_iter_lbs[iter_idx]
    first_array_part_tile_cond += '(%s == %s)' % (iter_name, iter_lb)
    if iter_idx < len(array_part_iters) - 1:
      first_array_part_tile_cond += ' && '
  misc_info['FIRST_ARRAY_PART_TILE_COND'] = first_array_part_tile_cond

  array_part_iter_decls = []
  engine_iter_decls = []
  for iter_name in array_part_iters:
    array_part_iter_decls.append('int ' + iter_name)
  for iter_name in engine_iters:
    engine_iter_decls.append('int ' + iter_name)

  misc_info['ARRAY_PART_ITERS'] = array_part_iters
  misc_info['ENGINE_ITERS'] = engine_iters
  misc_info['ARRAY_PART_ITER_DECLS'] = array_part_iter_decls
  misc_info['ENGINE_ITER_DECLS'] = engine_iter_decls

  # ENGINE_LOOP_PREFIX
  loop_cnt = 0
  for code_line in cloog_code:
    if code_line.find('for') >= 0:
      if loop_cnt >= array_part_band_width and loop_cnt < array_part_band_width \
        + engine_band_width:
        for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip(' ')
        lb = for_cond[for_cond.find('=')+1:for_cond.find(';')]
        misc_info['ENGINE_LOOP_PREFIX'] = lb
        break
      loop_cnt += 1

  # ENGINE_FEED_CODE
  file_name = config['CODE_DIR'] + '/' + acc_name + '_df_engine_feed_code.c'
  content = []
  with open(file_name, 'r') as f:
    for line in f.readlines():
      content.append(line)

  array_decls = []
  stmt_macros = []
  iter_decls = []
  cloog_code = []

  [array_decls, stmt_macros, iter_decls, cloog_code] = code_parse(content)
  gen_code = df_engine_feed_code_modify(cloog_code, stmt_macros, iter_decls, \
    acc_id, acc_name, acc_nrows, vsa, config, misc_info)
  misc_info['ENGINE_FEED_CODE'] = gen_code

  # ENGINE_READ_CODE
  file_name = config['CODE_DIR'] + '/' + acc_name + '_df_engine_read_code.c'
  content = []
  with open(file_name, 'r') as f:
    for line in f.readlines():
      content.append(line)

  array_decls = []
  stmt_macros = []
  iter_decls = []
  cloog_code = []

  [array_decls, stmt_macros, iter_decls, cloog_code] = code_parse(content)
  gen_code = df_engine_read_code_modify(cloog_code, stmt_macros, iter_decls, \
    acc_id, acc_name, acc_nrows, vsa, config, misc_info)
  misc_info['ENGINE_READ_CODE'] = gen_code

  # ENGINE_READ_LAST_CODE
  file_name = config['CODE_DIR'] + '/' + acc_name + '_df_engine_read_code.c'
  content = []
  with open(file_name, 'r') as f:
    for line in f.readlines():
      content.append(line)

  array_decls = []
  stmt_macros = []
  iter_decls = []
  cloog_code = []

  [array_decls, stmt_macros, iter_decls, cloog_code] = code_parse(content)
  gen_code = df_engine_read_last_code_modify(cloog_code, stmt_macros, iter_decls, \
    acc_id, acc_name, acc_nrows, vsa, config, misc_info)
  misc_info['ENGINE_READ_LAST_CODE'] = gen_code

  return misc_info

def df_engine_feed_code_modify(cloog_code, stmt_macros, iter_decls, \
  acc_id, acc_name, acc_nrows, vsa, config, misc_info):
  gen_code = []

  # parse stmts
  stmts = {}
  for stmt_macro in stmt_macros:
    stmt_info = {}
    stmt_decl = stmt_macro.strip('\n').split(' ')[1]
    stmt_text = stmt_macro.strip('\n').split(' ')[2]

    # get stmt_name
    stmt_name = stmt_decl[:stmt_decl.find('(')]
    # get stmt_iters
    stmt_iters = stmt_decl[stmt_decl.find('(')+1:stmt_decl.find(')')].split(',')

    stmts[stmt_name] = {}
    stmts[stmt_name]['ITERS'] = stmt_iters
    stmts[stmt_name]['TEXT'] = stmt_text

  # build the map from stmt_iters to loop_iters
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      iter_map = {}
      loop_iters = code_line[code_line.find('(')+1:code_line.find(')')].split(',')
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      for stmt_iter_id in range(len(stmts[stmt_name]['ITERS'])):
        stmt_iter = stmts[stmt_name]['ITERS'][stmt_iter_id]
        iter_map[stmt_iter] = '(' + loop_iters[stmt_iter_id] + ')'
      stmts[stmt_name]['ITER_MAP'] = iter_map

  # modify the statements in the cloog code
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      if stmt_name == 'S1':
        # feed to PE
        # add the pipeline pragma
        new_code_line = code_line[:code_line.find('S') - 2]
        new_code_line += '#pragma HLS PIPELINE II=1\n'
        gen_code.append(new_code_line)

        # get the array reference
        stmt_text = stmts[stmt_name]['TEXT']
        array_ref = []
        stmt_text = stmt_text[stmt_text.find('['):stmt_text.find(')')].strip()
        stmt_text = stmt_text.split('[')[1:]
        for ref in stmt_text:
          array_ref.append(ref.strip(']'))
        new_array_ref = gen_new_arr_ref(array_ref, stmts[stmt_name]['ITER_MAP'])

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += '%sData%dTransferChannelType fifo_data;\n' \
          % (config['PREFIX'], acc_id)
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'fifo_data = '
        new_code_line += 'buffer'
        idx = 0
        for new_ref in new_array_ref:
          new_code_line += '['
          new_code_line += new_ref
          new_code_line += ' - %s' % (misc_info['ARR_LBS'][idx])
          new_code_line += ']'
          idx += 1
        new_code_line += ';\n'
        
        # print(new_code_line)

        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'fifo_feed.write(fifo_data);\n'
        gen_code.append(new_code_line)
    else:
      gen_code.append(code_line)

  # parse the gen_code the second time to add teh iterator decls, engine guards
  # and other stmts
  new_gen_code = []
  # print iter decls
  new_gen_code.append('// iterator declarations\n')
  iters = iter_decls[iter_decls.find('int')+3:iter_decls.find(';')].strip().split(',')
  array_part_iters = misc_info['ARRAY_PART_ITERS']
  engine_iters = misc_info['ENGINE_ITERS']
  for loop_iter in iters:
    loop_iter = loop_iter.strip() # strip the possible space before the iterator
    if loop_iter in array_part_iters:
      continue
    else:
      new_gen_code.append('int %s;\n' % (loop_iter))
  new_gen_code.append('\n')

  loop_cnt = 0
  array_part_band_width = vsa['ARRAY_PART_BAND_WIDTH']
  engine_band_width = vsa['ENGINE_BAND_WIDTH'][acc_id]

  engine_code = gen_code[array_part_band_width:-array_part_band_width]
  engine_loop_skip_idx = engine_code[0].find('for')
  # eliminate all the extra space in each line
  for idx in range(len(engine_code)):
    engine_code[idx] = engine_code[idx][engine_loop_skip_idx:]

  if engine_band_width > 0:
    new_gen_code.append(engine_code[0])
    # add the guards
    new_code_line = '  if (%s == %s + engine_id) {\n' % (misc_info['ENGINE_ITERS'][0], misc_info['ENGINE_LOOP_PREFIX'])
    new_gen_code.append(new_code_line)

    for idx in range(1,len(engine_code)-1):
      new_code_line = ' ' * 2 + engine_code[idx]
      new_gen_code.append(new_code_line)

    new_gen_code.append(' ' * 2 + '}\n')
    new_gen_code.append('}\n')
  else:
    new_gen_code.append(engine_code)

  return new_gen_code

def df_engine_read_code_modify(cloog_code, stmt_macros, iter_decls,\
  acc_id, acc_name, acc_nrows, vsa, config, misc_info):
  gen_code = []

  # parse stmts
  stmts = {}
  for stmt_macro in stmt_macros:
    stmt_info = {}
    stmt_decl = stmt_macro.strip('\n').split(' ')[1]
    stmt_text = stmt_macro.strip('\n').split(' ')[2]

    # get stmt_name
    stmt_name = stmt_decl[:stmt_decl.find('(')]
    # get stmt_iters
    stmt_iters = stmt_decl[stmt_decl.find('(')+1:stmt_decl.find(')')].split(',')

    stmts[stmt_name] = {}
    stmts[stmt_name]['ITERS'] = stmt_iters
    stmts[stmt_name]['TEXT'] = stmt_text

  # build the map from stmt_iters to loop_iters
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      iter_map = {}
      loop_iters = code_line[code_line.find('(')+1:code_line.find(')')].split(',')
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      for stmt_iter_id in range(len(stmts[stmt_name]['ITERS'])):
        stmt_iter = stmts[stmt_name]['ITERS'][stmt_iter_id]
        iter_map[stmt_iter] = '(' + loop_iters[stmt_iter_id] + ')'
      stmts[stmt_name]['ITER_MAP'] = iter_map

  # modify the cloog code
  array_part_band_width = vsa['ARRAY_PART_BAND_WIDTH']
  engine_band_width = vsa['ENGINE_BAND_WIDTH'][acc_id]

  # print(acc_id, acc_name, acc_nrows)
  # print(cloog_code)

  gen_code = cloog_code[array_part_band_width:len(cloog_code)-array_part_band_width]
  # print(gen_code[0])
  # print(gen_code[0].strip(' '))

  # eliminate unnecssary indent
  indent = len(gen_code[0]) - len(gen_code[0].strip(' '))
  for idx in range(len(gen_code)):
    gen_code[idx] = gen_code[idx][indent:]

  new_gen_code = []
  if engine_band_width > 0:
    new_gen_code.append(gen_code[0])
    new_gen_code.append(' ' * 2 + 'if (%s == %s + engine_id){\n' % \
      (misc_info['ENGINE_ITERS'][0], misc_info['ENGINE_LOOP_PREFIX']))
    for idx in range(1,len(gen_code)-1):
      new_gen_code.append(' ' * 2 + gen_code[idx])
    new_gen_code.append(' ' * 2 + '} else if (%s > %s + engine_id){\n' % \
      (misc_info['ENGINE_ITERS'][0], misc_info['ENGINE_LOOP_PREFIX']))
    for idx in range(1,len(gen_code)-1):
      if gen_code[idx].find('S1') >= 0:
        new_gen_code.append(' ' * 2 + gen_code[idx].replace('S1', 'S2'))
      else :
        new_gen_code.append(' ' * 2 + gen_code[idx])
    new_gen_code.append(' ' * 2 + '}\n')
    new_gen_code.append(gen_code[-1])
  else:
    new_gen_code = gen_code

  cloog_code = new_gen_code

  gen_code = []
  # modify the statements in the cloog code
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      if stmt_name == 'S1':
        # read from FIFO
        # add the pipeline pragma
        new_code_line = code_line[:code_line.find('S') - 2]
        new_code_line += '#pragma HLS PIPELINE II=1\n'
        gen_code.append(new_code_line)

        # get the array reference
        stmt_text = stmts[stmt_name]['TEXT']
        array_ref = []
        stmt_text = stmt_text[stmt_text.find('['):stmt_text.find(' = ')].strip()
        stmt_text = stmt_text.split('[')[1:]
        for ref in stmt_text:
          array_ref.append(ref.strip(']'))
        new_array_ref = gen_new_arr_ref(array_ref, stmts[stmt_name]['ITER_MAP'])

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += '%sData%dTransferChannelType fifo_data = fifo_transfer_in.read();\n' \
          % (config['PREFIX'], acc_id)
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'buffer'
        idx = 0
        for new_ref in new_array_ref:
          new_code_line += '['
          new_code_line += new_ref
          new_code_line += ' - %s' % (misc_info['ARR_LBS'][idx])
          new_code_line += ']'
          idx += 1
        new_code_line += ' = fifo_data;\n'
        gen_code.append(new_code_line)

      elif stmt_name == 'S2':
        # add the pipeline pragma
        new_code_line = code_line[:code_line.find('S') - 2]
        new_code_line += '#pragma HLS PIPELINE II=1\n'
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += '%sData%dTransferChannelType fifo_data = fifo_transfer_in.read();\n' \
          % (config['PREFIX'], acc_id)
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'fifo_transfer_out.write(fifo_data);\n'
        gen_code.append(new_code_line)
    else:
      gen_code.append(code_line)

  # parse the gen_code the second time to add the iterator decls, engine_guards,
  # and data transfer statement between engines
  new_gen_code = []
  # print iter decls
  new_gen_code.append('// iterator declarations\n')
  iters = iter_decls[iter_decls.find('int')+3:iter_decls.find(';')].strip().split(',')
  array_part_iters = misc_info['ARRAY_PART_ITERS']
  engine_iters = misc_info['ENGINE_ITERS']
  for loop_iter in iters:
    loop_iter = loop_iter.strip()
    if loop_iter in array_part_iters:
      continue
    else:
      new_gen_code.append('int %s;\n' % (loop_iter))
  new_gen_code.append('\n')

  new_gen_code += gen_code

  return new_gen_code

def df_engine_read_last_code_modify(cloog_code, stmt_macros, iter_decls,\
  acc_id, acc_name, acc_nrows, vsa, config, misc_info):
  gen_code = []

  # parse stmts
  stmts = {}
  for stmt_macro in stmt_macros:
    stmt_info = {}
    stmt_decl = stmt_macro.strip('\n').split(' ')[1]
    stmt_text = stmt_macro.strip('\n').split(' ')[2]

    # get stmt_name
    stmt_name = stmt_decl[:stmt_decl.find('(')]
    # get stmt_iters
    stmt_iters = stmt_decl[stmt_decl.find('(')+1:stmt_decl.find(')')].split(',')

    stmts[stmt_name] = {}
    stmts[stmt_name]['ITERS'] = stmt_iters
    stmts[stmt_name]['TEXT'] = stmt_text

  # build the map from stmt_iters to loop_iters
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      iter_map = {}
      loop_iters = code_line[code_line.find('(')+1:code_line.find(')')].split(',')
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      for stmt_iter_id in range(len(stmts[stmt_name]['ITERS'])):
        stmt_iter = stmts[stmt_name]['ITERS'][stmt_iter_id]
        iter_map[stmt_iter] = '(' + loop_iters[stmt_iter_id] + ')'
      stmts[stmt_name]['ITER_MAP'] = iter_map

  # modify the cloog code
  array_part_band_width = vsa['ARRAY_PART_BAND_WIDTH']
  engine_band_width = vsa['ENGINE_BAND_WIDTH'][acc_id]

  gen_code = cloog_code[array_part_band_width:len(cloog_code)-array_part_band_width]
  # eliminate unnecssary indent
  indent = len(gen_code[0]) - len(gen_code[0].strip(' '))
  for idx in range(len(gen_code)):
    gen_code[idx] = gen_code[idx][indent:]

  new_gen_code = []
  if engine_band_width > 0:
    new_gen_code.append(gen_code[0])
    new_gen_code.append(' ' * 2 + 'if (%s == %s + engine_id){\n' % \
      (misc_info['ENGINE_ITERS'][0], misc_info['ENGINE_LOOP_PREFIX']))
    for idx in range(1,len(gen_code)-1):
      new_gen_code.append(' ' * 2 + gen_code[idx])
    #new_gen_code.append(' ' * 2 + '} else if (%s > %s + engine_id){\n' % \
    #  (misc_info['ENGINE_ITER'], misc_info['ENGINE_LOOP_PREFIX']))
    new_gen_code.append(' ' * 2 + '}\n')
    new_gen_code.append(gen_code[-1])
  else:
    new_gen_code = gen_code

  cloog_code = new_gen_code

  gen_code = []
  # modify the statements in the cloog code
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      if stmt_name == 'S1':
        # read from FIFO
        # add the pipeline pragma
        new_code_line = code_line[:code_line.find('S') - 2]
        new_code_line += '#pragma HLS PIPELINE II=1\n'
        gen_code.append(new_code_line)

        # get the array reference
        stmt_text = stmts[stmt_name]['TEXT']
        array_ref = []
        stmt_text = stmt_text[stmt_text.find('['):stmt_text.find(' = ')].strip()
        stmt_text = stmt_text.split('[')[1:]
        for ref in stmt_text:
          array_ref.append(ref.strip(']'))
        new_array_ref = gen_new_arr_ref(array_ref, stmts[stmt_name]['ITER_MAP'])

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += '%sData%dTransferChannelType fifo_data = fifo_transfer_in.read();\n' \
          % (config['PREFIX'], acc_id)
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'buffer'
        idx = 0
        for new_ref in new_array_ref:
          new_code_line += '['
          new_code_line += new_ref
          new_code_line += ' - %s' % (misc_info['ARR_LBS'][idx])
          new_code_line += ']'
          idx += 1
        new_code_line += ' = fifo_data;\n'
        gen_code.append(new_code_line)

      elif stmt_name == 'S2':
        # add the pipeline pragma
        new_code_line = code_line[:code_line.find('S') - 2]
        new_code_line += '#pragma HLS PIPELINE II=1\n'
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += '%sData%dTransferChannelType fifo_data = fifo_transfer_in.read();\n' \
          % (config['PREFIX'], acc_id)
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'fifo_transfer_out.write(fifo_data);\n'
        gen_code.append(new_code_line)
    else:
      gen_code.append(code_line)

  # parse the gen_code the second time to add the iterator decls, engine_guards,
  # and data transfer statement between engines
  new_gen_code = []
  # print iter decls
  new_gen_code.append('// iterator declarations\n')
  iters = iter_decls[iter_decls.find('int')+3:iter_decls.find(';')].strip().split(',')
  array_part_iters = misc_info['ARRAY_PART_ITERS']
  engine_iters = misc_info['ENGINE_ITERS']
  for loop_iter in iters:
    loop_iter = loop_iter.strip()
    if loop_iter in array_part_iters:
      continue
    else:
      new_gen_code.append('int %s;\n' % (loop_iter))
  new_gen_code.append('\n')

  new_gen_code += gen_code

  return new_gen_code

def dc_engine_collect_code_modify(cloog_code, stmt_macros, iter_decls, \
  acc_id, acc_name, acc_nrows, vsa, config, misc_info):
  gen_code = []

  # parse stmts
  stmts = {}
  for stmt_macro in stmt_macros:
    stmt_info = {}
    stmt_decl = stmt_macro.strip('\n').split(' ')[1]
    stmt_text = stmt_macro.strip('\n').split(' ')[2]

    # get stmt_name
    stmt_name = stmt_decl[:stmt_decl.find('(')]
    # get stmt_iters
    stmt_iters = stmt_decl[stmt_decl.find('(')+1:stmt_decl.find(')')].split(',')

    stmts[stmt_name] = {}
    stmts[stmt_name]['ITERS'] = stmt_iters
    stmts[stmt_name]['TEXT'] = stmt_text

  # build the map from stmt_iters to loop_iters
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      iter_map = {}
      loop_iters = code_line[code_line.find('(')+1:code_line.find(')')].split(',')
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      for stmt_iter_id in range(len(stmts[stmt_name]['ITERS'])):
        stmt_iter = stmts[stmt_name]['ITERS'][stmt_iter_id]
        iter_map[stmt_iter] = '(' + loop_iters[stmt_iter_id] + ')'
      stmts[stmt_name]['ITER_MAP'] = iter_map

  # modify the statements in the cloog code
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      if stmt_name == 'S1':
        # collect from PE
        # add the pipeline pragma
        new_code_line = code_line[:code_line.find('S') - 2]
        new_code_line += '#pragma HLS PIPELINE II=1\n'
        gen_code.append(new_code_line)

        # get the array reference
        stmt_text = stmts[stmt_name]['TEXT']
        array_ref = []
        stmt_text = stmt_text[stmt_text.find('['):stmt_text.find(')')].strip()
        stmt_text = stmt_text.split('[')[1:]
        for ref in stmt_text:
          array_ref.append(ref.strip(']'))
        new_array_ref = gen_new_arr_ref(array_ref, stmts[stmt_name]['ITER_MAP'])

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += '%sData%dTransferChannelType fifo_data;\n' \
          % (config['PREFIX'], acc_id)
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'fifo_data = fifo_collect.read();\n'
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'buffer'
        idx = 0
        for new_ref in new_array_ref:
          new_code_line += '['
          new_code_line += new_ref
          new_code_line += ' - %s' % (misc_info['ARR_LBS'][idx])
          new_code_line += ']'
          idx += 1
        new_code_line += ' = fifo_data'
        new_code_line += ';\n'
        gen_code.append(new_code_line)
    else:
      gen_code.append(code_line)

  # parse the gen_code the second time to add teh iterator decls, engine guards
  # and other stmts
  new_gen_code = []
  # print iter decls
  new_gen_code.append('// iterator declarations\n')
  iters = iter_decls[iter_decls.find('int')+3:iter_decls.find(';')].strip().split(',')
  array_part_iters = misc_info['ARRAY_PART_ITERS']
  engine_iters = misc_info['ENGINE_ITERS']
  for loop_iter in iters:
    loop_iter = loop_iter.strip()
    if loop_iter in array_part_iters:
      continue
    else:
      new_gen_code.append('int %s;\n' % (loop_iter))
  new_gen_code.append('\n')

  loop_cnt = 0
  array_part_band_width = vsa['ARRAY_PART_BAND_WIDTH']
  engine_band_width = vsa['ENGINE_BAND_WIDTH'][acc_id]
  # TODO: get the actual array_part_band_width
  actual_array_part_band_width = 0
  for code_line in gen_code:
    if code_line.find('for') >= 0:
      for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip()
      iter_name = for_cond[:for_cond.find('=')]
      iter_idx = int(iter_name[1:])

      if iter_idx <= array_part_band_width:
        actual_array_part_band_width += 1

  engine_code = gen_code[actual_array_part_band_width:-actual_array_part_band_width]

  #print(gen_code)
  #print(actual_array_part_band_width)

  engine_loop_skip_idx = engine_code[0].find('for')
  # eliminate all the extra space in each line
  for idx in range(len(engine_code)):
    engine_code[idx] = engine_code[idx][engine_loop_skip_idx:]

  if engine_band_width > 0:
    new_gen_code.append(engine_code[0])
    # add the guards
    new_code_line = '  if (%s == %s + engine_id) {\n' % (misc_info['ENGINE_ITERS'][0], misc_info['ENGINE_LOOP_PREFIX'])
    new_gen_code.append(new_code_line)

    for idx in range(1,len(engine_code)-1):
      new_code_line = ' ' * 2 + engine_code[idx]
      new_gen_code.append(new_code_line)

    new_gen_code.append(' ' * 2 + '}\n')
    new_gen_code.append('}\n')
  else:
    new_gen_code += engine_code

  return new_gen_code

# dc_engine will first transfer the data before it, and then the local data
def dc_engine_write_code_modify(cloog_code, stmt_macros, iter_decls, \
  acc_id, acc_name, acc_nrows, vsa, config, misc_info):
  gen_code = []

  # parse stmts
  stmts = {}
  for stmt_macro in stmt_macros:
    stmt_info = {}
    stmt_decl = stmt_macro.strip('\n').split()[1]
    stmt_text = stmt_macro.strip('\n').split()[2]

    # get stmt_name
    stmt_name = stmt_decl[:stmt_decl.find('(')]
    # get stmt_iters
    stmt_iters = stmt_decl[stmt_decl.find('(')+1:stmt_decl.find(')')].split(',')

    stmts[stmt_name] = {}
    stmts[stmt_name]['ITERS'] = stmt_iters
    stmts[stmt_name]['TEXT'] = stmt_text

  # build the map from stmt_iters to loop_iters
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      iter_map = {}
      loop_iters = code_line[code_line.find('(')+1:code_line.find(')')].split(',')
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      for stmt_iter_id in range(len(stmts[stmt_name]['ITERS'])):
        stmt_iter = stmts[stmt_name]['ITERS'][stmt_iter_id]
        iter_map[stmt_iter] = '(' + loop_iters[stmt_iter_id] + ')'
      stmts[stmt_name]['ITER_MAP'] = iter_map

  # modify the cloog code
  array_part_band_width = vsa['ARRAY_PART_BAND_WIDTH']
  engine_band_width = vsa['ENGINE_BAND_WIDTH'][acc_id]
  actual_array_part_band_width = 0
  for code_line in cloog_code:
    if code_line.find('for') >= 0:
      for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip()
      iter_name = for_cond[:for_cond.find('=')]
      iter_idx = int(iter_name[1:])

      if iter_idx <= array_part_band_width:
        actual_array_part_band_width += 1

  gen_code = cloog_code[actual_array_part_band_width:len(cloog_code)-actual_array_part_band_width]
  # eliminate unnecessary indent
  indent = len(gen_code[0]) - len(gen_code[0].strip(' '))
  for idx in range(len(gen_code)):
    gen_code[idx] = gen_code[idx][indent:]

  new_gen_code = []
  if engine_band_width > 0:
    new_gen_code.append(gen_code[0])
    new_gen_code.append(' ' * 2 + 'if (%s == %s + engine_id){\n' % \
      (misc_info['ENGINE_ITERS'][0], misc_info['ENGINE_LOOP_PREFIX']))
    for idx in range(1,len(gen_code)-1):
      new_gen_code.append(' ' * 2 + gen_code[idx])
    new_gen_code.append(' ' * 2 + '} else if (%s < %s + engine_id){\n' % \
      (misc_info['ENGINE_ITERS'][0], misc_info['ENGINE_LOOP_PREFIX']))
    for idx in range(1,len(gen_code)-1):
      if gen_code[idx].find('S1') >= 0:
        new_gen_code.append(' ' * 2 + gen_code[idx].replace('S1', 'S2'))
      else:
        new_gen_code.append(' ' * 2 + gen_code[idx])
    new_gen_code.append(' ' * 2 + '}\n')
    new_gen_code.append(gen_code[-1])
  else:
    new_gen_code = gen_code

  cloog_code = new_gen_code

  # modify the statements in the cloog code
  gen_code = []
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      if stmt_name == 'S1':
        # write to FIFO
        # add the pipeline pragma
        new_code_line = code_line[:code_line.find('S') - 2]
        new_code_line += '#pragma HLS PIPELINE II=1\n'
        gen_code.append(new_code_line)

        # get the array reference
        stmt_text = stmts[stmt_name]['TEXT']
        #print(stmt_text)
        array_ref = []
        stmt_text = stmt_text[stmt_text.find('['):stmt_text.find(')')].strip()
        #print(stmt_text)
        stmt_text = stmt_text.split('[')[1:]
        #print(stmt_text)
        for ref in stmt_text:
          array_ref.append(ref.strip(']'))
        new_array_ref = gen_new_arr_ref(array_ref, stmts[stmt_name]['ITER_MAP'])

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += '%sData%dTransferChannelType fifo_data;\n' % \
          (config['PREFIX'], acc_id)
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'fifo_data = '
        new_code_line += 'buffer'
        idx = 0
        for new_ref in new_array_ref:
          new_code_line += '['
          new_code_line += new_ref
          new_code_line += ' - %s' % (misc_info['ARR_LBS'][idx])
          new_code_line += ']'
        new_code_line += ';\n'
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'fifo_transfer_out.write(fifo_data);\n'
        gen_code.append(new_code_line)

      elif stmt_name == 'S2':
        # add the pipeline pragma
        new_code_line = code_line[:code_line.find('S') - 2]
        new_code_line += '#pragma HLS PIPELINE II=1\n'
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += '%sData%dTransferChannelType fifo_data = fifo_transfer_in.read();\n' \
          % (config['PREFIX'], acc_id)
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'fifo_transfer_out.write(fifo_data);\n'
        gen_code.append(new_code_line)
    else:
      gen_code.append(code_line)

  # parse the gen_code the second time to add the iterator decls, engine_guards,
  # and data transfer statement between engines
  new_gen_code = []
  # print iter decls
  new_gen_code.append('// iterator declarations\n')
  iters = iter_decls[iter_decls.find('int')+3:iter_decls.find(';')].strip().split(',')
  array_part_iters = misc_info['ARRAY_PART_ITERS']
  engine_iters = misc_info['ENGINE_ITERS']
  for loop_iter in iters:
    loop_iter = loop_iter.strip()
    if loop_iter in array_part_iters:
      continue
    else:
      new_gen_code.append('int %s;\n' % (loop_iter))
  new_gen_code += gen_code

  return new_gen_code

def dc_engine_write_first_code_modify(cloog_code, stmt_macros, iter_decls, \
  acc_id, acc_name, acc_nrows, vsa, config, misc_info):
  gen_code = []

  # parse stmts
  stmts = {}
  for stmt_macro in stmt_macros:
    stmt_info = {}
    stmt_decl = stmt_macro.strip('\n').split()[1]
    stmt_text = stmt_macro.strip('\n').split()[2]

    # get stmt_name
    stmt_name = stmt_decl[:stmt_decl.find('(')]
    # get stmt_iters
    stmt_iters = stmt_decl[stmt_decl.find('(')+1:stmt_decl.find(')')].split(',')

    stmts[stmt_name] = {}
    stmts[stmt_name]['ITERS'] = stmt_iters
    stmts[stmt_name]['TEXT'] = stmt_text

  # build the map from stmt_iters to loop_iters
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      iter_map = {}
      loop_iters = code_line[code_line.find('(')+1:code_line.find(')')].split(',')
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      for stmt_iter_id in range(len(stmts[stmt_name]['ITERS'])):
        stmt_iter = stmts[stmt_name]['ITERS'][stmt_iter_id]
        iter_map[stmt_iter] = '(' + loop_iters[stmt_iter_id] + ')'
      stmts[stmt_name]['ITER_MAP'] = iter_map

  # modify the cloog code
  array_part_band_width = vsa['ARRAY_PART_BAND_WIDTH']
  engine_band_width = vsa['ENGINE_BAND_WIDTH'][acc_id]
  actual_array_part_band_width = 0
  for code_line in cloog_code:
    if code_line.find('for') >= 0:
      for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip()
      iter_name = for_cond[:for_cond.find('=')]
      iter_idx = int(iter_name[1:])

      if iter_idx <= array_part_band_width:
        actual_array_part_band_width += 1

  gen_code = cloog_code[actual_array_part_band_width:len(cloog_code)-actual_array_part_band_width]
  # eliminate unnecessary indent
  indent = len(gen_code[0]) - len(gen_code[0].strip(' '))
  for idx in range(len(gen_code)):
    gen_code[idx] = gen_code[idx][indent:]

  new_gen_code = []
  if engine_band_width > 0:
    new_gen_code.append(gen_code[0])
    new_gen_code.append(' ' * 2 + 'if (%s == %s + engine_id){\n' % \
      (misc_info['ENGINE_ITERS'][0], misc_info['ENGINE_LOOP_PREFIX']))
    for idx in range(1,len(gen_code)-1):
      new_gen_code.append(' ' * 2 + gen_code[idx])
    # new_gen_code.append(' ' * 2 + '} else if (%s < %s + engine_id){\n' % \
    #   (misc_info['ENGINE_ITER'], misc_info['ENGINE_LOOP_PREFIX']))
    # for idx in range(1,len(gen_code)-1):
    #   if gen_code[idx].find('S1') >= 0:
    #     new_gen_code.append(' ' * 4 + gen_code[idx].replace('S1', 'S2'))
    #   else:
    #     new_gen_code.append(' ' * 4 + gen_code[idx])
    new_gen_code.append(' ' * 2 + '}\n')
    new_gen_code.append(gen_code[-1])
  else:
    new_gen_code = gen_code

  cloog_code = new_gen_code

  # modify the statements in the cloog code
  gen_code = []
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      if stmt_name == 'S1':
        # write to FIFO
        # add the pipeline pragma
        new_code_line = code_line[:code_line.find('S') - 2]
        new_code_line += '#pragma HLS PIPELINE II=1\n'
        gen_code.append(new_code_line)

        # get the array reference
        stmt_text = stmts[stmt_name]['TEXT']
        array_ref = []
        stmt_text = stmt_text[stmt_text.find('['):stmt_text.find(')')].strip()
        stmt_text = stmt_text.split('[')[1:]
        for ref in stmt_text:
          array_ref.append(ref.strip(']'))
        new_array_ref = gen_new_arr_ref(array_ref, stmts[stmt_name]['ITER_MAP'])

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += '%sData%dTransferChannelType fifo_data;\n' % \
          (config['PREFIX'], acc_id)
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'fifo_data = '
        new_code_line += 'buffer'
        idx = 0
        for new_ref in new_array_ref:
          new_code_line += '['
          new_code_line += new_ref
          new_code_line += ' - %s' % (misc_info['ARR_LBS'][idx])
          new_code_line += ']'
        new_code_line += ';\n'
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'fifo_transfer_out.write(fifo_data);\n'
        gen_code.append(new_code_line)

      elif stmt_name == 'S2':
        # add the pipeline pragma
        new_code_line = code_line[:code_line.find('S') - 2]
        new_code_line += '#pragma HLS PIPELINE II=1\n'
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += '%sData%dTransferChannelType fifo_data = fifo_transfer_in.read();\n' \
          % (config['PREFIX'], acc_id)
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'fifo_transfer_out.write(fifo_data);\n'
        gen_code.append(new_code_line)
    else:
      gen_code.append(code_line)

  # parse the gen_code the second time to add the iterator decls, engine_guards,
  # and data transfer statement between engines
  new_gen_code = []
  # print iter decls
  new_gen_code.append('// iterator declarations\n')
  iters = iter_decls[iter_decls.find('int')+3:iter_decls.find(';')].strip().split(',')
  array_part_iters = misc_info['ARRAY_PART_ITERS']
  engine_iters = misc_info['ENGINE_ITERS']
  for loop_iter in iters:
    loop_iter = loop_iter.strip()
    if loop_iter in array_part_iters:
      continue
    else:
      new_gen_code.append('int %s;\n' % (loop_iter))
  new_gen_code += gen_code

  return new_gen_code

def extract_dc_engine_misc_from_code(vsa, config, acc_id, acc_name, acc_nrows):
  misc_info = {}

  file_name = config['CODE_DIR'] + '/' + acc_name + '_dc_engine_write_code.c'
  content = []
  with open(file_name, 'r') as f:
    for line in f.readlines():
      content.append(line)

  array_decls = []
  stmt_macros = []
  iter_decls = []
  cloog_code = []

  [array_decls, stmt_macros, iter_decls, cloog_code] = code_parse(content)

  # ARRAY_DECLS
  # e.g., "A": [32][32]
  misc_info['ARRAY_DECLS'] = {}
  for array_decl in array_decls:
    array_decl = array_decl.strip('\n').strip(';').split(' ')[1]
    # get array_name
    array_name = array_decl[:array_decl.find('[')]
    array_name = array_name.split('_')[-1]
    # get array_extent
    array_extent = array_decl[array_decl.find('['):]

    misc_info['ARRAY_DECLS'][array_name] = array_extent

  # ARRAY_PART_LOOPS
  # e.g.,
  # for (t1=0;t1<...) {
  #   for (t2=0;t2<...) {

  # LAST_ARRAY_PART_TILE_COND
  # e.g., t1 == 31 && t2 == 31 && t3 == 31

  # FIRST_ARRY_PART_TILE_COND
  # e.g., t1 == 0 && t2 == 0 && t3 == 0

  # ARRAY_PART_ITER_DECLS
  # e.g., int t1, int t2, ...
  array_part_iters = []
  engine_iters = []
  # other_iters = []
  array_part_iter_ubs = []
  array_part_iter_lbs = []
  array_part_band_width = vsa['ARRAY_PART_BAND_WIDTH']
  # NOTE: the number of actual array part loops may not equal to the array_part_band_width
  # This needs to be taken care of
  engine_band_width = vsa['ENGINE_BAND_WIDTH'][acc_id]

  array_part_loops = []

  for code_line in cloog_code:
    if code_line.find('for') >= 0:
      for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip(' ')
      iter_name = for_cond[:for_cond.find('=')]
      iter_idx = int(iter_name[1:])

      if iter_idx <= array_part_band_width:
        array_part_loops.append(code_line)
        array_part_iters.append(iter_name)
        tmp_cond = for_cond[for_cond.find('<=')+2:]
        ub = tmp_cond[:tmp_cond.find(';')]
        array_part_iter_ubs.append(ub)

        lb = for_cond[for_cond.find('=')+1:for_cond.find(';')]
        array_part_iter_lbs.append(lb)

      elif iter_idx > array_part_band_width and iter_idx <= array_part_band_width + \
        engine_band_width:
        engine_iters.append(iter_name)

  # get the lower bounds for local buffer
  arr_lbs = []
  for code_line in cloog_code:
    if code_line.find('for') >= 0:
      for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip(' ')
      iter_name = for_cond[:for_cond.find('=')]
      iter_idx = int(iter_name[1:])

      if iter_idx > array_part_band_width + engine_band_width:
        lb = for_cond[for_cond.find('=')+1:for_cond.find(';')]
        arr_lbs.append(lb)

  misc_info['ARR_LBS'] = arr_lbs
  misc_info['ARRAY_PART_LOOPS'] = array_part_loops
  misc_info['ARRAY_PART_ITER_UBS'] = array_part_iter_ubs
  misc_info['ARRAY_PART_ITER_LBS'] = array_part_iter_lbs

  last_array_part_tile_cond = ""
  for iter_idx in range(len(array_part_iters)):
    iter_name = array_part_iters[iter_idx]
    iter_ub = array_part_iter_ubs[iter_idx]
    last_array_part_tile_cond += '%s == %s' % (iter_name, iter_ub)
    if iter_idx < len(array_part_iters) - 1:
      last_array_part_tile_cond += '&&'
  misc_info['LAST_ARRAY_PART_TILE_COND'] = last_array_part_tile_cond

  first_array_part_tile_cond = ""
  for iter_idx in range(len(array_part_iters)):
    iter_name = array_part_iters[iter_idx]
    iter_lb = array_part_iter_lbs[iter_idx]
    first_array_part_tile_cond += '(%s == %s)' % (iter_name, iter_lb)
    if iter_idx < len(array_part_iters) - 1:
      first_array_part_tile_cond += ' && '
  misc_info['FIRST_ARRAY_PART_TILE_COND'] = first_array_part_tile_cond

  array_part_iter_decls = []
  engine_iter_decls = []
  for iter_name in array_part_iters:
    array_part_iter_decls.append('int ' + iter_name)
  for iter_name in engine_iters:
    engine_iter_decls.append('int ' + iter_name)

  misc_info['ARRAY_PART_ITERS'] = array_part_iters
  misc_info['ENGINE_ITERS'] = engine_iters
  misc_info['ARRAY_PART_ITER_DECLS'] = array_part_iter_decls
  misc_info['ENGINE_ITER_DECLS'] = engine_iter_decls

  # ENGINE_LOOP_PREFIX
  for code_line in cloog_code:
    if code_line.find('for') >= 0:
      for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip(' ')
      iter_name = for_cond[:for_cond.find('=')]
      iter_idx = int(iter_name[1:])

      if iter_idx > array_part_band_width and iter_idx <= array_part_band_width + \
        engine_band_width:
        lb = for_cond[for_cond.find('=')+1:for_cond.find(';')]
        misc_info['ENGINE_LOOP_PREFIX'] = lb
        break

  # ENGINE_COLLECT_CODE
  file_name = config['CODE_DIR'] + '/' + acc_name + '_dc_engine_collect_code.c'
  content = []
  with open(file_name, 'r') as f:
    for line in f.readlines():
      content.append(line)

  array_decls = []
  stmt_macros = []
  iter_decls = []
  cloog_code = []

  [array_decls, stmt_macros, iter_decls, cloog_code] = code_parse(content)
  gen_code = dc_engine_collect_code_modify(cloog_code, stmt_macros, iter_decls, \
    acc_id, acc_name, acc_nrows, vsa, config, misc_info)
  misc_info['ENGINE_COLLECT_CODE'] = gen_code

  # ENGINE_WRITE_CODE
  file_name = config['CODE_DIR'] + '/' + acc_name + '_dc_engine_write_code.c'
  content = []
  with open(file_name, 'r') as f:
    for line in f.readlines():
      content.append(line)

  array_decls = []
  stmt_macros = []
  iter_decls = []
  cloog_code = []

  [array_decls, stmt_macros, iter_decls, cloog_code] = code_parse(content)
  gen_code = dc_engine_write_code_modify(cloog_code, stmt_macros, iter_decls, \
    acc_id, acc_name, acc_nrows, vsa, config, misc_info)
  misc_info['ENGINE_WRITE_CODE'] = gen_code

  # ENGINE_WRITE_LAST_CODE
  file_name = config['CODE_DIR'] + '/' + acc_name + '_dc_engine_write_code.c'
  content = []
  with open(file_name, 'r') as f:
    for line in f.readlines():
      content.append(line)

  array_decls = []
  stmt_macros = []
  iter_decls = []
  cloog_code = []

  [array_decls, stmt_macros, iter_decls, cloog_code] = code_parse(content)
  gen_code = dc_engine_write_first_code_modify(cloog_code, stmt_macros, iter_decls, \
    acc_id, acc_name, acc_nrows, vsa, config, misc_info)
  misc_info['ENGINE_WRITE_FIRST_CODE'] = gen_code

  return misc_info

def op_transfer_code_modify(cloog_code, stmt_macros, iter_decls, \
  acc_id, acc_name, acc_nrows, vsa, config, misc_info):
  gen_code = []

  # parse stmts
  stmts = {}
  for stmt_macro in stmt_macros:
    stmt_info = {}
    stmt_decl = stmt_macro.strip('\n').split(' ')[1]
    stmt_text = stmt_macro.strip('\n').split(' ')[2]

    # get stmt_name
    stmt_name = stmt_decl[:stmt_decl.find('(')]
    # get stmt_iters
    stmt_iters = stmt_decl[stmt_decl.find('(')+1:stmt_decl.find(')')].split(',')

    stmts[stmt_name] = {}
    stmts[stmt_name]['ITERS'] = stmt_iters
    stmts[stmt_name]['TEXT'] = stmt_text

  # build the map from stmt_iters to loop_iters
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      iter_map = {}
      loop_iters = code_line[code_line.find('(')+1:code_line.find(')')].split(',')
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      for stmt_iter_id in range(len(stmts[stmt_name]['ITERS'])):
        stmt_iter = stmts[stmt_name]['ITERS'][stmt_iter_id]
        iter_map[stmt_iter] = '(' + loop_iters[stmt_iter_id] + ')'
      stmts[stmt_name]['ITER_MAP'] = iter_map

  # modify the statements in the cloog code
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      if stmt_name == 'S1':
        # add the pipeline pragma
        new_code_line = code_line[:code_line.find('S') - 2]
        new_code_line += '#pragma HLS PIPELINE II=1\n'
        gen_code.append(new_code_line)

        # get the array reference
        stmt_text = stmts[stmt_name]['TEXT']
        array_ref = []
        stmt_text = stmt_text[stmt_text.find('['):stmt_text.find(')')].strip()
        stmt_text = stmt_text.split('[')[1:]
        for ref in stmt_text:
          array_ref.append(ref.strip(']'))
        new_array_ref = gen_new_arr_ref(array_ref, stmts[stmt_name]['ITER_MAP'])

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += '%sData%dTransferChannelType fifo_data;\n'\
          % (config['PREFIX'], acc_id)
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'fifo_data = fifo_in.read();\n'
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'fifo_out.write(fifo_data);\n'
        gen_code.append(new_code_line)
    else:
      gen_code.append(code_line)

  # parse the gen_code the second time to add the iterator decls, space gurads
  # and other stmts
  new_gen_code = []
  # print iter decls
  new_gen_code.append('// iterator declarations\n')
  for loop_iter in misc_info['ITERS']:
    new_gen_code.append('int %s;\n' % (loop_iter))
  new_gen_code.append('\n')

  loop_cnt = 0
  array_part_band_width = vsa['ARRAY_PART_BAND_WIDTH']
  space_band_width = vsa['SPACE_BAND_WIDTH']

  # add array partition loops
  for line_id in range(array_part_band_width):
    new_gen_code.append(gen_code[line_id])

  # add space loops
  space_pe_code = gen_code[array_part_band_width:-array_part_band_width]
  space_pe_loop_skip_idx = space_pe_code[0].find('for')

  indent_inc_level = 0
  for line_id in range(space_band_width):
    new_gen_code.append(' ' * indent_inc_level + space_pe_code[line_id])
    indent_inc_level += 1
    if line_id - array_part_band_width == 0:
      new_code_line = ' ' * indent_inc_level + \
        'if (%s == %s + row_id) {\n' % (misc_info['SPACE_ITERS'][line_id - array_part_band_width], \
          misc_info['SPACE_LOOP_PREFIXS'][line_id - array_part_band_width])
    else:
      new_code_line = ' ' * indent_inc_level + \
        'if (%s == %s + col_id) {\n' % (misc_info['SPACE_ITERS'][line_id - array_part_band_width], \
          misc_info['SPACE_LOOP_PREFIXS'][line_id - array_part_band_width])
    new_gen_code.append(new_code_line)

  for line_id in range(space_band_width:):
    new_gen_code.append(' ' * indent_inc_level + space_pe_code[line_id])

  for line_id in range(space_band_width):
    new_gen_code.append(' ' * (space_band_width - 1 - line_id) + '}\n')

  for line_id in range(array_part_band_width):
    new_gen_code.append(gen_code[-array_part_band_width + line_id])

  return new_gen_code

def op_transfer_last_code_modify(cloog_code, stmt_macros, iter_decls, \
  acc_id, acc_name, acc_nrows, vsa, config, misc_info):
  gen_code = []

  # parse stmts
  stmts = {}
  for stmt_macro in stmt_macros:
    stmt_info = {}
    stmt_decl = stmt_macro.strip('\n').split(' ')[1]
    stmt_text = stmt_macro.strip('\n').split(' ')[2]

    # get stmt_name
    stmt_name = stmt_decl[:stmt_decl.find('(')]
    # get stmt_iters
    stmt_iters = stmt_decl[stmt_decl.find('(')+1:stmt_decl.find(')')].split(',')

    stmts[stmt_name] = {}
    stmts[stmt_name]['ITERS'] = stmt_iters
    stmts[stmt_name]['TEXT'] = stmt_text

  # build the map from stmt_iters to loop_iters
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      iter_map = {}
      loop_iters = code_line[code_line.find('(')+1:code_line.find(')')].split(',')
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      for stmt_iter_id in range(len(stmts[stmt_name]['ITERS'])):
        stmt_iter = stmts[stmt_name]['ITERS'][stmt_iter_id]
        iter_map[stmt_iter] = '(' + loop_iters[stmt_iter_id] + ')'
      stmts[stmt_name]['ITER_MAP'] = iter_map

  # modify the statements in the cloog code
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      if stmt_name == 'S1':
        # add the pipeline pragma
        new_code_line = code_line[:code_line.find('S') - 2]
        new_code_line += '#pragma HLS PIPELINE II=1\n'
        gen_code.append(new_code_line)

        # get the array reference
        stmt_text = stmts[stmt_name]['TEXT']
        array_ref = []
        stmt_text = stmt_text[stmt_text.find('['):stmt_text.find(')')].strip()
        stmt_text = stmt_text.split('[')[1:]
        for ref in stmt_text:
          array_ref.append(ref.strip(']'))
        new_array_ref = gen_new_arr_ref(array_ref, stmts[stmt_name]['ITER_MAP'])

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += '%sData%dTransferChannelType fifo_data;\n'\
          % (config['PREFIX'], acc_id)
        gen_code.append(new_code_line)

        new_code_line = code_line[:code_line.find('S')]
        new_code_line += 'fifo_data = fifo_in.read();\n'
        gen_code.append(new_code_line)

        #new_code_line = code_line[:code_line.find('S')]
        #new_code_line += 'fifo_out.write(fifo_data);\n'
        #gen_code.append(new_code_line)
    else:
      gen_code.append(code_line)

  # parse the gen_code the second time to add the iterator decls, space gurads
  # and other stmts
  new_gen_code = []
  # print iter decls
  new_gen_code.append('// iterator declarations\n')
  for loop_iter in misc_info['ITERS']:
    new_gen_code.append('int %s;\n' % (loop_iter))
  new_gen_code.append('\n')

  loop_cnt = 0
  array_part_band_width = vsa['ARRAY_PART_BAND_WIDTH']
  space_band_width = vsa['SPACE_BAND_WIDTH']

  # add array partition loops
  for line_id in range(array_part_band_width):
    new_gen_code.append(gen_code[line_id])

  # add space loops
  space_pe_code = gen_code[array_part_band_width:-array_part_band_width]
  space_pe_loop_skip_idx = space_pe_code[0].find('for')

  indent_inc_level = 0
  for line_id in range(space_band_width):
    new_gen_code.append(' ' * indent_inc_level + space_pe_code[line_id])
    indent_inc_level += 1
    if line_id - array_part_band_width == 0:
      new_code_line = ' ' * indent_inc_level + \
        'if (%s == %s + row_id) {\n' % (misc_info['SPACE_ITERS'][line_id - array_part_band_width], \
          misc_info['SPACE_LOOP_PREFIXS'][line_id - array_part_band_width])
    else:
      new_code_line = ' ' * indent_inc_level + \
        'if (%s == %s + col_id) {\n' % (misc_info['SPACE_ITERS'][line_id - array_part_band_width], \
          misc_info['SPACE_LOOP_PREFIXS'][line_id - array_part_band_width])
    new_gen_code.append(new_code_line)

  for line_id in range(space_band_width:):
    new_gen_code.append(' ' * indent_inc_level + space_pe_code[line_id])

  for line_id in range(space_band_width):
    new_gen_code.append(' ' * (space_band_width - 1 - line_id) + '}\n')

  for line_id in range(array_part_band_width):
    new_gen_code.append(gen_code[-array_part_band_width + line_id])

  return new_gen_code

def extract_op_transfer_misc_from_code(vsa, config, acc_id, acc_name, acc_nrows):
  misc_info = {}

  file_name = config['CODE_DIR'] + '/' + acc_name + '_op_transfer_code.c'
  content = []
  with open(file_name, 'r') as f:
    for line in f.readlines():
      content.append(line)

  array_decls = []
  stmt_macros = []
  iter_decls = []
  cloog_code = []

  [array_decls, stmt_macros, iter_decls, cloog_code] = code_parse(content)

  # ITERS
  # e.g., t1, t2, t3, t4
  iters = iter_decls[iter_decls.find('t'):iter_decls.find(';')].split(',').strip(' ')
  misc_info['ITERS'] = iters

  # ARRAY_DECLS
  misc_info['ARRAY_DECLS'] = {}
  for array_decl in array_decls:
    array_decl = array_decl.strip('\n').strip(';').split(' ')[1]
    # get array_name
    array_name = array_decl[:array_decl.find('[')]
    array_name = array_name.split('_')[-1]
    # get array_extent
    array_extent = array_decl[array_decl.find('['):]

    misc_info['ARRAY_DECLS'][array_name] = array_extent
 
  # ARRAY_PART_LOOPS
  array_part_band_width = vsa['ARRAY_PART_BAND_WIDTH']
  space_band_width = vsa['SPACE_BAND_WIDTH']

  array_part_iters = []
  space_iters = []
  
  array_part_loops = []

  loop_cnt = 0
  for code_line in cloog_code:
    if loop_cnt < array_part_band_width and code_line.find('for') >= 0:
      array_part_loops.append(code_line)
      for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip(' ')
      iter_name = for_cond[:for_cond.find('=')]
      array_part_iters.append(iter_name)
      
      loop_cnt += 1
    elif loop_cnt >= array_part_band_width and loop_cnt < array_part_band_width + \
      space_band_width:
      for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip(' ')
      iter_name = for_cond[:for_cond.find('=')]
      space_iters.append(iter_name)

  misc_info['ARRAY_PART_ITERS'] = array_part_iters
  misc_info['SPACE_ITERS'] = space_iters
  misc_info['ARRAY_PART_LOOPS'] = array_part_loops

  # SPACE_LOOP_PREFIX
  space_loop_prefixs = []
  loop_cnt = 0
  for code_line in cloog_code:
    if code_line.find('for') >= 0:
      if loop_cnt >= array_part_band_width and loop_cnt < array_part_band_width \
        + engine_band_width:
        for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip(' ')
        lb = for_cond[for_cond.find('=')+1:for_cond.find(';')]
        space_loop_prefixs.append(lb) 
      loop_cnt += 1

  misc_info['SPACE_LOOP_PREFIXS'] = space_loop_prefixs

  # TRANSFER_CODE
  gen_code = op_transfer_code_modify(cloog_code, stmt_macros, iter_decls, \
    acc_id, acc_name, acc_nrows, vsa, config, misc_info)
  misc_info['TRANSFER_CODE'] = gen_code

  # TRANSFER_LAST_CODE
  gen_code = op_transfer_last_code_modify(cloog_code, stmt_macros, iter_decls, \
    acc_id, acc_name, acc_nrows, vsa, config, misc_info)
  misc_info['TRANSFER_LAST_CODE'] = gen_code

  return misc_info

# TODO: to be finished
def res_transfer_code_modify(cloog_code, stmt_macros, iter_decls, \
  acc_id, acc_name, acc_nrows, vsa, config, misc_info):
  gen_code = []

  # parse stmts
  stmts = {}
  for stmt_macro in stmt_macros:
    stmt_info = {}
    stmt_decl = stmt_macro.strip('\n').split(' ')[1]
    stmt_text = stmt_macro.strip('\n').split(' ')[2]

    # get stmt_name
    stmt_name = stmt_decl[:stmt_decl.find('(')]
    # get stmt_iters
    stmt_iters = stmt_decl[stmt_decl.find('(')+1:stmt_decl.find(')')].split(',')

    stmts[stmt_name] = {}
    stmts[stmt_name]['ITERS'] = stmt_iters
    stmts[stmt_name]['TEXT'] = stmt_text

  # build the map from stmt_iters to loop_iters
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      iter_map = {}
      loop_iters = code_line[code_line.find('(')+1:code_line.find(')')].split(',')
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      for stmt_iter_id in range(len(stmts[stmt_name]['ITERS'])):
        stmt_iter = stmts[stmt_name]['ITERS'][stmt_iter_id]
        iter_map[stmt_iter] = '(' + loop_iters[stmt_iter_id] + ')'
      stmts[stmt_name]['ITER_MAP'] = iter_map

  # modify the statements in the cloog code

  # parse the gen_code the second time to add the iterator decls, space guards
  # and other stmts
  new_gen_code = []

  return new_gen_code    

#TODO: to be finished
def res_transfer_first_code_modify(cloog_code, stmt_macros, iter_decls, \
  acc_id, acc_name, acc_nrows, vsa, config, misc_info):
  gen_code = []

  # parse stmts
  stmts = {}
  for stmt_macro in stmt_macros:
    stmt_info = {}
    stmt_decl = stmt_macro.strip('\n').split(' ')[1]
    stmt_text = stmt_macro.strip('\n').split(' ')[2]

    # get stmt_name
    stmt_name = stmt_decl[:stmt_decl.find('(')]
    # get stmt_iters
    stmt_iters = stmt_decl[stmt_decl.find('(')+1:stmt_decl.find(')')].split(',')

    stmts[stmt_name] = {}
    stmts[stmt_name]['ITERS'] = stmt_iters
    stmts[stmt_name]['TEXT'] = stmt_text

  # build the map from stmt_iters to loop_iters
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      iter_map = {}
      loop_iters = code_line[code_line.find('(')+1:code_line.find(')')].split(',')
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      for stmt_iter_id in range(len(stmts[stmt_name]['ITERS'])):
        stmt_iter = stmts[stmt_name]['ITERS'][stmt_iter_id]
        iter_map[stmt_iter] = '(' + loop_iters[stmt_iter_id] + ')'
      stmts[stmt_name]['ITER_MAP'] = iter_map

  # modify the statements in the cloog code

  # parse the gen_code the second time to add the iterator decls, space guards
  # and other stmts
  new_gen_code = []

  return new_gen_code    

# TODO: this module needs further modification
def extract_res_transfer_misc_from_code(vsa, config, acc_id, acc_name, acc_nrows):
  misc_info = {}

  file_name = config['CODE_DIR'] + '/' + acc_name + '_res_transfer_code.c'
  content = []
  with open(file_name, 'r') as f:
    for line in f.readlines():
      content.append(line)

  array_decls = []
  stmt_macros = []
  iter_decls = []
  cloog_code = []

  [array_decls, stmt_macros, iter_decls, cloog_code] = code_parse(content)

  # ITERS
  # e.g., t1, t2, t3, t4
  iters = iter_decls[iter_decls.find('t'):iter_decls.find(';')].split(',').strip(' ')
  misc_info['ITERS'] = iters

  # ARRAY_DECLS
  misc_info['ARRAY_DECLS'] = {}
  for array_decl in array_decls:
    array_decl = array_decl.strip('\n').strip(';').split(' ')[1]
    # get array_name
    array_name = array_decl[:array_decl.find('[')]
    array_name = array_name.split('_')[-1]
    # get array_extent
    array_extent = array_decl[array_decl.find('['):]

    misc_info['ARRAY_DECLS'][array_name] = array_extent

  # ARRAY_PART_LOOPs
  array_part_band_width = vsa['ARRAY_PART_BAND_WIDTH']
  space_band_width = vsa['SPACE_BAND_WIDTH']
  # TODO: get the actual array_part_band_width
  actual_array_part_band_width = 0
  for code_line in cloog_code:
    if code_line.find('for') >= 0:
      for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip()
      iter_name = for_cond[:for_cond.find('=')]
      iter_idx = int(iter_name[1:])

      if iter_idx <= array_part_band_width:
        actual_array_part_band_width += 1

  array_part_iters = []
  space_iters = []
  
  array_part_loops = []

  loop_cnt = 0
  for code_line in cloog_code:
    if loop_cnt < actual_array_part_band_width and code_line.find('for') >= 0:
      array_part_loops.append(code_line)
      for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip(' ')
      iter_name = for_cond[:for_cond.find('=')]
      array_part_iters.append(iter_name)
      
      loop_cnt += 1
    elif loop_cnt >= actual_array_part_band_width and loop_cnt < actual_array_part_band_width + \
      space_band_width:
      for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip(' ')
      iter_name = for_cond[:for_cond.find('=')]
      space_iters.append(iter_name)

  misc_info['ARRAY_PART_ITERS'] = array_part_iters
  misc_info['SPACE_ITERS'] = space_iters
  misc_info['ARRAY_PART_LOOPS'] = array_part_loops

  # SPACE_LOOP_PREFIX
  space_loop_prefixs = []
  loop_cnt = 0
  for code_line in cloog_code:
    if code_line.find('for') >= 0:
      if loop_cnt >= actual_array_part_band_width and loop_cnt < actual_array_part_band_width \
        + engine_band_width:
        for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip(' ')
        lb = for_cond[for_cond.find('=')+1:for_cond.find(';')]
        space_loop_prefixs.append(lb) 
      loop_cnt += 1

  misc_info['SPACE_LOOP_PREFIXS'] = space_loop_prefixs

  # TRANSFER_CODE
  gen_code = res_transfer_code_modify(cloog_code, stmt_macros, iter_decls, \
    acc_id, acc_name, acc_nrows, vsa, config, misc_info)
  misc_info['TRANSFER_CODE'] = gen_code

  # TRANSFER_FIRST_CODE
  gen_code = res_transfer_first_code_modify(cloog_code, stmt_macros, iter_decls, \
    acc_id, acc_name, acc_nrows, vsa, config, misc_info)
  misc_info['TRANSFER_FIRST_CODE'] = gen_code

  return misc_info

def compute_code_modify(cloog_code, stmt_macros, iter_decls, vsa, config, misc_info):
  gen_code = []

  # parse stmts
  stmts = {}
  for stmt_macro in stmt_macros:
    stmt_info = {}
    stmt_decl = stmt_macro.strip('\n').split(' ')[1]
    stmt_text = stmt_macro.strip('\n').split(' ')[2]

    # get stmt_name
    stmt_name = stmt_decl[:stmt_decl.find('(')]
    # get stmt_iters
    stmt_iters = stmt_decl[stmt_decl.find('(')+1:stmt_decl.find(')')].split(',')

    stmts[stmt_name] = {}
    stmts[stmt_name]['ITERS'] = stmt_iters
    stmts[stmt_name]['TEXT'] = stmt_text

  # build the map from stmt_iters to loop_iters
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      iter_map = {}
      loop_iters = code_line[code_line.find('(')+1:code_line.find(')')].split(',')
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      for stmt_iter_id in range(len(stmts[stmt_name]['ITERS'])):
        stmt_iter = stmts[stmt_name]['ITERS'][stmt_iter_id]
        iter_map[stmt_iter] = '(' + loop_iters[stmt_iter_id] + ')'
      stmts[stmt_name]['ITER_MAP'] = iter_map

  # modify the statements in the cloog code
  op_stmt_cnt = 0
  res_stmt_cnt = 0
  op_stmt_name_list = ['S' + str(id) for id in range(2,2+len(vsa['OP_NAME']))]
  res_stmt_name_list = ['S' + str(id) for id in range(2+len(vsa['OP_NAME']),2+len(vsa['OP_NAME']+len(vsa['RES_NAME'])))]
  for line_id in range(len(cloog_code)):
    code_line = cloog_code[line_id]
    if code_line.find('S') >= 0:
      stmt_name = code_line[code_line.find('S'):code_line.find('(')]
      if stmt_name == 'S1':
        # compute stmt
        # add the pipeline pragma
        new_code_line = code_line[:code_line.find('S') - 2]
        new_code_line += '#pragma HLS PIPELINE II=1\n'
        gen_code.append(new_code_line)

        # get the array reference
        # TODO: regular expression to replace the original refernece with the new reference
        stmt_text = stmts[stmt_name]['TEXT']
        array_ref = []
        for res_name in vsa['RES_NAME']:
          stmt_text = stmt_text[stmt_text.find(res_name + '[')]
      elif stmt_name in op_stmt_name_list:

      elif stmt_name in res_stmt_name_list:

  # parse the gen_code the second time to add the iterators decls, space guards 
  # and other stmts
  new_gen_code = []
  # TODO

  return new_gen_code

def extract_compute_misc_from_code(vsa, config):
  misc_info = {}

  file_name = config['CODE_DIR'] + '/' + 'compute_code.c'
  content = []
  with open(file_name, 'r') as f:
    for line in f.readlines():
      content.append(line)

  array_decls = []
  stmt_macros = []
  iter_decls = []
  cloog_code = []

  [array_decls, stmt_macros, iter_decls, cloog_code] = code_parse(content)

  # ITERS
  iters = iter_decls[iter_decls.find('t'):iter_decls.find(';')].split(',').strip(' ')
  misc_info['ITERS'] = iters

  # ARRAY_DECLS
  misc_info['ARRAY_DECLS'] = {}
  for array_decl in array_decls:
    array_decl = array_decl.strip('\n').strip(';').split(' ')[1]
    # get array_name
    array_name = array_decl[:array_decl.find('[')]
    array_name = array_name.split('_')[-1]
    # get array_extent
    array_extent = array_decl[array_decl.find('['):]

    misc_info['ARRAY_DECLS'][array_name] = array_extent

  # ARRAY_PART_LOOPS
  array_part_band_width = vsa['ARRAY_PART_BAND_WIDTH']
  space_band_width = vsa['SPACE_BAND_WIDTH']

  array_part_iters = []
  space_iters = []
  
  array_part_loops = []

  loop_cnt = 0
  for code_line in cloog_code:
    if loop_cnt < array_part_band_width and code_line.find('for') >= 0:
      array_part_loops.append(code_line)
      for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip(' ')
      iter_name = for_cond[:for_cond.find('=')]
      array_part_iters.append(iter_name)
      
      loop_cnt += 1
    elif loop_cnt >= array_part_band_width and loop_cnt < array_part_band_width + \
      space_band_width:
      for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip(' ')
      iter_name = for_cond[:for_cond.find('=')]
      space_iters.append(iter_name)

  misc_info['ARRAY_PART_ITERS'] = array_part_iters
  misc_info['SPACE_ITERS'] = space_iters
  misc_info['ARRAY_PART_LOOPS'] = array_part_loops  

  # SPACE_LOOP_PREFIX
  space_loop_prefixs = []
  loop_cnt = 0
  for code_line in cloog_code:
    if code_line.find('for') >= 0:
      if loop_cnt >= array_part_band_width and loop_cnt < array_part_band_width \
        + engine_band_width:
        for_cond = code_line[code_line.find('(')+1: code_line.find('{')].strip(' ')
        lb = for_cond[for_cond.find('=')+1:for_cond.find(';')]
        space_loop_prefixs.append(lb) 
      loop_cnt += 1

  misc_info['SPACE_LOOP_PREFIXS'] = space_loop_prefixs

  # COMPUTE_CODE
  gen_code = compute_code_modify(cloog_code, stmt_macros, iter_decls, \
    vsa, config, misc_info)
  misc_info['COMPUTE_CODE'] = gen_code

  return misc_info