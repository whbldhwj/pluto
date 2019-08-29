/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 * 
 * Author: Jie Wang
 */
#include "pluto.h"
#include "assert.h"
#include "psa_vsa_pe.h"

/* Generate modules inside PEs, including:
 * op_transfer, compute, and res_transfer
 */
void vsa_pe_code_extract(PlutoProg *prog, VSA *vsa) {
  int op_idx;
  int res_idx;

  /* Get the tile and space band */    
  int nbands;
  Band **bands;
  bands = psa_get_part_space_bands(prog, &nbands);

  /* Get the statements under the band */
  Ploop **loops;
  unsigned nloops;
  loops = psa_get_intra_tile_dist_loops(bands[0], prog, &nloops);

  /* Get read/write access pairs with statements */
  unsigned l;
  int *num_read_data, *num_write_data, *num_read_write_data, *num_data;
  int *array_part_level, *space_level, *time_level;

  /* helper statements */
  Stmt ****op_trans_stmts;
  Stmt ****res_trans_stmts;
  Stmt ****compute_stmts;

  num_write_data = (int *)malloc(nloops * sizeof(int));
  num_read_data = (int *)malloc(nloops * sizeof(int));
  num_read_write_data = (int *)malloc(nloops * sizeof(int));
  num_data = (int *)malloc(nloops * sizeof(int));

  array_part_level = (int *)malloc(nloops * sizeof(int));
  space_level = (int *)malloc(nloops * sizeof(int));
  time_level = (int *)malloc(nloops * sizeof(int));

  op_trans_stmts = (Stmt ****)malloc(nloops * sizeof(Stmt ****));
  res_trans_stmts = (Stmt ****)malloc(nloops * sizeof(Stmt ****));
  compute_stmts = (Stmt ****)malloc(nloops * sizeof(Stmt ****));

  psa_init_level(prog, loops, nloops, array_part_level, space_level, time_level);

  for (l = 0; l < nloops; l++) {
    int i;
    Ploop *loop = loops[l];

    int *num_stmts_per_racc;
    int *num_stmts_per_wacc;
    int *num_stmts_per_acc;

    struct stmt_access_pair ***racc_stmts;
    struct stmt_access_pair ***wacc_stmts;
    struct stmt_access_pair ***acc_stmts;

    racc_stmts = get_read_access_with_stmts(
      loop->stmts, loop->nstmts, &num_read_data[l], &num_stmts_per_racc);
    wacc_stmts = get_write_access_with_stmts(
      loop->stmts, loop->nstmts, &num_write_data[l], &num_stmts_per_wacc);
    acc_stmts = get_read_write_access_with_stmts(
      loop->stmts, loop->nstmts, &num_read_write_data[l], &num_stmts_per_acc);

    num_data[l] = num_write_data[l];

    op_trans_stmts[l] = 
        (Stmt ***)malloc(num_read_data[l] * sizeof(Stmt **));
    res_trans_stmts[l] = 
        (Stmt ***)malloc(num_write_data[l] * sizeof(Stmt **));
    compute_stmts[l] = 
        (Stmt ***)malloc(1 *sizeof(Stmt **));

    Stmt *anchor_stmt = get_new_anchor_stmt(loop->stmts, loop->nstmts);

    for (i = 0; i < num_read_data[l]; i++) {        
      op_trans_stmts[l][i] = psa_gen_op_read_in_code(
          racc_stmts[i], num_stmts_per_racc[i], prog, anchor_stmt,
          array_part_level[l], space_level[l], time_level[l], l, vsa);        
    }

    for (i = 0; i < num_write_data[l]; i++) {
      res_trans_stmts[l][i] = psa_gen_res_write_out_code(
          wacc_stmts[i], num_stmts_per_wacc[i], prog, anchor_stmt,
          array_part_level[l], space_level[l], time_level[l], l, vsa);
    }

    compute_stmts[l][0] = psa_gen_compute_code(
      // op_trans_stmts[l], res_trans_stmts[l],
      racc_stmts, num_stmts_per_racc, num_read_data[l],
      wacc_stmts, num_stmts_per_wacc, num_write_data[l],
      prog, anchor_stmt,
      array_part_level[l], space_level[l], time_level[l], l, vsa);

  }
    
  free(num_write_data);
  free(num_read_data);
  free(num_data);
  free(array_part_level);
  free(space_level);
  free(time_level);  
}

