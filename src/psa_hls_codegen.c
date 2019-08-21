/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 *
 * Author: Jie Wang
 */
#include "pluto.h"
#include "psa_vsa.h"
#include "psa_hls_codegen.h"

void psa_intel_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name) {
  /* PE Code */
  PlutoProg *new_prog = NULL;
  new_prog = pluto_prog_dup(prog);
  psa_PE_codegen(new_prog, vsa, src_file_name, 0);
  pluto_prog_free(new_prog);

  /* Data Transfer Code */
  new_prog = pluto_prog_dup(prog);
  psa_data_trans_codegen(new_prog, vsa, src_file_name, 0);
  pluto_prog_free(new_prog);

  /* Top Kernel Code */
  new_prog = pluto_prog_dup(prog);
  psa_top_kernel_codegen(new_prog, vsa, src_file_name, 0);
  pluto_prog_free(new_prog);

  /* Header Code */
  new_prog = pluto_prog_dup(prog);
  psa_header_codegen(new_prog, vsa, src_file_name, 0);
  pluto_prog_free(new_prog);
}

void psa_xilinx_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name) {
  /* PE Code */  
  PlutoProg *new_prog = NULL;
  new_prog = pluto_prog_dup(prog);
  psa_PE_codegen(new_prog, vsa, src_file_name, 1);
  pluto_prog_free(new_prog);

  /* Data Transfer Code */
  new_prog = pluto_prog_dup(prog);
  psa_data_trans_codegen(new_prog, vsa, src_file_name, 1);
  pluto_prog_free(new_prog);

  /* Top Kernel Code */
  new_prog = pluto_prog_dup(prog);
  psa_top_kernel_codegen(new_prog, vsa, src_file_name, 1);
  pluto_prog_free(new_prog);

  /* Header Code */
  new_prog = pluto_prog_dup(prog);
  psa_header_codegen(new_prog, vsa, src_file_name, 1);
  pluto_prog_free(new_prog); 
}

void psa_data_trans_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name, int target) {

}

void psa_top_kernel_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name, int target) {

}

/* 
 * This function generates PE function code.
 * It will call different APIs depedending on the target platform.
 * 0 - Intel OpenCL
 * 1 - Xilinx HLS
 */
