/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 * 
 * Author: Jie Wang
 */
#include "pluto.h"
#include "psa_vsa.h"

/*
 * This function analyzes the read/write access info from the program and extracts
 * information about operands and results, including:
 * - op_names, res_names
 * - op_num, res_num
 * - op_dims, res_dims
 */
void vsa_op_res_extract(PlutoProg *prog, VSA *vsa) {
  int i, j;
  char **read_names = NULL;
  int *read_dims = NULL;
  int num_read_names = 0;
  char **write_names = NULL;
  int *write_dims = NULL;
  int num_write_names = 0;

  /* Add all read and write names from the program */
  for (i = 0; i < prog->nstmts; i++) {
    Stmt *stmt = prog->stmts[i];
    for (j = 0; j < stmt->nreads; j++) {
      PlutoAccess *read_acc = stmt->reads[j];
      if (read_acc->mat->nrows == 1) {
        int col;
        for (col = 0; col < read_acc->mat->ncols; col++) {
          if (read_acc->mat->val[0][col] != 0)
            break;          
        }
        if (col == read_acc->mat->ncols) {
          /* scalar */
          continue;
        }
      } 
      char *name = read_acc->name;
      int n;
      /* Check if the name has already been added. */
      for (n = 0; n < num_read_names; n++) {
        if (!strcmp(read_names[n], name))
          break;
      }
      if (n == num_read_names) {             
        num_read_names++;
        read_names = (char **)realloc(read_names, num_read_names * sizeof(char *));
        read_names[num_read_names - 1] = strdup(name);            
        read_dims = (int *)realloc(read_dims, num_read_names * sizeof(int));
        read_dims[num_read_names - 1] = read_acc->mat->nrows;
      }            
    }  

    for (j = 0; j < stmt->nwrites; j++) {
      PlutoAccess *write_acc = stmt->writes[j];
      char *name = write_acc->name;
      int n;
      /* Check if the name has already been added. */
      for (n = 0; n < num_write_names; n++) {
        if (!strcmp(write_names[n], name))
          break;          
      }
      if (n == num_write_names) {
        num_write_names++;
        write_names = (char **)realloc(write_names, num_write_names * sizeof(char *));
        write_names[num_write_names - 1] = strdup(name);
        write_dims = (int *)realloc(write_dims, num_write_names * sizeof(int));
        write_dims[num_write_names - 1] = write_acc->mat->nrows;
      }
    }
  }

  /* Operand array will only read accesses,
   * the rest wiil be the result array.
   * NOTE: Currently we assume each array comes with only one unique reference.
   */
  char **op_names = NULL;
  char **res_names = NULL;
  int op_num = 0;
  int res_num = 0;
  int *op_dims = NULL;
  int *res_dims = NULL;
  
  for (i = 0; i < num_read_names; i++) {
    char *r_name = read_names[i];
    for (j = 0; j < num_write_names; j++) {
      char *w_name = write_names[j];
      if (!strcmp(r_name, w_name)) 
        break;      
    }
    if (j == num_write_names) {
      /* check if the name already exists in op_names */
      int n;
      for (n = 0; n < op_num; n++) {
        if (!strcmp(r_name, op_names[n]))
          break;
      }
      if (n == op_num) {
        op_num++;
        op_names = (char **)realloc(op_names, op_num * sizeof(char *));
        op_names[op_num - 1] = strdup(r_name);
        op_dims = (int *)realloc(op_dims, op_num * sizeof(int));
        op_dims[op_num - 1] = read_dims[i];
      }
    }
  }

  /* All the rest array names that are not op_names will be inserted into res_names */
  for (i = 0; i < num_read_names; i++) {
    char *r_name = read_names[i];
    for (j = 0; j < op_num; j++) {
      if (!strcmp(r_name, op_names[j])) {
        break;
      }
    }
    if (j == op_num) {
      res_num++;
      res_names = (char **)realloc(res_names, res_num * sizeof(char *));
      res_names[res_num - 1] = strdup(r_name);
      res_dims = (int *)realloc(res_dims, res_num * sizeof(int));
      res_dims[res_num - 1] = read_dims[i];              
    }
  }
  for (i = 0; i < num_write_names; i++) {
    char *w_name = write_names[i];
    for (j = 0; j < res_num; j++) {
      if (!strcmp(w_name, res_names[j])) {
        break;
      }
    }
    if (j == res_num) {
      res_num++;
      res_names = (char **)realloc(res_names, res_num * sizeof(char *));
      res_names[res_num - 1] = strdup(w_name);
      res_dims = (int *)realloc(res_dims, res_num * sizeof(int));
      res_dims[res_num - 1] = write_dims[i];                      
    }
  }

  vsa->op_num = op_num;
  vsa->res_num = res_num;
  vsa->op_dims = op_dims;
  vsa->res_dims = res_dims;
  vsa->op_names = op_names;
  vsa->res_names = res_names;

  //free(read_names);
  //free(write_names);
  //free(read_dims);
  //free(write_names);
}