Stmt **psa_gen_op_read_in_code(
  struct stmt_access_pair **racc_stmts, int num_accs,
  PlutoProg *prog, Stmt *anchor_stmt,
  int array_part_level, int space_level, int time_level,
  int loop_num, VSA *vsa
) {
  int i, acc_nrows;
  acc_nrows = racc_stmts[0]->acc->mat->nrows;
  char *acc_name = racc_stmts[0]->acc->name;

  PlutoConstraints *read_in = NULL;
  int copy_level;
  copy_level = racc_stmts[0]->stmt->dim;  
  read_in = compute_read_in(racc_stmts[0], copy_level, prog);

  char *op_buf_size =
      get_parametric_bounding_box(read_in, copy_level, acc_nrows, 
                                  prog->npar, (const char **)prog->params);

  char *op_buf_decl =
      get_parametric_bounding_box_decl(read_in, copy_level, acc_nrows, 
                                  prog->npar, (const char **)prog->params);                                  
  
  char *op_transfer_stmt_text = "";
  char *op_transfer_module_name = "op_transfer";

  char *l_buf_name, *fifo_name;
  char *access_function;

  l_buf_name = concat("l_buf_", acc_name);
  fifo_name = concat("fifo_", acc_name);
  access_function = "";
  for (i = copy_level; i < copy_level + acc_nrows; i++) {
    access_function = concat(access_function, "[");
    char iter_tmp[100];
    sprintf(iter_tmp, "d%d", i + 1);
    access_function = concat(access_function, iter_tmp);
    access_function = concat(access_function, "]");
  }

  op_transfer_stmt_text = concat(op_transfer_stmt_text, l_buf_name);
  op_transfer_stmt_text = concat(op_transfer_stmt_text, access_function);
  op_transfer_stmt_text = concat(op_transfer_stmt_text, " = ");
  op_transfer_stmt_text = concat(op_transfer_stmt_text, fifo_name);
  op_transfer_stmt_text = concat(op_transfer_stmt_text, ".read()");

  generate_scanner_code(
    vsa, prog, read_in, copy_level,
    acc_name, acc_nrows,
    // op_buf_size,
    op_buf_decl,
    op_transfer_stmt_text,
    op_transfer_module_name
  );

  Stmt *op_trans_stmt = NULL;

  Stmt **op_trans_stmts = 
    (Stmt **)malloc(1 * sizeof(Stmt *));

  op_trans_stmts[0] = op_trans_stmt;

  return op_trans_stmts;
}

Stmt **psa_gen_res_write_out_code(
  struct stmt_access_pair **wacc_stmts, int num_accs,
  PlutoProg *prog, Stmt *anchor_stmt,
  int array_part_level, int space_level, int time_level,
  int loop_num, VSA *vsa
) {
  int i, acc_nrows;
  acc_nrows = wacc_stmts[0]->acc->mat->nrows;
  char *acc_name = wacc_stmts[0]->acc->name;

  int copy_level;
  copy_level = time_level;

  PlutoConstraints *write_out = NULL;
  write_out = compute_write_out(wacc_stmts[0], copy_level, prog);

  char *buf_size = 
      get_parametric_bounding_box(write_out, copy_level, acc_nrows, 
                                  prog->npar, (const char **)prog->params);

  char *buf_decl = 
      get_parametric_bounding_box_decl(write_out, copy_level, acc_nrows, 
                                  prog->npar, (const char **)prog->params);                                  

  char *l_buf_name, *fifo_name;
  char *l_count_name;

  char *res_transfer_module_name = "res_transfer";
  char *res_transfer_stmt_text = "";

  l_buf_name = concat("l_buf_", acc_name);
  l_count_name = concat("l_count_", acc_name);
  fifo_name = concat("fifo_", acc_name);
  res_transfer_stmt_text = concat(res_transfer_stmt_text, l_buf_name);
  res_transfer_stmt_text = concat(res_transfer_stmt_text, "[");
  res_transfer_stmt_text = concat(res_transfer_stmt_text, l_count_name);
  res_transfer_stmt_text = concat(res_transfer_stmt_text, "++] = ");
  res_transfer_stmt_text = concat(res_transfer_stmt_text, fifo_name);
  res_transfer_stmt_text = concat(res_transfer_stmt_text, ".read()");

  generate_scanner_code(
    vsa, prog, write_out, copy_level,
    acc_name, acc_nrows,
    // buf_size, 
    buf_decl, 
    res_transfer_stmt_text, res_transfer_module_name
  );

  Stmt *res_trans_stmt = NULL;

  Stmt **res_trans_stmts = 
    (Stmt **)malloc(1 * sizeof(Stmt *));

  res_trans_stmts[0] = res_trans_stmt;

  return res_trans_stmts;  
}