void psa_PE_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name, int target) {
  // Add the data transfer statements into the program
  int s = 0;
  while(s < prog->nstmts) {  
    Stmt *stmt = prog->stmts[s];
    Stmt **before_stmts = NULL;
    Stmt **after_stmts = NULL;
    int nbefore_stmts = 0;
    int nafter_stmts = 0;

    for (int acc_id = 0; acc_id < stmt->nreads; acc_id++) {
      PlutoAccess *acc = stmt->reads[acc_id];
      // Look up the acc_io_map
      // Case 0: If the acc belongs to RAR with trans_bound - IN_BOUND and trans_type - EMBEDDED
      // We will add one read-in stmt (regX = fifoX_in.read()) and 
      // one write-out stmt (fifoX_out.write(regX))
      // Case 1: If the acc belongs to RAR with trans_bound - IN_BOUND and trans_type - SEPARATE
      // We will add one read-in stmt (regX = fifoX_in.read())
      // Case 2: If the acc belongs to RAW with trans_bound - IN_BOUND and trans_type - EMBEDDED
      // We will add one read-in stmt (regX = fifoX_in.read())
      // Case 3: If the acc belongs to RAW with trans_bound - IN_BOUND and trans_type - SEPARATE
      // We will add one read-in stmt (regX = fifoX_in.read())
      // Case 4: If the acc belongs to RAW with trans_bound - NO_BOUND
      // Do nothing
      int io_map_id = 0;
      while (io_map_id < vsa->io_map_num_entries) {
        struct stmt_access_io_pair *stmt_access_io = vsa->io_map[io_map_id];
        if (stmt_access_io->acc->sym_id == acc->sym_id) {
          int case_id;
          if (IS_RAR(stmt_access_io->dep->type)) {
            if (stmt_access_io->L1_trans_bound == IN_BOUND && stmt_access_io->L1_trans_type == EMBEDDED) {
              case_id = 0;
            } else if (stmt_access_io->L1_trans_bound == IN_BOUND && stmt_access_io->L1_trans_type == SEPARATE) {
              case_id = 1;
            }
          } else if (IS_RAW(stmt_access_io->dep->type)) {
            if (stmt_access_io->L1_trans_bound == IN_BOUND && stmt_access_io->L1_trans_type == EMBEDDED) {
              case_id = 2;              
            } else if (stmt_access_io->L1_trans_bound == IN_BOUND && stmt_access_io->L1_trans_type == SEPARATE) {
              case_id = 3;
            } else if (stmt_access_io->L1_trans_bound == NO_BOUND) {
              case_id = 4;
            }
          }

          if (case_id == 0) {
            // case 0          
            // insert stmt: regX = fifoX_in.read()
            // stmt_text
            char *stmt_text1 = "";
            char reg_text[20], fifo_text[30];
            sprintf(reg_text, "reg%s_%d", acc->name, acc->sym_id);
            if (target == INTEL_TARGET) {
              sprintf(fifo_text, "fifo%s_%d_PE", acc->name, stmt_access_io->trans_set);
              stmt_text1 = concat(stmt_text1, reg_text);
              stmt_text1 = concat(stmt_text1, " = read_channel_intel(");
              stmt_text1 = concat(stmt_text1, fifo_text);
              stmt_text1 = concat(stmt_text1, "[row][col]);");
            } else if (target == XILINX_TARGET) {
              sprintf(fifo_text, "fifo%s_%d_in", acc->name, stmt_access_io->trans_set);
              stmt_text1 = concat(stmt_text1, reg_text);
              stmt_text1 = concat(stmt_text1, " = ");
              stmt_text1 = concat(stmt_text1, fifo_text);
              stmt_text1 = concat(stmt_text1, ".read();");
            } 
            
            // stmt_domain, stmt_trans, stmt_iterators
            PlutoConstraints *new_domain1 = stmt_access_io->domain;
            if (!new_domain1 || pluto_constraints_is_empty(new_domain1)) {
              io_map_id++;
              continue;
            }

            // create the statement
            Stmt *new_stmt1 = psa_create_helper_stmt(stmt->trans->nrows, new_domain1, stmt_text1, STMT_UNKNOWN, PSA_L1_TRANS_IN);

            // add the statement
            nbefore_stmts++;
            before_stmts = realloc(before_stmts, nbefore_stmts * sizeof(Stmt *));
            before_stmts[nbefore_stmts - 1] = new_stmt1;
            
            // free up memory
            free(stmt_text1);
            
            // insert stmt: fifoX_out.write(regX);
            // stmt_text
            char *stmt_text2 = "";
            if (target == INTEL_TARGET) {
              sprintf(fifo_text, "fifo%s_%d_PE", acc->name, stmt_access_io->trans_set);
              stmt_text2 = concat(stmt_text2, "write_channel_intel(");                
              stmt_text2 = concat(stmt_text2, fifo_text);
              char *tmp_str = psa_gen_PE_channel_acc(stmt_access_io->L1_trans_dir);
              stmt_text2 = concat(stmt_text2, tmp_str);            
              stmt_text2 = concat(stmt_text2, ", ");
              stmt_text2 = concat(stmt_text2, reg_text);
              stmt_text2 = concat(stmt_text2, ");");                
              free(tmp_str);
            } else if (target == XILINX_TARGET) {
              sprintf(fifo_text, "fifo%s_%d_out", acc->name, stmt_access_io->trans_set);
              stmt_text2 = concat(stmt_text2, fifo_text);
              stmt_text2 = concat(stmt_text2, ".write(");
              stmt_text2 = concat(stmt_text2, reg_text);
              stmt_text2 = concat(stmt_text2, ");");                
            }

            // stmt_domain, stmt_trans, stmt_iterators
            PlutoConstraints *new_domain2 = stmt_access_io->domain;
            if (!new_domain2 || pluto_constraints_is_empty(new_domain2)) {
              io_map_id++;
              continue;
            }

            // create the statement            
            Stmt *new_stmt2 = psa_create_helper_stmt(stmt->trans->nrows, new_domain2, stmt_text2, STMT_UNKNOWN, PSA_L1_TRANS_OUT);

            // add the statement
            nafter_stmts++;
            after_stmts = realloc(after_stmts, nafter_stmts * sizeof(Stmt *));
            after_stmts[nafter_stmts - 1] = new_stmt2;

            // free up memory
            free(stmt_text2);            
          } else if (case_id < 4) {
            // case 1-3
            // insert stmt: regX = fifoX_in.read()
            char *stmt_text1 = "";
            char reg_text[20], fifo_text[30];
            sprintf(reg_text, "reg%s_%d", acc->name, acc->sym_id);
            if (target == INTEL_TARGET) {
              sprintf(fifo_text, "fifo%s_%d_PE", acc->name, stmt_access_io->trans_set);
              stmt_text1 = concat(stmt_text1, reg_text);
              stmt_text1 = concat(stmt_text1, " = read_channel_intel(");
              stmt_text1 = concat(stmt_text1, fifo_text);
              stmt_text1 = concat(stmt_text1, "[row][col]);");                
            } else if (target == XILINX_TARGET) {
              sprintf(fifo_text, "fifo%s_%d_in", acc->name, stmt_access_io->trans_set);
              stmt_text1 = concat(stmt_text1, reg_text);
              stmt_text1 = concat(stmt_text1, " = ");
              stmt_text1 = concat(stmt_text1, fifo_text);
              stmt_text1 = concat(stmt_text1, ".read();");
            } 
            
            // stmt_domain, stmt_trans, stmt_iterators
            PlutoConstraints *new_domain1 = stmt_access_io->domain;
            if (!new_domain1 || pluto_constraints_is_empty(new_domain1)) {
              io_map_id++;
              continue;
            }

            Stmt *new_stmt1 = psa_create_helper_stmt(stmt->trans->nrows, new_domain1, stmt_text1, STMT_UNKNOWN, PSA_L1_TRANS_IN);

            // add the statemwent
            nbefore_stmts++;
            before_stmts = realloc(after_stmts, nbefore_stmts * sizeof(Stmt *));
            before_stmts[nbefore_stmts - 1] = new_stmt1;
            
            // free up memory
            free(stmt_text1);
          } 
        } else {
          // Do nothing
        }

        io_map_id++;
      }
    }
    for (int acc_id = 0; acc_id < stmt->nwrites; acc_id++) {
      PlutoAccess *acc = stmt->writes[acc_id];
      // Look up the acc_io_map
      // Case 0: If the acc belongs to RAW with trans_bound - OUT_BOUND and trans_type - EMBEDDED
      // We will add one write-out stmt (fifoX_out.write(regX));
      // Case 1: If the acc belongs to RAW with trans_bound - OUT_BOUND and trans_type - SEPARATE
      // We will add one write-out stmt (fifoX_out.write(X[][]));
      int io_map_id = 0;
      while (io_map_id < vsa->io_map_num_entries) {
        struct stmt_access_io_pair *stmt_access_io = vsa->io_map[io_map_id];
        if (stmt_access_io->acc->sym_id == acc->sym_id) {
          int case_id;
          if (IS_RAW(stmt_access_io->dep->type)) {
            if (stmt_access_io->L1_trans_bound == OUT_BOUND && stmt_access_io->L1_trans_type == EMBEDDED) {
              case_id = 0;
            } else if (stmt_access_io->L1_trans_bound == OUT_BOUND && stmt_access_io->L1_trans_type == SEPARATE) {
              case_id = 1;
            }
          }
          
          if (case_id == 0) {
            // case 0
            // insert stmt: fifoX_out.write(regX)
            char *stmt_text1 = "";
            char reg_text[20], fifo_text[30];
            sprintf(reg_text, "reg%s_%d", acc->name, acc->sym_id);
            if (target == INTEL_TARGET) {
              sprintf(fifo_text, "fifo%s_%d_PE", acc->name, stmt_access_io->trans_set);
              stmt_text1 = concat(stmt_text1, "write_channel_intel(");
              stmt_text1 = concat(stmt_text1, fifo_text);
              char *tmp_str = psa_gen_PE_channel_acc(stmt_access_io->L1_trans_dir);
              stmt_text1 = concat(stmt_text1, tmp_str);
              stmt_text1 = concat(stmt_text1, ", ");
              stmt_text1 = concat(stmt_text1, reg_text);
              stmt_text1 = concat(stmt_text1, ");");
              free(tmp_str);
            } else if (target == XILINX_TARGET) {
              sprintf(fifo_text, "fifo%s_%d_out", acc->name, stmt_access_io->trans_set);
              stmt_text1 = concat(stmt_text1, fifo_text);
              stmt_text1 = concat(stmt_text1, ".write(");
              stmt_text1 = concat(stmt_text1, reg_text);
              stmt_text1 = concat(stmt_text1, ");");
            }

            // stmt_domain
            PlutoConstraints *new_domain1 = stmt_access_io->domain;
            if (!new_domain1 || pluto_constraints_is_empty(new_domain1)) {
              io_map_id++;
              continue;
            }

            Stmt *new_stmt1 = psa_create_helper_stmt(stmt->trans->nrows, new_domain1, stmt_text1, STMT_UNKNOWN, PSA_L1_TRANS_OUT);

            // add the statement
            nafter_stmts++;
            after_stmts = realloc(after_stmts, nafter_stmts * sizeof(Stmt *));
            after_stmts[nafter_stmts - 1] = new_stmt1;

            // free up memory
            free(stmt_text1);
          } else if (case_id == 1) {
            // case 1
            // insert stmt: fifoX_out.write(X[][])
            char *stmt_text1 = "";
            char reg_text[20], fifo_text[30];
            sprintf(reg_text, "reg%s_%d", acc->name, acc->sym_id);
            if (target == INTEL_TARGET) {
              sprintf(fifo_text, "fifo%s_%d_PE", acc->name, stmt_access_io->trans_set);
              stmt_text1 = concat(stmt_text1, "write_channel_intel(");
              stmt_text1 = concat(stmt_text1, fifo_text);
              char *tmp_str = psa_gen_PE_channel_acc(stmt_access_io->L1_trans_dir);
              stmt_text1 = concat(stmt_text1, tmp_str);
              stmt_text1 = concat(stmt_text1, ", ");
              free(tmp_str);

              char **names = (char **)malloc((stmt->trans->nrows + prog->npar) * sizeof(char *));              
              for (int k = 0; k < stmt->trans->nrows; k++) {
                char iter_str[20];
                sprintf(iter_str, "d%d", k + 1);
                names[k] = strdup(iter_str);
              }
              for (int k = stmt->trans->nrows; k < stmt->trans->nrows + prog->npar; k++) {
                names[k] = strdup(prog->params[k - stmt->trans->nrows]);
              }
              tmp_str = psa_gen_new_acc_str(stmt, acc, names);

              stmt_text1 = concat(stmt_text1, tmp_str);
              stmt_text1 = concat(stmt_text1, ");");
              free(tmp_str);
              for (int k = 0; k < stmt->trans->nrows + prog->npar; k++) {
                free(names[k]);
              }
              free(names);
            } else if (target == XILINX_TARGET) {
              sprintf(fifo_text, "fifo%s_%d_out", acc->name, stmt_access_io->trans_set);
              char **names = (char **)malloc((stmt->trans->nrows + prog->npar) * sizeof(char *));              
              for (int k = 0; k < stmt->trans->nrows; k++) {
                char iter_str[20];
                sprintf(iter_str, "d%d", k + 1);
                names[k] = strdup(iter_str);
              }
              for (int k = stmt->trans->nrows; k < stmt->trans->nrows + prog->npar; k++) {
                names[k] = strdup(prog->params[k - stmt->trans->nrows]);
              }
              char *tmp_str = psa_gen_new_acc_str(stmt, acc, names);

              stmt_text1 = concat(stmt_text1, fifo_text);
              stmt_text1 = concat(stmt_text1, ".write(");          
              stmt_text1 = concat(stmt_text1, tmp_str);
              stmt_text1 = concat(stmt_text1, ");");              
              free(tmp_str);
              for (int k = 0; k < stmt->trans->nrows + prog->npar; k++) {
                free(names[k]);                
              }
              free(names);
            }

            // stmt_domain
            PlutoConstraints *new_domain1 = stmt_access_io->domain;
            if (!new_domain1 || pluto_constraints_is_empty(new_domain1)) {
              io_map_id++;
              continue;
            }

            Stmt *new_stmt1 = psa_create_helper_stmt(stmt->trans->nrows, new_domain1, stmt_text1, STMT_UNKNOWN, PSA_L1_TRANS_OUT);

            // add the statement
            nafter_stmts++;
            after_stmts = realloc(after_stmts, nafter_stmts * sizeof(Stmt *));
            after_stmts[nafter_stmts - 1] = new_stmt1;

            // free up memory
            free(stmt_text1);           
          }
        }
        io_map_id++;
      }
    }

    // insert the stmts and separaet them at the same time
    int nsep_stmts = 0;
    Stmt **sep_stmts = (Stmt **)malloc((nbefore_stmts + 1 + nafter_stmts) * sizeof(Stmt *));
    for (int i = 0; i < nbefore_stmts; i++) {
      pluto_add_given_stmt(prog, before_stmts[i]);
      sep_stmts[nsep_stmts++] = before_stmts[i];
    }
    sep_stmts[nsep_stmts++] = stmt;
    for (int i = 0; i < nafter_stmts; i++) {
      pluto_add_given_stmt(prog, after_stmts[i]);
      sep_stmts[nsep_stmts++] = after_stmts[i];
    }
    
    if (nsep_stmts > 1)
      pluto_separate_stmts(prog, sep_stmts, nsep_stmts, stmt->trans->nrows, 0);

    // increase the stmt counter
    s += nsep_stmts;
  }

  /* Print out the program */
  pluto_transformations_pretty_print(prog);
  // generate AST
  // entering AST and modifying the local index  
  // generate the code and plug into the function code
  FILE *cloog_fp;
  cloog_fp = fopen(".codegen.PE.cloog", "w+");
  if (!cloog_fp) {
    fprintf(stderr, "[PSA] Can't open .cloog file: '.codegen.PE.cloog'\n");
    return;
  }
  pluto_gen_cloog_file(cloog_fp, prog);
  rewind(cloog_fp);
  
  FILE *out_fp;
  out_fp = fopen(".codegen.PE.c", "w");
  if (!out_fp) {
    fprintf(stderr, "[PSA] Can't open file '.codegen.PE.c' for writing\n");
    return;
  }
  
  pluto_gen_cloog_code(prog, prog->num_hyperplanes, 1, cloog_fp, out_fp);

  fclose(cloog_fp);
  fclose(out_fp);

  /* Get basename */
  char *basec, *bname;
  basec = strdup(src_file_name);
  bname = basename(basec);

  char *dump_file_name;
  if (target == 0) {
    dump_file_name = malloc(strlen(bname) - 2 + strlen(".") + strlen("PE.intel") + strlen(".cl") + 1);
    strncpy(dump_file_name, bname, strlen(bname) - 2);
    dump_file_name[strlen(bname) - 2] = '\0';
    strcat(dump_file_name, ".");
    strcat(dump_file_name, "PE.intel");
    strcat(dump_file_name, ".cl");
  } else if (target == 1) {
    dump_file_name = malloc(strlen(bname) - 2 + strlen(".") + strlen("PE.xilinx") + strlen(".c") + 1);
    strncpy(dump_file_name, bname, strlen(bname) - 2);
    dump_file_name[strlen(bname) - 2] = '\0';
    strcat(dump_file_name, ".");
    strcat(dump_file_name, "PE.xilinx");
    strcat(dump_file_name, ".c");   
  }

  FILE *fp = fopen(dump_file_name, "w");
  if (!fp) {
    fprintf(stdout, "[PSA] Error! File %s can't be opened!\n", dump_file_name);
    return;
  }
  free(basec);
  free(dump_file_name);

  // print stmt macros 
  for (int i = 0; i < prog->nstmts; i++) {
    gen_stmt_macro(prog->stmts[i], fp);
  }
  fprintf(fp, "\n");

  // TODO: Handle irregular space domain in the future
  if (target == INTEL_TARGET) {
    fprintf(fp, "__attribute__((max_global_work_dim(0)))\n");
    fprintf(fp, "__attribute__((autorun))\n");
    fprintf(fp, "__attribtue__((num_compute_units(SYS_ARRAY_NUM_ROWS, SYS_ARRAY_NUM_COLS)))\n");
    fprintf(fp, "__kernel void PE_kernel()\n");
  } else if (target == XILINX_TARGET) {
    // TODO
    fprintf(fp, "void PE_kernel(\n");
    fprintf(fp, "  int row,\n");
    fprintf(fp, "  int col\n");
    fprintf(fp, ")\n");
  }

  fprintf(fp, "{\n");
  
  if (target == INTEL_TARGET) {
    fprintf(fp, "  const int row = get_compute_id(0);\n");
    fprintf(fp, "  const int col = get_compute_id(1);\n");
  } else if (target == XILINX_TARGET){     
  }
  
  // print the iterator decls
  if (prog->num_hyperplanes >= 1) {
    fprintf(fp, "  int ");
    for (int i = 0; i < prog->num_hyperplanes; i++) {
      if (i != 0)
        fprintf(fp, ", ");
      fprintf(fp, "t%d", i + 1);      
    }
    fprintf(fp, ";\n\n");
  }

  // print the array decls
  // TODO

  // print the function body
  // read in the generate c code print it out to the current PE function code  
  FILE *in_fp;
  in_fp = fopen(".codegen.PE.c", "r");
  if (!in_fp) {
    fprintf(stderr, "[PSA] Can't open file '.codegen.PE.c' for reading\n");
    return;
  }
  
  char *line = NULL;
  int len = 0;
  int read;
  while((read = getline(&line, &len, in_fp) != -1)) {
    fprintf(fp, "  %s", line);
  }
  fprintf(fp, "\n");

  fprintf(fp, "}\n");

  fclose(in_fp);
  fclose(fp);
}