/*
 * Extract op_channel_dir and res_channel_dir from the prog.
 * Currently, we support op_channel_dir and res_channel_dir in:
 * R, Right
 * L, Left,
 * U, Up,
 * D, Down,
 * and combinations of these four above (e.g., RD, Right-down).
 * Examine the RAR dep for operands and the RAW dep for results. 
 */
void vsa_channel_dir_extract(PlutoProg *prog, VSA *vsa) {
  int i, j;

  char **op_names = vsa->op_names;
  char **res_names = vsa->res_names;

  char **op_channel_dirs = (char **)malloc(vsa->op_num * sizeof(char *));
  char **res_channel_dirs = (char **)malloc(vsa->res_num * sizeof(char *));

  for (i = 0; i < vsa->op_num; i++) {
    char *op_name = op_names[i];
    for (j = 0; j < prog->ndeps; j++) {
      Dep *dep = prog->deps[j];
      /* RAR dep for operands */
      if (IS_RAR(dep->type) && !strcmp(dep->src_acc->name, op_name)) {
        DepDis *space_dep_dis = (DepDis *)malloc(prog->array_dim * sizeof(DepDis));
        /* Collect dep dis components at space loops */
        int h;
        int space_dim = 0;
        for (h = 0; h < prog->num_hyperplanes; h++) {
          HyperplaneProperties hProps = prog->hProps[h];
          if (IS_PSA_SPACE_LOOP(hProps.psa_type)) {
            space_dep_dis[space_dim] = dep->disvec[h];
            space_dim++;
          }
        }

        /* Analyze the direction */
        if (prog->array_dim == 1) {
          if (space_dep_dis[0] == DEP_DIS_PLUS_ONE) {
            op_channel_dirs[i] = "R";
          } else if (space_dep_dis[0] == DEP_DIS_MINUS_ONE) {
            op_channel_dirs[i] = "L";
          } else if (space_dep_dis[0] == DEP_DIS_ZERO) {
            op_channel_dirs[i] = "D";
          } else {
            fprintf(stdout, "[PSA] Error! Wrong dependence component at space loops!\n");
          }
        } else if (prog->array_dim == 2) {
          if (space_dep_dis[0] == DEP_DIS_ZERO) {
            if (space_dep_dis[1] == DEP_DIS_PLUS_ONE) {              
              /* 0,1 */
              op_channel_dirs[i] = "R";
            } else if (space_dep_dis[i] == DEP_DIS_ZERO) {
              /* 0,0 */
              /* TODO: In place - will be decided in the I/O elimination */
              op_channel_dirs[i] = "I"; 
            } else if (space_dep_dis[i] == DEP_DIS_MINUS_ONE) {
              /* 0,-1 */
              op_channel_dirs[i] = "L";
            } else {
              fprintf(stdout, "[PSA] Error! Wrong dependence component at space loop!\n");
            }
          } else if (space_dep_dis[0] == DEP_DIS_PLUS_ONE) {
            if (space_dep_dis[1] == DEP_DIS_PLUS_ONE) {
              /* 1,1 */
              op_channel_dirs[i] = "DR";
            } else if (space_dep_dis[1] == DEP_DIS_ZERO) {
              /* 1,0 */
              op_channel_dirs[i] = "D";
            } else if (space_dep_dis[1] == DEP_DIS_MINUS_ONE) {
              /* 1,-1 */
              op_channel_dirs[i] = "DL";
            } else {
              fprintf(stdout, "[PSA] Error! Wrong dependence component at space loop!\n");
            }
          } else if (space_dep_dis[0] == DEP_DIS_MINUS_ONE) {
            if (space_dep_dis[1] == DEP_DIS_PLUS_ONE) {
              /* -1,1 */
              op_channel_dirs[i] = "UR";
            } else if (space_dep_dis[1] == DEP_DIS_ZERO) {
              /* -1,0 */
              op_channel_dirs[i] = "U";
            } else if (space_dep_dis[1] == DEP_DIS_MINUS_ONE) {
              /* -1,-1 */
              op_channel_dirs[i] = "UL";
            } else {
              fprintf(stdout, "[PSA] Error! Wrong dependence component at space loop!\n");
            }
          } else {
            fprintf(stdout, "[PSA] Error! Wrong dependence component at space loop!\n");
          }
        }

        free(space_dep_dis);
      }
    }
  }

  for (i = 0; i < vsa->res_num; i++) {
    char *res_name = res_names[i];
    for (j = 0; j < prog->ndeps; j++) {
      Dep *dep = prog->deps[j];
      /* RAW dep for results */
      if (IS_RAW(dep->type) && !strcmp(dep->src_acc->name, res_name)) {
        DepDis *space_dep_dis = (DepDis *)malloc(prog->array_dim * sizeof(DepDis));
        /* Collect dep dis components at space loop */
        int h;
        int space_dim = 0;
        for (h = 0; h < prog->num_hyperplanes; h++) {
          HyperplaneProperties hProps = prog->hProps[h];
          if (IS_PSA_SPACE_LOOP(hProps.psa_type)) {
            space_dep_dis[space_dim] = dep->disvec[h];
            space_dim++;
          }
        }

        /* Analyze the direction */
        if (prog->array_dim == 1) {
          if (space_dep_dis[0] == DEP_DIS_PLUS_ONE) {
            res_channel_dirs[i] = "R";
          } else if (space_dep_dis[0] == DEP_DIS_MINUS_ONE) {
            res_channel_dirs[i] = "L";
          } else if (space_dep_dis[0] == DEP_DIS_ZERO) {
            res_channel_dirs[i] = "D";
          } else {
            fprintf(stdout, "[PSA] Error! Wrong dependence component at space loops!\n");
          }
        } else if (prog->array_dim == 2) {
          if (space_dep_dis[0] == DEP_DIS_ZERO) {
            if (space_dep_dis[1] == DEP_DIS_PLUS_ONE) {              
              /* 0,1 */
              res_channel_dirs[i] = "R";
            } else if (space_dep_dis[i] == DEP_DIS_ZERO) {
              /* 0,0 */
              /* TODO: In place - will be decided in the I/O elimination */
              res_channel_dirs[i] = "I"; 
            } else if (space_dep_dis[i] == DEP_DIS_MINUS_ONE) {
              /* 0,-1 */
              res_channel_dirs[i] = "L";
            } else {
              fprintf(stdout, "[PSA] Error! Wrong dependence component at space loop!\n");
            }
          } else if (space_dep_dis[0] == DEP_DIS_PLUS_ONE) {
            if (space_dep_dis[1] == DEP_DIS_PLUS_ONE) {
              /* 1,1 */
              res_channel_dirs[i] = "DR";
            } else if (space_dep_dis[1] == DEP_DIS_ZERO) {
              /* 1,0 */
              res_channel_dirs[i] = "D";
            } else if (space_dep_dis[1] == DEP_DIS_MINUS_ONE) {
              /* 1,-1 */
              res_channel_dirs[i] = "DL";
            } else {
              fprintf(stdout, "[PSA] Error! Wrong dependence component at space loop!\n");
            }
          } else if (space_dep_dis[0] == DEP_DIS_MINUS_ONE) {
            if (space_dep_dis[1] == DEP_DIS_PLUS_ONE) {
              /* -1,1 */
              res_channel_dirs[i] = "UR";
            } else if (space_dep_dis[1] == DEP_DIS_ZERO) {
              /* -1,0 */
              res_channel_dirs[i] = "U";
            } else if (space_dep_dis[1] == DEP_DIS_MINUS_ONE) {
              /* -1,-1 */
              res_channel_dirs[i] = "UL";
            } else {
              fprintf(stdout, "[PSA] Error! Wrong dependence component at space loop!\n");
            }
          } else {
            fprintf(stdout, "[PSA] Error! Wrong dependence component at space loop!\n");
          }
        }

        free(space_dep_dis);
      }
    }
  }

  vsa->op_channel_dirs = op_channel_dirs;
  vsa->res_channel_dirs = res_channel_dirs;
}