/*
 * Generate the op_transfer, compute, and res_transfer statement
 * seprate them and generate one whole new program
 */
Stmt **psa_gen_compute_code(
  // Stmt ***op_trans_stmts, Stmt ***res_trans_stmts,
  struct stmt_access_pair ***racc_stmts, int *num_raccs, int num_read_data,
  struct stmt_access_pair ***wacc_stmts, int *num_waccs, int num_write_data,
  PlutoProg *prog, Stmt *anchor_stmt,
  int array_part_level, int space_level, int time_level,
  int loop_num, VSA *vsa
) {
  char base_name[1024];
  strcpy(base_name, "");
  sprintf(base_name + strlen(base_name), "module_code/compute_code");

  char file_name[1024];
  strcpy(file_name, base_name);
  sprintf(file_name + strlen(file_name), ".c");

  char cloog_file_name[1024];
  strcpy(cloog_file_name, base_name);
  sprintf(cloog_file_name + strlen(cloog_file_name), ".cloog");

  FILE *fp = fopen(file_name, "w");
  assert(fp != NULL);

  //PlutoProg *new_prog = pluto_prog_dup(prog);
  Stmt **op_transfer_stmts = (Stmt **)malloc(vsa->op_num * sizeof(Stmt *));
  Stmt **res_transfer_stmts = (Stmt **)malloc(vsa->res_num * sizeof(Stmt *));    

  int i, j;
  int op_count = 0;
  int res_count = 0;

  for (i = 0; i < num_read_data; i++) {
    int acc_nrows = racc_stmts[i][0]->acc->mat->nrows;
    char *acc_name = racc_stmts[i][0]->acc->name;

    for (j = 0; j < vsa->op_num; j++) {
      if (!strcmp(vsa->op_names[j], acc_name)) {
        break;
      }
    }    

    if (j == vsa->op_num)
      continue;

    PlutoConstraints *read_in = NULL;
    int copy_level = racc_stmts[i][0]->stmt->dim;
    
    read_in = compute_read_in(racc_stmts[i][0], copy_level, prog);

    char *buf_size = 
      get_parametric_bounding_box(read_in, copy_level, acc_nrows, 
                                  prog->npar, (const char **)prog->params);   

    char *buf_decl = 
      get_parametric_bounding_box_decl(read_in, copy_level, acc_nrows, 
                                  prog->npar, (const char **)prog->params);                                       
                              
    fprintf(fp, "data_t_%s l_buf_%s%s;\n", acc_name, acc_name, buf_decl);

    char *stmt_text = "";
    char *l_buf_name, *fifo_name;
    char *access_function;

    l_buf_name = concat("l_buf_", acc_name);
    fifo_name = concat("fifo_", acc_name);
    access_function = "";
    for (j = copy_level; j < copy_level + acc_nrows; j++) {
      access_function = concat(access_function, "[");
      char iter_tmp[100];
      sprintf(iter_tmp, "d%d", j + 1);
      access_function = concat(access_function, iter_tmp);
      access_function = concat(access_function, "]");
    }    

    stmt_text = concat(stmt_text, l_buf_name);
    stmt_text = concat(stmt_text, access_function);
    stmt_text = concat(stmt_text, " = ");
    stmt_text = concat(stmt_text, fifo_name);
    stmt_text = concat(stmt_text, ".read()");

    pluto_constraints_project_out_isl_single(read_in, copy_level, acc_nrows);

    // create the statement
    Stmt *op_transfer_stmt = psa_create_helper_stmt(
          anchor_stmt, copy_level, stmt_text,
          STMT_UNKNOWN, PSA_STMT_UNKNOWN);

    pluto_constraints_intersect(op_transfer_stmt->domain, read_in);

    op_transfer_stmts[op_count++] = op_transfer_stmt;

    free(stmt_text);
    free(buf_size);
  }

  for (i = 0; i < num_write_data; i++) {
    int j;
    int acc_nrows = wacc_stmts[i][0]->acc->mat->nrows;
    char *acc_name = wacc_stmts[i][0]->acc->name;

    PlutoConstraints *write_out = NULL;
    //int copy_level = wacc_stmts[i][0]->stmt->dim;    
    int copy_level = time_level;

// #ifdef JIE_DEBUG
//     fprintf(stdout, "[Debug] psa_vsa_pe\n");
// #endif

    write_out = compute_write_out(wacc_stmts[i][0], copy_level, prog);

    char *buf_size = 
      get_parametric_bounding_box(write_out, copy_level, acc_nrows, 
                                  prog->npar, (const char **)prog->params);      

    char *buf_decl = 
      get_parametric_bounding_box_decl(write_out, copy_level, acc_nrows, 
                                  prog->npar, (const char **)prog->params);  

    fprintf(fp, "data_t_%s l_buf_%s%s;\n", acc_name, acc_name, buf_decl);

// #ifdef JIE_DEBUG
//     pluto_constraints_pretty_print(stdout, write_out);
// #endif

    pluto_constraints_free(write_out);

    copy_level = wacc_stmts[i][0]->stmt->dim;    
    write_out = compute_write_out(wacc_stmts[i][0], copy_level, prog);

    char *stmt_text = "";
    char *l_buf_name, *fifo_name;
    char *access_function;

    l_buf_name = concat("l_buf_", acc_name);
    fifo_name = concat("fifo_", acc_name);
    access_function = "";
    for (j = copy_level; j < copy_level + acc_nrows; j++) {
      access_function = concat(access_function, "[");
      char iter_tmp[100];
      sprintf(iter_tmp, "d%d", j + 1);
      access_function = concat(access_function, iter_tmp);
      access_function = concat(access_function, "]");
    }    
    
    stmt_text = concat(stmt_text, fifo_name);
    stmt_text = concat(stmt_text, ".write(");
    stmt_text = concat(stmt_text, l_buf_name);
    stmt_text = concat(stmt_text, access_function);
    stmt_text = concat(stmt_text, ")");    

    // char *module_name = "tmp_func";
    // generate_scanner_code(
    //   vsa, prog, write_out, copy_level,
    //   acc_name, acc_nrows,
    //   buf_size, stmt_text, module_name
    // );

    pluto_constraints_project_out_isl_single(write_out, copy_level, acc_nrows);

// #ifdef JIE_DEBUG
//     pluto_constraints_pretty_print(stdout, write_out);
// #endif

    // create the statement
    Stmt *res_transfer_stmt = psa_create_helper_stmt(
          anchor_stmt, copy_level, stmt_text,
          STMT_UNKNOWN, PSA_STMT_UNKNOWN);    

    pluto_constraints_intersect(res_transfer_stmt->domain, write_out);

    res_transfer_stmts[res_count++] = res_transfer_stmt;

    free(stmt_text);    
    free(buf_size);
  }

  int sep_stmts_size = vsa->op_num + vsa->res_num;
  Stmt **sep_stmts = (Stmt **)malloc(sep_stmts_size * sizeof(Stmt *));

  int count = 0;
  for (i = 0; i < vsa->op_num; i++) {
    pluto_add_given_stmt(prog, op_transfer_stmts[i]);
    sep_stmts[count++] = op_transfer_stmts[i];
  }
  for (i = 0; i < vsa->res_num; i++) {
    pluto_add_given_stmt(prog, res_transfer_stmts[i]);
    sep_stmts[count++] = res_transfer_stmts[i];
  }

  // seperate these stmts
  pluto_separate_stmts(prog, sep_stmts, count, anchor_stmt->dim, 0);

  // print out
  FILE *cloogfp = fopen(cloog_file_name, "w+");
  assert(cloogfp != NULL);

  pluto_gen_cloog_file(cloogfp, prog);
  rewind(cloogfp);
  fflush(cloogfp);

  psa_generate_declarations(prog, fp);
  //pluto_gen_cloog_code(prog, 1, prog->num_hyperplanes, cloogfp, fp);
  pluto_gen_cloog_code(prog, prog->num_hyperplanes, 1, cloogfp, fp);

  // pluto_prog_free(prog);

  fclose(fp);
  fclose(cloogfp);
}