void psa_header_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name, int target) {
  /* Get basename */
  char *basec, *bname;
  basec = strdup(src_file_name);
  bname = basename(basec);

  char *dump_file_name;
  if (target == 0) {
    dump_file_name = malloc(strlen(bname) - 2 + strlen(".") + strlen("intel") + strlen(".h") + 1);
    strncpy(dump_file_name, bname, strlen(bname) - 2);
    dump_file_name[strlen(bname) - 2] = '\0';
    strcat(dump_file_name, ".");
    strcat(dump_file_name, "intel");
    strcat(dump_file_name, ".h");
  } else if (target == 1) {
    dump_file_name = malloc(strlen(bname) - 2 + strlen(".") + strlen("xilinx") + strlen(".h") + 1);
    strncpy(dump_file_name, bname, strlen(bname) - 2);
    dump_file_name[strlen(bname) - 2] = '\0';
    strcat(dump_file_name, ".");
    strcat(dump_file_name, "xilinx");
    strcat(dump_file_name, ".h");
  }

  FILE *fp = fopen(dump_file_name, "w");
  if (!fp) {
    fprintf(stdout, "[PSA] Error! File %s can't be opened!\n", dump_file_name);
    return;
  }
  free(basec);
  free(dump_file_name);

  /* Macros */
  fprintf(fp, "#define SYS_ARRAY_NUM_ROWS %d\n", vsa->sa_rows);
  fprintf(fp, "#define SYS_ARRAY_NUM_COLS %d\n", vsa->sa_cols);

  fclose(fp); 
}