/*
 * Calculate the op/res_engine_num
 */
void vsa_engine_num_extract(PlutoProg *prog, VSA *vsa) {
  int i, j;

  vsa->op_engine_nums = (int *)malloc(vsa->op_num * sizeof(int));
  vsa->res_engine_nums = (int *)malloc(vsa->res_num * sizeof(int));

  int op_num = vsa->op_num;
  int res_num = vsa->res_num;

  int sa_rows = vsa->sa_rows;
  int sa_cols = vsa->sa_cols;

  char **op_channel_dirs = vsa->op_channel_dirs;
  char **res_channel_dirs = vsa->res_channel_dirs;

  for (i = 0; i < op_num; i++) {
    char *op_channel_dir = op_channel_dirs[i];
    if (!strcmp(op_channel_dir, "U") || !strcmp(op_channel_dir, "D")) {
      vsa->op_engine_nums[i] = sa_cols;
    } else if (!strcmp(op_channel_dir, "L") || !strcmp(op_channel_dir, "R")) {
      vsa->op_engine_nums[i] = sa_rows;
    } else {
      /* TODO: What about I? */
      vsa->op_engine_nums[i] = sa_rows + sa_cols;
    }
  } 

  for (i = 0; i < res_num; i++) {
    char *res_channel_dir = res_channel_dirs[i];
    if (!strcmp(res_channel_dir, "U") || !strcmp(res_channel_dir, "D")) {
      vsa->res_engine_nums[i] = sa_cols;
    } else if (!strcmp(res_channel_dir, "L") || !strcmp(res_channel_dir, "R")) {
      vsa->res_engine_nums[i] = sa_rows;
    } else {
      /* TODO: What about I? */
      vsa->res_engine_nums[i] = sa_rows + sa_cols;
    }
  } 
}

/*
 * Detect the array type: local, global
 */
void vsa_type_extract(PlutoProg *prog, VSA *vsa) {
  int i, j;
  for (i = 0; i < vsa->res_num; i++) {
    char *res_name = vsa->res_names[i];
    for (j = 0; j < prog->ndeps; j++) {
      Dep *dep = prog->deps[j];
      /* RAW dep for results */
      if (IS_RAW(dep->type) && !strcmp(dep->src_acc->name, res_name)) {
        DepDis *space_dep_dis = (DepDis *)malloc(prog->array_dim * sizeof(DepDis));
        /* Collect dep dis components at space loop */
        int h;
        int space_dim = 0;
        for (h = 0; h < prog->num_hyperplanes; h++) {
          HyperplaneProperties hProps = prog->hProps[h];
          if (IS_PSA_SPACE_LOOP(hProps.psa_type)) {
            space_dep_dis[space_dim] = dep->disvec[h];
            space_dim++;
          }
        }

        if (prog->array_dim == 1) {
          if (space_dep_dis[0] != DEP_DIS_ZERO)
            break;          
        } else if (prog->array_dim == 2) {
          if (space_dep_dis[0] != DEP_DIS_ZERO || space_dep_dis[1] != DEP_DIS_ZERO)
            break;
        }
      }
    }
  }

  if ((i == vsa->res_num) && (j == prog->ndeps)) {
    vsa->type = "local";
  } else {
    vsa->type = "global";
  }  
}

/* 
 * Generate the following code for data feeders:
 * - DFHead copy-in code
 * - DFEngine read/feed code
 */
void vsa_df_code_extract(PlutoProg *prog, VSA *vsa) {
  int i;

  /* Get the tile and space band */

  /* Get the statements under the band */

  /* Get read/write access pairs with statements */

  /* Compute the read-in set */

  /* Compute the buffer size */

  /* Generate code for DFHead copy-in code, feeding code */
  /* Copy-in code: Use the local-indexed buffers */
  /* Feeding code: Use the incremental writing style */

  /* Generate code for DFEngine copy-in code, feeding code */
  /* Copy-in code: Use the incremental reading style */
  /* Feeding code: Use the local-indexed buffers */

  for (i = 0; i < vsa->op_num; i++) {
    char *op_name = vsa->op_names[i];    
  }
}

/*
 * Generate the following code for data collectors:
 * - DCHead write-out code
 * - DCEngine write/collect code
 */
void vsa_dc_code_extract(PlutoProg *prog, VSA *vsa) {  
  unsigned i;

  /* Get the tile and space band */  
  int nbands;
  Band **bands;
  bands = psa_get_part_space_bands(prog, &nbands);

  /* Get the statements under the band */
  Ploop **loops;  
  unsigned nloops;
  // Get the last space loop
  loops = psa_get_intra_tile_dist_loops(bands[0], prog, &nloops);

  /* Get read/write access pairs with statements */
  unsigned l;
  int *num_write_data, *num_read_data, *num_read_write_data, *num_data;
  int *copy_level, *outer_dist_loop_level;
  // helper statements
  Stmt ****copy_comm_stmts, ****write_out_stmts, ****data_alloc_stmts, 
    ****ref_count_update_stmts, ****copy_back_stmts;

  num_write_data = (int *)malloc(nloops * sizeof(int));
  num_read_data = (int *)malloc(nloops * sizeof(int));
  num_read_write_data = (int *)malloc(nloops * sizeof(int));
  num_data = (int *)malloc(nloops *sizeof(int));  

  copy_level = (int *)malloc(nloops * sizeof(int));
  outer_dist_loop_level = (int *)malloc(nloops * sizeof(int));

  copy_comm_stmts = (Stmt ****)malloc(nloops * sizeof(Stmt ***));
  write_out_stmts = (Stmt ****)malloc(nloops * sizeof(Stmt ***));
  data_alloc_stmts = (Stmt ****)malloc(nloops * sizeof(Stmt ***));
  ref_count_update_stmts = (Stmt ****)malloc(nloops * sizeof(Stmt ***));
  copy_back_stmts = (Stmt ****)malloc(nloops * sizeof(Stmt ***));

  int num_comm_stmts = 0;

  // init_copy_level(prog, loops, nloops, copy_level, outer_dist_loop_level);
  psa_init_copy_level(prog, loops, nloops, copy_level, outer_dist_loop_level);

  for (l = 0; l < nloops; l++) {
    Ploop *loop = loops[l];

    int *num_stmts_per_wacc; // indexed by data variable
    int *num_stmts_per_racc; // indexed by data variable
    int *num_stmts_per_acc;  // indexed by data variable

    struct stmt_access_pair ***wacc_stmts; // indexed by data variable
    struct stmt_access_pair ***racc_stmts; // indexed by data variable
    struct stmt_access_pair ***acc_stmts;  // indexed by data variable

    acc_stmts = get_read_write_access_with_stmts(
      loop->stmts, loop->nstmts, &num_read_write_data[l], &num_stmts_per_acc);
    racc_stmts = get_read_access_with_stmts(
      loop->stmts, loop->nstmts, &num_read_data[l], &num_stmts_per_racc);
    wacc_stmts = get_write_access_with_stmts(
      loop->stmts, loop->nstmts, &num_write_data[l], &num_stmts_per_wacc);

    num_data[l] = num_write_data[l];

    copy_comm_stmts[l] = 
      (Stmt ***)malloc(num_read_write_data[l] * sizeof(Stmt **));
    write_out_stmts[l] = 
      (Stmt ***)malloc(num_write_data[l] * sizeof(Stmt **));
    copy_back_stmts[l] = 
      (Stmt ***)malloc(num_write_data[l] * sizeof(Stmt **));
    data_alloc_stmts[l] = 
      (Stmt ***)malloc(num_read_write_data[l] * sizeof(Stmt **));
    ref_count_update_stmts[l] = 
      (Stmt ***)malloc(num_read_write_data[l] * sizeof(Stmt **));

    /* 
     * The anchor statement is with maximum dimension, and the domain is the 
     * union of all the statements under the current loop.
     */
    Stmt *anchor_stmt = get_new_anchor_stmt(loop->stmts, loop->nstmts);

    /* Generate the data alloc code */
    // for (i = 0; i < num_read_write_data[l]; i++) {
    //   data_alloc_stmts[l][i] = gen_data_tile_alloc_code(
    //     acc_stmts[i], num_stmts_per_acc[i], prog, copy_level, l
    //   );
    //   ref_count_update_stmts[l][i] = gen_data_tile_ref_count_update_code(
    //     acc_stmts[i], num_stmts_per_acc[i], prog, copy_level, l
    //   );
    //   copy_comm_stmts[l][i] = 
    //     psa_gen_comm_code_opt(
    //       i, acc_stmts[i], num_stmts_per_acc[i], nloops,
    //       num_read_write_data[l], prog, anchor_stmt,
    //       copy_level, outer_dist_loop_level[l], l,
    //       &num_comm_stmts, vsa
    //     );      
    // }

    // for (i = 0; i < num_write_data[l]; i++) {
    //   copy_back_stmts[l][i] = gen_copy_back_code(
    //     wacc_stmts[i], num_stmts_per_wacc[i], prog, copy_level, l
    //   );
    //   vsa_gen_data_tile_copy_back_cloog_code(prog, l, wacc_stmts[i], 
    //         num_stmts_per_wacc[i], copy_level[l], vsa);
    // }

    for (i = 0; i < num_write_data[l]; i++) {
      write_out_stmts[l][i] = psa_gen_write_out_code(
        wacc_stmts[i], num_stmts_per_wacc[i], prog, anchor_stmt,
        copy_level, outer_dist_loop_level[l], l, vsa
      );
    }

  }

  /* Compute the write-out set */

  /* Compute the buffer size */

  /* Generate code for DCHead write-out code, collecting code */
  /* Write-out code: Use the local-indexed buffers */
  /* Collecting code: Use the incremental writing style */

  /* Generate code for DCEngnine write-out code, collecting code */
  /* Write-out code: Use the incremental writing style */
  /* Collecting code: Use the local-indexed buffers */

  free(num_write_data);
  free(num_read_data);
  free(num_read_write_data);
  free(num_data);
  free(copy_level);
  free(outer_dist_loop_level);
}

/*
 * copy_level[loop_num]: number of outer loops of source loop to be treated as parameters
 * (copy_level-1)^th (0-indexed) loop should be the parallel loop)
 */
Stmt **psa_gen_write_out_code(
  struct stmt_access_pair **wacc_stmts, int num_accs,
  PlutoProg *prog, Stmt *anchor_stmt, int *copy_level, 
  int outer_dist_loop_level, int loop_num, VSA *vsa
) {
  int i, src_copy_level, acc_nrows;
  src_copy_level = copy_level[loop_num];

  // generate access string, e.g., A[d1][d2];
  char *access = reconstruct_access(wacc_stmts[0]->acc);
  acc_nrows = wacc_stmts[0]->acc->mat->nrows;
  char *acc_name = wacc_stmts[0]->acc->name;

  PlutoConstraints *write_out = NULL;
  for (i = 0; i < num_accs; i++) {
    PlutoConstraints *write_out_one;
    // compute the write-out set based on the given access function
    write_out_one = compute_write_out(wacc_stmts[i], src_copy_level, prog);
    if (write_out == NULL)
      write_out = pluto_constraints_dup(write_out_one);
    else
      write_out = pluto_constraints_unionize(write_out, write_out_one);
    pluto_constraints_free(write_out_one);    

#ifdef JIE_DEBUG
    pluto_constraints_pretty_print(stdout, write_out);
#endif    
  }

  char *lw_buf_size =
      get_parametric_bounding_box(write_out, src_copy_level, acc_nrows, 
                                  prog->npar, (const char **)prog->params);

#ifdef JIE_DEBUG
  fprintf(stdout, "[Debug] Bounding box: %s\n", lw_buf_size);
#endif

  char *lw_recv_buf_size = malloc(1280);
  strcpy(lw_recv_buf_size, lw_buf_size);

  /* Start code generation */
  

}

/*
 * The copy level is the first time loop
 * The outer dist loop level is the first space loop
 */
void psa_init_copy_level(PlutoProg *prog, Ploop **loops, int nloops,
                         int *copy_level, int *outer_dist_loop_level) {
  Band **bands;
  unsigned nbands;
  bands = pluto_get_outermost_permutable_bands(prog, &nbands);

  unsigned i, h;
  unsigned first_space_hyp, first_time_hyp;
  bool first_array_part_hyp_found;
  bool first_space_hyp_found;
  bool first_time_hyp_found;
  for (i = 0; i < nbands; i++) {
    Band *band_cur = bands[i];
    // Mark the loop properties 
    first_array_part_hyp_found = false;
    first_space_hyp_found = false;
    first_time_hyp_found = false;

    for (h = 0; h < prog->num_hyperplanes; h++) {
      if (IS_PSA_ARRAY_PART_LOOP(prog->hProps[h].psa_type) &&
        !first_array_part_hyp_found) {
        first_array_part_hyp_found = true;
      }
      if (IS_PSA_SPACE_LOOP(prog->hProps[h].psa_type) && !first_space_hyp_found) {
        first_space_hyp = h;
        first_space_hyp_found = true;
      }
      if (IS_PSA_TIME_LOOP(prog->hProps[h].psa_type) && !first_time_hyp_found) {
        first_time_hyp = h;
        first_time_hyp_found = true;
      }
    }    
  }

  for (i = 0; i < nloops; i++) {
    copy_level[i] = first_time_hyp;
    outer_dist_loop_level[i] = first_space_hyp;
  }
}

/* 
 * Get the loop band that contains the array_part and space loops
 */
Band **psa_get_part_space_bands(PlutoProg *prog, int *nbands) {  
  Band **bands;
  bands = pluto_get_outermost_permutable_bands(prog, nbands);

  assert(*nbands == 1);

  /* Change the band width */
  unsigned i, h;
  for (i = 0; i < *nbands; i++) {
    Band *band_cur = bands[i];
    /* Mark the loop properties */
    unsigned first_space_hyp, first_time_hyp;
    bool first_array_part_hyp_found = false;
    bool first_space_hyp_found = false;
    bool first_time_hyp_found = false;
    for (h = 0; h < prog->num_hyperplanes; h++) {
      if (IS_PSA_ARRAY_PART_LOOP(prog->hProps[h].psa_type) &&
        !first_array_part_hyp_found) {
        first_array_part_hyp_found = true;
      }
      if (IS_PSA_SPACE_LOOP(prog->hProps[h].psa_type) && !first_space_hyp_found) {
        first_space_hyp = h;
        first_space_hyp_found = true;
      }
      if (IS_PSA_TIME_LOOP(prog->hProps[h].psa_type) && !first_time_hyp_found) {
        first_time_hyp = h;
        first_time_hyp_found = true;
      }
    }

    band_cur->width = first_time_hyp;    
  }

  return bands;
}

/*
 * Get the last loop before the intra-tile loops.
 */
Ploop **psa_get_intra_tile_dist_loops(Band *band, PlutoProg *prog, int *nloops) {
  Ploop **loops_tmp;
  int nloops_tmp;
  loops_tmp = pluto_get_loops_under(
    band->loop->stmts, band->loop->nstmts,
    band->loop->depth + band->width - 1, prog, &nloops_tmp
  );  

  unsigned i;
  for (i = 0; i < nloops_tmp; i++) {
    if (loops_tmp[i]->depth == band->loop->depth + band->width - 1) {
      break;
    }
  }

  Ploop **loops = (Ploop**)malloc(1 * sizeof(Ploop*));
  loops[0] = loops_tmp[i];
  *nloops = 1;

  return loops;
}

/* 
 * Extract necessary information from PlutoProg to complete the fields of VSA.
 */
void pluto_prog_to_vsa(PlutoProg *prog, VSA *vsa) {
  int i, j;  

  /* SA_ROWS */
  vsa->sa_rows = prog->array_nrow;

  /* SA_COLS */
  vsa->sa_cols = prog->array_ncol;

  /* IL_ENABLE */
  vsa->il_enable = prog->array_il_enable;

  /* ROW_IL_FACTOR */
  vsa->row_il_factor = prog->array_il_factor[0];

  /* COL_IL_FACTOR */
  vsa->col_il_factor = prog->array_il_factor[1];

  /* SIMD_FACTOR */
  vsa->simd_factor = prog->array_simd_factor;

  /* OP_ENGINE_NUM, RES_ENGINE_NUM */ 
  vsa_engine_num_extract(prog, vsa);   

  /* FC_SPLIT_FACTOR */
  // TODO: add this feature in the future
  vsa->fc_split_factors = (int *)malloc((vsa->op_num + vsa->res_num) * sizeof(int));
  for (i = 0; i < vsa->op_num + vsa->res_num; i++) {
    vsa->fc_split_factors[i] = 1;
  }

  /* FC_GROUP_FACTOR */
  // TODO: add this feature in the future
  vsa->fc_group_factors = (int *)malloc((vsa->op_num + vsa->res_num) *sizeof(int));
  for (i = 0; i < vsa->op_num + vsa->res_num; i++) {
    vsa->fc_group_factors[i] = 1;
  }

  /* ITERATORS */  
  /* MAT_STAT */

  /* DF Code */
  vsa_df_code_extract(prog, vsa);

  /* DC Code */
  vsa_dc_code_extract(prog, vsa);

  return vsa;
}

VSA *vsa_alloc() {
  VSA *vsa = (VSA *)malloc(sizeof(VSA));

  vsa->df_feed_counter_code = NULL;
  vsa->df_feed_addr_cal_code = NULL;
  vsa->dc_collect_counter_code = NULL;
  vsa->dc_collect_addr_cal_code = NULL;
  vsa->array_sizes = NULL;
  vsa->dfc_head_buf_sizes = NULL;
  vsa->op_names = NULL;
  vsa->res_names = NULL;
  vsa->row_il_factor = 0;
  vsa->col_il_factor = 0;
  vsa->global_accum_num = 0;
  vsa->local_reg_num = 0;
  vsa->op_engine_nums = NULL;
  vsa->res_engine_nums = NULL;
  vsa->sa_rows = 0;
  vsa->sa_cols = 0;
  vsa->fc_split_factors = NULL;
  vsa->fc_group_factors = NULL;
  vsa->iters = NULL;
  vsa->il_enable = 0;
  vsa->simd_factor = 0;
  vsa->op_num = 0;
  vsa->res_num = 0;
  vsa->op_dims = NULL;
  vsa->res_dims = NULL;
  vsa->op_channel_dirs = NULL;
  vsa->res_channel_dirs = NULL;
  vsa->dfc_buf_sizes = NULL;
  vsa->head_code = NULL;
  vsa->mac_stat = NULL;
  vsa->last_patch_code = NULL;
  vsa->last_tile_size = NULL;
  vsa->type = NULL;

  return vsa;
}

void vsa_free(VSA *vsa) {

}

void psa_print_string_with_indent(FILE *fp, int indent, char *to_print) {
  fprintf(fp, "%*s%s", indent, "", to_print);
}

void psa_print_int_with_indent(FILE *fp, int indent, int to_print) {
  fprintf(fp, "%*s%d", indent, "", to_print);
}

void psa_print_string_list_with_indent(FILE *fp, int indent, char **list, int len) {
  int i;
  for (i = 0; i < len; i++) {
    psa_print_string_with_indent(fp, indent, "\"");
    psa_print_string_with_indent(fp, 0, list[i]);
    if (i < len - 1) {
      psa_print_string_with_indent(fp, 0, "\",\n");
    } else {
      psa_print_string_with_indent(fp, 0, "\"\n");
    }
  }
}

void psa_print_int_list_with_indent(FILE *fp, int indent, int *list, int len) {
  int i;
  for (i = 0; i < len; i++) {
    psa_print_int_with_indent(fp, indent, list[i]);
    if (i < len - 1) {
      psa_print_string_with_indent(fp, 0, ",\n");
    } else {
      psa_print_string_with_indent(fp, 0, "\n");
    }
  }
}

/* Print out the content of the vsa following the JSON format */
void psa_vsa_pretty_print(FILE *fp, const VSA *vsa) {
  int i;
  fprintf(fp, "{\n");

  /* OP_NAME */
  psa_print_string_with_indent(fp, 2, "\"OP_NAME\": [\n");
  psa_print_string_list_with_indent(fp, 4, vsa->op_names, vsa->op_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* RES_NAME */
  psa_print_string_with_indent(fp, 2, "\"RES_NAME\": [\n");
  psa_print_string_list_with_indent(fp, 4, vsa->res_names, vsa->res_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* OP_DIM */
  psa_print_string_with_indent(fp, 2, "\"OP_DIM\": [\n");
  psa_print_int_list_with_indent(fp, 4, vsa->op_dims, vsa->op_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* RES_DIM */
  psa_print_string_with_indent(fp, 2, "\"RES_DIM\": [\n");
  psa_print_int_list_with_indent(fp, 4, vsa->res_dims, vsa->res_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* OP_CHANNEL_NUM */
  psa_print_string_with_indent(fp, 2, "\"OP_CHANNEL_NUM\": ");  
  psa_print_int_with_indent(fp, 0, vsa->op_num);
  psa_print_string_with_indent(fp, 0, ",\n");

  /* RES_CHANNEL_NUM */
  psa_print_string_with_indent(fp, 2, "\"RES_CHANNEL_NUM\": ");
  psa_print_int_with_indent(fp, 0, vsa->res_num);
  psa_print_string_with_indent(fp, 0, ",\n");

  /* FC_SPLIT_FACTOR */
  psa_print_string_with_indent(fp, 2, "\"FC_SPLIT_FACTOR\": [\n");
  psa_print_int_list_with_indent(fp, 4, vsa->fc_split_factors, vsa->op_num + vsa->res_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* FC_GROUP_FACTOR */
  psa_print_string_with_indent(fp, 2, "\"FC_GROUP_FACTOR\": [\n");
  psa_print_int_list_with_indent(fp, 4, vsa->fc_group_factors, vsa->op_num + vsa->res_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* OP_CHANNEL_DIR */
  psa_print_string_with_indent(fp, 2, "\"OP_CHANNEL_DIR\": [\n");
  psa_print_string_list_with_indent(fp, 4, vsa->op_channel_dirs, vsa->op_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* RES_CHANNEL_DIR */
  psa_print_string_with_indent(fp, 2, "\"RES_CHANNEL_DIR\": [\n");
  psa_print_string_list_with_indent(fp, 4, vsa->res_channel_dirs, vsa->res_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* IL_ENABLE */
  psa_print_string_with_indent(fp, 2, "\"IL_ENABLE\": ");
  psa_print_int_with_indent(fp, 0, vsa->il_enable);
  psa_print_string_with_indent(fp, 0, ",\n");

  /* ROW_IL_FACTOR */
  psa_print_string_with_indent(fp, 2, "\"ROW_IL_FACTOR\": ");
  psa_print_int_with_indent(fp, 0, vsa->row_il_factor);
  psa_print_string_with_indent(fp, 0, ",\n");

  /* COL_IL_FACTOR */
  psa_print_string_with_indent(fp, 2, "\"COL_IL_FACTOR\": ");
  psa_print_int_with_indent(fp, 0, vsa->col_il_factor);
  psa_print_string_with_indent(fp, 0, ",\n");

  /* SIMD_FACTOR */
  psa_print_string_with_indent(fp, 2, "\"SIMD_FACTOR\": ");
  psa_print_int_with_indent(fp, 0, vsa->simd_factor);
  psa_print_string_with_indent(fp, 0, ",\n");

  /* SA_ROWS */
  psa_print_string_with_indent(fp, 2, "\"SA_ROWS\": ");
  psa_print_int_with_indent(fp, 0, vsa->sa_rows);
  psa_print_string_with_indent(fp, 0, ",\n");  

  /* SA_COLS */
  psa_print_string_with_indent(fp, 2, "\"SA_COLS\": ");
  psa_print_int_with_indent(fp, 0, vsa->sa_cols);
  psa_print_string_with_indent(fp, 0, ",\n");  

  /* OP_ENGINE_NUM */
  psa_print_string_with_indent(fp, 2, "\"OP_ENGINE_NUM\": [\n");
  psa_print_int_list_with_indent(fp, 4, vsa->op_engine_nums, vsa->op_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* RES_ENGINE_NUM */
  psa_print_string_with_indent(fp, 2, "\"RES_ENGINE_NUM\": [\n");
  psa_print_int_list_with_indent(fp, 4, vsa->res_engine_nums, vsa->res_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* TYPE */
  psa_print_string_with_indent(fp, 2, "\"TYPE\": \"");
  psa_print_string_with_indent(fp, 0, vsa->type);
  psa_print_string_with_indent(fp, 0, "\",\n");

  fprintf(fp, "}\n");
}
