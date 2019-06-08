/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 * 
 * Author: Jie Wang
 */
#include "pluto.h"
#include "psa_vsa_dfc.h"

/* 
 * Generate the following code for data feeders:
 * - DFHead copy-in code
 * - DFEngine read/feed code
 */
void vsa_df_code_extract(PlutoProg *prog, VSA *vsa) {
  int op_idx;
  for (op_idx = 0; op_idx < vsa->op_num; op_idx++) {
    char *op_name = vsa->op_names[op_idx];    
    PlutoProg *new_prog = pluto_prog_dup(prog);

    unsigned i;

    /* Get the tile and space band */
    int nbands;
    Band **bands;
    bands = psa_get_part_space_bands(new_prog, &nbands);

    /* Get the statements under the band */
    Ploop **loops;
    unsigned nloops;
    // Get the last space loop
    loops = psa_get_intra_tile_dist_loops(bands[0], new_prog, &nloops);

    /* Get read/write access pairs with statements */
    unsigned l;
    int *num_read_data, *num_data;
    int *array_part_level, *space_level, *time_level;

    // helper statements
    Stmt ****df_comm_stmts;

    num_read_data = (int *)malloc(nloops * sizeof(int));
    num_data = (int *)malloc(nloops * sizeof(int));

    array_part_level = (int *)malloc(nloops * sizeof(int));
    space_level = (int *)malloc(nloops * sizeof(int));
    time_level = (int *)malloc(nloops * sizeof(int));

    df_comm_stmts = (Stmt ****)malloc(nloops * sizeof(Stmt ***));
    
    psa_init_level(new_prog, loops, nloops, array_part_level, space_level, time_level);        

    for (l = 0; l < nloops; l++) {
      Ploop *loop = loops[l];
   
      //int *num_stmts_per_wacc; // indexed by data variable
      int *num_stmts_per_racc; // indexed by data variable
      //int *num_stmts_per_acc;  // indexed by data variable
   
      //struct stmt_access_pair ***wacc_stmts; // indexed by data variable
      struct stmt_access_pair ***racc_stmts; // indexed by data variable
      //struct stmt_access_pair ***acc_stmts;  // indexed by data variable
   
      // acc_stmts = get_read_write_access_with_stmts(
      //   loop->stmts, loop->nstmts, &num_read_write_data[l], &num_stmts_per_acc);
      racc_stmts = get_read_access_with_stmts(
         loop->stmts, loop->nstmts, &num_read_data[l], &num_stmts_per_racc);
      // wacc_stmts = get_write_access_with_stmts(
      //  loop->stmts, loop->nstmts, &num_write_data[l], &num_stmts_per_wacc);
   
      num_data[l] = num_read_data[l];
         
      df_comm_stmts[l] = 
           (Stmt ***)malloc(num_read_data[l] * sizeof(Stmt **));

      /* 
       * The anchor statement is with maximum dimension, and the domain is the 
       * union of all the statements under the current loop.
       */
      Stmt *anchor_stmt = get_new_anchor_stmt(loop->stmts, loop->nstmts);      
   
      for (i = 0; i < num_read_data[l]; i++) {
        if (!strcmp(op_name, racc_stmts[i][0]->acc->name)) {
          df_comm_stmts[l][i] = psa_gen_dfc_read_in_code(
            racc_stmts[i], num_stmts_per_racc[i], new_prog, anchor_stmt,
            array_part_level[l], space_level[l], time_level[l], l, vsa    
          );
        } else {
          df_comm_stmts[l][i] = NULL;
        }
      }   
    }    
   
    free(num_read_data);    
    free(num_data);
    free(array_part_level);
    free(space_level);    
    free(time_level);

    pluto_prog_free(new_prog);
  }
}

/*
 * Generate the following code for data collectors:
 * - DCHead write-out code
 * - DCEngine write/collect code
 */
void vsa_dc_code_extract(PlutoProg *prog, VSA *vsa) {  
  int res_idx;
  for (res_idx = 0; res_idx < vsa->res_num; res_idx++) {
    char *res_name = vsa->res_names[res_idx];
    PlutoProg *new_prog = pluto_prog_dup(prog);

    unsigned i;
   
    /* Get the tile and space band */  
    int nbands;
    Band **bands;
    bands = psa_get_part_space_bands(new_prog, &nbands);
   
    /* Get the statements under the band */
    Ploop **loops;  
    unsigned nloops;
    // Get the last space loop
    loops = psa_get_intra_tile_dist_loops(bands[0], new_prog, &nloops);
   
    /* Get read/write access pairs with statements */
    unsigned l;
    int *num_write_data, *num_data;
    int *array_part_level, *space_level, *time_level;
   
    // helper statements    
    Stmt ****dc_comm_stmts;
   
    num_write_data = (int *)malloc(nloops * sizeof(int));    
    num_data = (int *)malloc(nloops *sizeof(int));  
       
    array_part_level = (int *)malloc(nloops * sizeof(int));
    space_level = (int *)malloc(nloops * sizeof(int));
    time_level = (int *)malloc(nloops * sizeof(int));
      
    dc_comm_stmts = (Stmt ****)malloc(nloops * sizeof(Stmt ***));
    
    psa_init_level(new_prog, loops, nloops, array_part_level, space_level, time_level);
   
    for (l = 0; l < nloops; l++) {
      Ploop *loop = loops[l];
   
      int *num_stmts_per_wacc; // indexed by data variable
      //int *num_stmts_per_racc; // indexed by data variable
      //int *num_stmts_per_acc;  // indexed by data variable
   
      struct stmt_access_pair ***wacc_stmts; // indexed by data variable
      //struct stmt_access_pair ***racc_stmts; // indexed by data variable
      //struct stmt_access_pair ***acc_stmts;  // indexed by data variable
   
      // acc_stmts = get_read_write_access_with_stmts(
      //   loop->stmts, loop->nstmts, &num_read_write_data[l], &num_stmts_per_acc);
      // racc_stmts = get_read_access_with_stmts(
      //   loop->stmts, loop->nstmts, &num_read_data[l], &num_stmts_per_racc);
      wacc_stmts = get_write_access_with_stmts(
        loop->stmts, loop->nstmts, &num_write_data[l], &num_stmts_per_wacc);
   
      num_data[l] = num_write_data[l];
         
      dc_comm_stmts[l] = 
           (Stmt ***)malloc(num_write_data[l] * sizeof(Stmt **));

      /* 
       * The anchor statement is with maximum dimension, and the domain is the 
       * union of all the statements under the current loop.
       */
      Stmt *anchor_stmt = get_new_anchor_stmt(loop->stmts, loop->nstmts);      
   
      for (i = 0; i < num_write_data[l]; i++) {
        if (!strcmp(res_name, wacc_stmts[i][0]->acc->name)) {
          dc_comm_stmts[l][i] = psa_gen_dfc_write_out_code(
            wacc_stmts[i], num_stmts_per_wacc[i], new_prog, anchor_stmt,
            array_part_level[l], space_level[l], time_level[l], l, vsa    
          );
        } else {
          dc_comm_stmts[l][i] = NULL;
        }
      }   
    }    
   
    free(num_write_data);    
    free(num_data);
    free(array_part_level);
    free(space_level);    
    free(time_level);

    pluto_prog_free(new_prog);
  }
}

void pluto_stmt_loop_reverse(Stmt *stmt, int level, PlutoProg *prog) {
  int j, tmp;
  
  for (j = 0; j < stmt->trans->ncols; j++) {
    stmt->trans->val[level][j] = -stmt->trans->val[level][j];
  }
}

void pluto_reverse(PlutoProg *prog, int level) {
  int k;
  HyperplaneProperties hTmp;

  Stmt **stmts = prog->stmts;
  int nstmts = prog->nstmts;

  for (k = 0; k < nstmts; k++) {
    pluto_stmt_loop_reverse(stmts[k], level, prog);
  }
}

/*
 * Transform the space loop if needed
 * TODO: temporary solution
 */
void pluto_prog_dc_transform(PlutoProg *prog, VSA *vsa, 
                             int *dfc_engine_level, int *dfc_loader_level,
                             struct stmt_access_pair *acc_stmt,
                             int array_part_level, int space_level, int time_level) {
  int i;
  char *acc_name = acc_stmt->acc->name;  

  for (i = 0; i < vsa->res_num; i++) {
    if (!strcmp(vsa->res_names[i], acc_name)) {
      break;
    }
  }
  char *acc_channel_dir = vsa->res_channel_dirs[i];
  int array_dim = prog->array_dim;

  if (array_dim == 2) {
    if (!strcmp(acc_channel_dir, "D")) {
      pluto_interchange(prog, space_level, space_level + 1);
      pluto_reverse(prog, space_level); // given we collect the data from the end engine to the start engine
      pluto_reverse(prog, space_level + 1);

      *dfc_engine_level = space_level + 1;      
    } else if (!strcmp(acc_channel_dir, "R")) {
      pluto_reverse(prog, space_level);
      pluto_reverse(prog, space_level);
      
      *dfc_engine_level = space_level + 1;
    } else {
      fprintf(stdout, "[PSA] Error! Not supported!\n");
      exit(1);
    }
  } else if (array_dim == 1) {
    if (!strcmp(acc_channel_dir, "D")) {
      pluto_reverse(prog, space_level);

      *dfc_engine_level = space_level + 1;
    } else if (!strcmp(acc_channel_dir, "R")) {
      pluto_reverse(prog, space_level);

      *dfc_engine_level = space_level;
    } else {
      fprintf(stdout, "[PSA] Error! Not supporterd!\n");
      exit(1);
    }
  }

  *dfc_loader_level = space_level;
}

/*
 * copy_level[loop_num]: number of outer loops of source loop to be treated as parameters
 * (copy_level-1)^th (0-indexed) loop should be the parallel loop)
 */
Stmt **psa_gen_dfc_write_out_code(
  struct stmt_access_pair **wacc_stmts, int num_accs,
  PlutoProg *prog, Stmt *anchor_stmt, 
  int array_part_level, int space_level, int time_level, 
  int loop_num, VSA *vsa
) {
  int i, acc_nrows;

  // Generate access string, e.g., A[d1][d2];
  char *access = reconstruct_access(wacc_stmts[0]->acc);
  acc_nrows = wacc_stmts[0]->acc->mat->nrows;
  char *acc_name = wacc_stmts[0]->acc->name;

  // Based on the channel direction, adjust the transformation matrix  
  int dfc_engine_level;
  int dfc_loader_level;
  pluto_prog_dc_transform(prog, vsa, &dfc_engine_level, &dfc_loader_level,
                          wacc_stmts[0], 
                          array_part_level, space_level, time_level);

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Acc: %s\n", acc_name);
//   fprintf(stdout, "[Debug] engine_level: %d\n", dfc_engine_level);
//   fprintf(stdout, "[Debug] loader level: %d\n", dfc_loader_level);
// #endif

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] dfc_write_out\n");
// #endif

  /*
   * Generate code for engines
   */
  // Calculate the write out set, i.e., the data collect set for each DC engine
  PlutoConstraints *write_out_engine = NULL;
  for (i = 0; i < num_accs; i++) {
    PlutoConstraints *write_out_one;
    // compute the write-out set based on the given access function
    write_out_one = compute_write_out(wacc_stmts[i], dfc_engine_level, prog);
    if (write_out_engine == NULL)
      write_out_engine = pluto_constraints_dup(write_out_one);
    else
      write_out_engine = pluto_constraints_unionize(write_out_engine, write_out_one);
    pluto_constraints_free(write_out_one);    
  }

// #ifdef JIE_DEBUG
//   pluto_constraints_pretty_print(stdout, write_out_engine);
// #endif

  char *engine_buf_size =
      get_parametric_bounding_box(write_out_engine, dfc_engine_level, acc_nrows, 
                                  prog->npar, (const char **)prog->params);

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Bounding box: %s\n", lw_buf_size);
// #endif    

  /* Start code generation */
  char *l_buf_name, *g_buf_name, *fifo_name;
  char *l_count_name;
  char *access_function;

  // dc_engine_collect_stmt
  char *dc_engine_collect_module_name = "dc_engine_collect";
  char *dc_engine_collect_stmt_text = "";

  l_buf_name = concat("l_buf_", acc_name);
  l_count_name = concat("l_count_", acc_name);
  fifo_name = concat("fifo_", acc_name);
  dc_engine_collect_stmt_text =   
    malloc(strlen(l_buf_name) + strlen("[") + strlen(l_count_name) + 
           strlen("++] = ") + strlen(fifo_name) + strlen(".read()") + 1);
  sprintf(dc_engine_collect_stmt_text, "%s[%s++] = %s.read()", l_buf_name, 
        l_count_name, fifo_name);

  generate_scanner_code(
    vsa, prog, write_out_engine, dfc_engine_level,
    acc_name, acc_nrows,
    engine_buf_size, dc_engine_collect_stmt_text, dc_engine_collect_module_name
  );

  // dc_engine_write_stmt
  char *dc_engine_write_module_name = "dc_engine_write";
  char *dc_engine_write_stmt_text = "";

  dc_engine_write_stmt_text = 
    malloc(strlen(fifo_name) + strlen(".write(") + strlen(l_buf_name) + 
            strlen("[") + strlen(l_count_name) + strlen("++])"));
  sprintf(dc_engine_write_stmt_text, "%s.write(%s[%s++])", fifo_name, l_buf_name,
        l_count_name);

  generate_scanner_code(
    vsa, prog, write_out_engine, dfc_engine_level,
    acc_name, acc_nrows,
    engine_buf_size, dc_engine_write_stmt_text, dc_engine_write_module_name
  );        

  // generate_dc_engine_collect(
  //   vsa, prog, write_out, dfc_engine_level,
  //   acc_name, acc_nrows,
  //   lw_buf_size    
  // );  

  // generate_dc_engine_write(
  //   vsa, prog, write_out, dfc_engine_level,
  //   acc_name, acc_nrows,
  //   lw_buf_size
  // );

  pluto_constraints_free(write_out_engine);  
  free(engine_buf_size);
  free(dc_engine_collect_stmt_text);
  free(dc_engine_write_stmt_text);

  /*
   * Generate code for loaders
   */  
  PlutoConstraints *write_out_loader = NULL;
  for (i = 0; i < num_accs; i++) {
    PlutoConstraints *write_out_one;
    // compute the write-out set based on the given access function    
    write_out_one = compute_write_out(wacc_stmts[i], dfc_loader_level, prog);
    if (write_out_loader == NULL)
      write_out_loader = pluto_constraints_dup(write_out_one);
    else
      write_out_loader = pluto_constraints_unionize(write_out_loader, write_out_one);
    pluto_constraints_free(write_out_one);    
  }

// #ifdef JIE_DEBUG
//   pluto_constraints_pretty_print(stdout, write_out_loader);
// #endif      

  /* 
   * Lower bound and upper bound expressions will be extracted from the generated
   * Cloog code.
   */
  //PlutoConstraints **loader_lw_buf_lb;
  //PlutoConstraints **loader_lw_buf_ub;

  /* TODO: Add a function to get the rectangle hull */
  /*
   * isl_fixed_box * isl_map_get_range_simple_fixed_box_hull)(isl_map *map)
   */

  char *loader_buf_size =
      get_parametric_bounding_box(write_out_loader, dfc_loader_level, acc_nrows, 
                                  prog->npar, (const char **)prog->params);

  /* Start code generation */
  //char *l_buf_name, *g_buf_name, *fifo_name;

  // dc_loader_write_stmt
  char *dc_loader_write_module_name = "dc_loader_write";
  char *dc_loader_write_stmt_text = "";

  g_buf_name = concat("g_buf_", acc_name);    
  access_function = "";
  for (i = dfc_loader_level; i < dfc_loader_level + acc_nrows; i++) {
    access_function = concat(access_function, "[");
    char iter_tmp[100];
    sprintf(iter_tmp, "d%d", i + 1);
    access_function = concat(access_function, iter_tmp);
    access_function = concat(access_function, "]");
  }
  dc_loader_write_stmt_text = concat(dc_loader_write_stmt_text, acc_name);
  dc_loader_write_stmt_text = concat(dc_loader_write_stmt_text, access_function);  
  dc_loader_write_stmt_text = concat(dc_loader_write_stmt_text, " = ");
  dc_loader_write_stmt_text = concat(dc_loader_write_stmt_text, g_buf_name);
  dc_loader_write_stmt_text = concat(dc_loader_write_stmt_text, access_function);  

  // generate_scanner_code(
  //   vsa, prog, write_out_loader, dfc_loader_level,
  //   acc_name, acc_nrows,
  //   loader_buf_size, dc_loader_write_stmt_text, dc_loader_write_module_name
  // );  

  Stmt *loader_write_stmt = psa_create_helper_stmt_raw(
    dfc_loader_level + acc_nrows, write_out_loader,
    dc_loader_write_stmt_text,
    STMT_UNKNOWN, PSA_DC_LOADER_WRITE, loop_num);

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Bounding box: %s\n", loader_lw_buf_size);
// #endif  
  
// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Loader collect\n");
// #endif
  PlutoConstraints *write_out_loader_collect;
  write_out_loader_collect = compute_write_out(
    wacc_stmts[0], prog->num_hyperplanes, prog);  

// #ifdef JIE_DEBUG
//   pluto_constraints_pretty_print(stdout, loader_collect);
// #endif  

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Loader collect constraints:\n");
//   pluto_constraints_pretty_print(stdout, write_out_loader_collect);
// #endif

  // dc_loader_collect_stmt
  char *dc_loader_collect_module_name = "dc_loader_collect";
  char *dc_loader_collect_stmt_text = "";

  g_buf_name = concat("g_buf_", acc_name);    
  access_function = "";
  for (i = prog->num_hyperplanes; i < prog->num_hyperplanes + acc_nrows; i++) {
    access_function = concat(access_function, "[");
    char iter_tmp[100];
    sprintf(iter_tmp, "t%d", i + 1);
    access_function = concat(access_function, iter_tmp);
    access_function = concat(access_function, "]");
  }
  dc_loader_collect_stmt_text = concat("", g_buf_name);
  dc_loader_collect_stmt_text = concat(dc_loader_collect_stmt_text, access_function);  
  dc_loader_collect_stmt_text = concat(dc_loader_collect_stmt_text, " = ");
  dc_loader_collect_stmt_text = concat(dc_loader_collect_stmt_text, fifo_name);
  dc_loader_collect_stmt_text = concat(dc_loader_collect_stmt_text, ".read()");  

  Stmt *loader_collect_stmt = psa_create_helper_stmt_raw(
    prog->num_hyperplanes + acc_nrows, write_out_loader_collect,
    dc_loader_collect_stmt_text,
    STMT_UNKNOWN, PSA_DC_LOADER_COLLECT, loop_num
    );

  // generate_scanner_code(
  //   vsa, prog,
  //   write_out_loader_collect, prog->num_hyperplanes,
  //   acc_name, acc_nrows,
  //   loader_buf_size, 
  //   dc_loader_collect_stmt_text,
  //   dc_loader_collect_module_name       
  // );  

  // create the program
  char *dc_loader_module_name = "dc_loader";

  psa_gen_dc_loader_code (
    loader_write_stmt, loader_collect_stmt,
    dfc_loader_level,
    dc_loader_module_name,
    acc_name, acc_nrows,
    loader_buf_size
  );

  pluto_constraints_free(write_out_loader);
  pluto_constraints_free(write_out_loader_collect);    
  free(loader_buf_size);

  Stmt *dc_engine_collect_stmt = NULL;
  Stmt *dc_engine_write_stmt = NULL;
  Stmt *dc_loader_collect_stmt = NULL;
  Stmt *dc_loader_write_stmt = NULL;

#define NUM_WRITE_OUT_STMTS 4  
  Stmt **write_out_stmts = 
    (Stmt **)malloc(NUM_WRITE_OUT_STMTS * sizeof(Stmt *));

  write_out_stmts[0] = dc_engine_collect_stmt;
  write_out_stmts[1] = dc_engine_write_stmt;
  write_out_stmts[2] = dc_loader_collect_stmt;
  write_out_stmts[3] = dc_loader_write_stmt;
  
  return write_out_stmts;
}

/* Set the dimension names of type "type" according to the elements
 * in the array "names".
 */
static __isl_give isl_space *set_names(__isl_take isl_space *space,
                                       enum isl_dim_type type, char **names) {
  int i;

  for (i = 0; i < isl_space_dim(space, type); ++i)
    space = isl_space_set_dim_name(space, type, i, names[i]);

  return space;
}

PlutoConstraints *psa_convex_hull(PlutoConstraints *in) {
  isl_ctx *ctx;

  ctx = isl_ctx_alloc();

  isl_space *dim;
  isl_set *dom;
  dim = isl_space_set_alloc(ctx, 0, 2);  
  char **iter_names = (char **)malloc(2 * sizeof(char *));
  iter_names[0] = "i";
  iter_names[1] = "j";
  dim = set_names(dim, isl_dim_set, iter_names);
  // convert plutoconstraints to isl_set
  dom = isl_set_empty(isl_space_copy(dim));
  int i, j;
  int n_eq = 0, n_ineq = 0;
  isl_mat *eq, *ineq;
  isl_basic_set *bset;

  for (i = 0; i < in->nrows; i++) {
    if (in->is_eq[i]) {
      n_eq++;
    }
    else
    {
      n_ineq++;
    }    
  }

  eq = isl_mat_alloc(ctx, n_eq, in->ncols);
  ineq = isl_mat_alloc(ctx, n_ineq, in->ncols);
    
  n_eq = n_ineq = 0;
  for (i = 0; i < in->nrows; i++) {
    isl_mat **m;
    int row;

    if (in->is_eq[i]) {
      m = &eq;
      row = n_eq++;
    } else {
      m = &ineq;
      row = n_ineq++;
    }

    for (j = 0; j < in->ncols; ++j) {
      int t = in->val[i][j];
      *m = isl_mat_set_element_si(*m, row, j, t);
    }
  }

  bset = isl_basic_set_from_constraint_matrices(
    dim, eq, ineq, isl_dim_set, isl_dim_div, isl_dim_param, isl_dim_cst
  );
  dom = isl_set_from_basic_set(bset);
  char *dom_print = isl_set_to_str(dom);
  fprintf(stdout, "%s\n", dom_print);

  isl_basic_set *dom_basic = isl_set_convex_hull(dom);  

  dom_print = isl_basic_set_to_str(dom_basic);
  fprintf(stdout, "%s\n", dom_print);
  dom = isl_set_from_basic_set(dom_basic);
  PlutoConstraints *out = isl_set_to_pluto_constraints(dom);

  isl_ctx_free(ctx);

  return out;
}

/*
 * Init the level of the first loop each band: array_part, space, and time
 */
void psa_init_level(PlutoProg *prog, Ploop **loops, int nloops,
                    int *array_part_level, int *space_level, int *time_level) {
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
    array_part_level[i] = bands[0]->loop->depth;
    space_level[i] = first_space_hyp;
    time_level[i] = first_time_hyp;
    //copy_level[i] = first_time_hyp;
    //outer_dist_loop_level[i] = first_space_hyp;
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
 * copy_level[loop_num]: number of outer loops of source loop to be treated as parameters
 * (copy_level-1)^th (0-indexed) loop should be the parallel loop)
 */
Stmt **psa_gen_dfc_read_in_code(
  struct stmt_access_pair **racc_stmts, int num_accs,
  PlutoProg *prog, Stmt *anchor_stmt, 
  int array_part_level, int space_level, int time_level, 
  int loop_num, VSA *vsa
) {
  int i, acc_nrows;

  // Generate access string, e.g., A[d1][d2];
  char *access = reconstruct_access(racc_stmts[0]->acc);
  acc_nrows = racc_stmts[0]->acc->mat->nrows;
  char *acc_name = racc_stmts[0]->acc->name;

  // Based on the channel direction, adjust the transformation matrix  
  int dfc_engine_level;
  int dfc_loader_level;

// #ifdef JIE_DEBUG
//   pluto_transformations_pretty_print(prog);
// #endif

  pluto_prog_df_transform(prog, vsa, &dfc_engine_level, &dfc_loader_level,
                          racc_stmts[0], 
                          array_part_level, space_level, time_level);

// #ifdef JIE_DEBUG
//   pluto_transformations_pretty_print(prog);
// #endif

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] acc: %s\n", acc_name);
//   fprintf(stdout, "[Debug] engine_level: %d\n", dfc_engine_level);
//   fprintf(stdout, "[Debug] loader_level: %d\n", dfc_loader_level);
// #endif

  /* 
   * Generate code for engines
   */
  PlutoConstraints *read_in_engine = NULL;
  for (i = 0; i < num_accs; i++) {
    PlutoConstraints *read_in_one;
    // compute the write-out set based on the given access function
    read_in_one = compute_read_in(racc_stmts[i], dfc_engine_level, prog);
    if (read_in_engine == NULL)
      read_in_engine = pluto_constraints_dup(read_in_one);
    else
      read_in_engine = pluto_constraints_unionize(read_in_engine, read_in_one);
    pluto_constraints_free(read_in_one);    
  }

// #ifdef JIE_DEBUG
//   pluto_constraints_pretty_print(stdout, read_in_engine);
// #endif      

  char *engine_buf_size =
      get_parametric_bounding_box(read_in_engine, dfc_engine_level, acc_nrows, 
                                  prog->npar, (const char **)prog->params);  

  /* Start code generation */
  char *l_buf_name, *g_buf_name, *fifo_name;
  char *access_function;

  char *df_engine_read_stmt_text = "";
  char *df_engine_read_module_name = "df_engine_read";

  l_buf_name = concat("l_buf_", acc_name);
  g_buf_name = concat("g_buf_", acc_name);
  fifo_name = concat("fifo_", acc_name);
  access_function = "";
  for (i = dfc_engine_level; i < dfc_engine_level + acc_nrows; i++) {
    access_function = concat(access_function, "[");
    char iter_tmp[100];
    sprintf(iter_tmp, "d%d", i + 1);
    access_function = concat(access_function, iter_tmp);
    access_function = concat(access_function, "]");
  }

  df_engine_read_stmt_text = concat(df_engine_read_stmt_text, l_buf_name);
  df_engine_read_stmt_text = concat(df_engine_read_stmt_text, access_function);
  df_engine_read_stmt_text = concat(df_engine_read_stmt_text, " = ");
  df_engine_read_stmt_text = concat(df_engine_read_stmt_text, fifo_name);
  df_engine_read_stmt_text = concat(df_engine_read_stmt_text, ".read()");

  generate_scanner_code(
    vsa, prog, read_in_engine, dfc_engine_level,
    acc_name, acc_nrows,
    engine_buf_size,
    df_engine_read_stmt_text,
    df_engine_read_module_name
  ); 

  // generate_df_engine_read(
  //   vsa, prog, read_in_engine, dfc_engine_level,
  //   acc_name, acc_nrows,
  //   loader_lr_buf_size
  // );
  PlutoConstraints *read_in_engine_feed = NULL;
  int copy_level = racc_stmts[0]->stmt->dim;
  // read_in_engine_feed = compute_read_in_engine_reuse(
  //   racc_stmts[0], copy_level, dfc_engine_level, prog
  // );
  read_in_engine_feed = compute_read_in(
    racc_stmts[0], copy_level, prog
  );

  char *df_engine_feed_stmt_text = "";
  char *df_engine_feed_module_name = "df_engine_feed";

  l_buf_name = concat("l_buf_", acc_name);
  g_buf_name = concat("g_buf_", acc_name);
  fifo_name = concat("fifo_", acc_name);
  access_function = "";
  for (i = copy_level; i < copy_level + acc_nrows; i++) {
    access_function = concat(access_function, "[");
    char iter_tmp[100];
    sprintf(iter_tmp, "d%d", i + 1);
    access_function = concat(access_function, iter_tmp);
    access_function = concat(access_function, "]");
  }

  df_engine_feed_stmt_text = concat(df_engine_feed_stmt_text, fifo_name);
  df_engine_feed_stmt_text = concat(df_engine_feed_stmt_text, ".write(");
  df_engine_feed_stmt_text = concat(df_engine_feed_stmt_text, l_buf_name);
  df_engine_feed_stmt_text = concat(df_engine_feed_stmt_text, access_function);
  df_engine_feed_stmt_text = concat(df_engine_feed_stmt_text, ")");  

  generate_scanner_code(
    vsa, prog, read_in_engine_feed, copy_level,
    acc_name, acc_nrows,
    engine_buf_size,
    df_engine_feed_stmt_text,
    df_engine_feed_module_name
  );   

  /*
   * Generate code for loaders
   */    
  PlutoConstraints *read_in_loader = NULL;
  for (i = 0; i < num_accs; i++) {
    PlutoConstraints *read_in_one;
    // compute the write-out set based on the given access function
    read_in_one = compute_read_in(racc_stmts[i], dfc_loader_level, prog);
    if (read_in_loader == NULL)
      read_in_loader = pluto_constraints_dup(read_in_one);
    else
      read_in_loader = pluto_constraints_unionize(read_in_loader, read_in_one);
    pluto_constraints_free(read_in_one);    
  }

// #ifdef JIE_DEBUG
//   pluto_constraints_pretty_print(stdout, read_in_loader);
// #endif      

  char *loader_buf_size =
      get_parametric_bounding_box(read_in_loader, dfc_loader_level, acc_nrows, 
                                  prog->npar, (const char **)prog->params);

  l_buf_name = concat("l_buf_", acc_name);
  g_buf_name = concat("g_buf_", acc_name);
  fifo_name = concat("fifo_", acc_name);
  access_function = "";
  for (i = dfc_loader_level; i < dfc_loader_level + acc_nrows; i++) {
    access_function = concat(access_function, "[");
    char iter_tmp[100];
    sprintf(iter_tmp, "d%d", i + 1);
    access_function = concat(access_function, iter_tmp);
    access_function = concat(access_function, "]");
  }

  char *df_loader_read_module_name = "df_loader_read";
  char *df_loader_read_stmt_text = "";

  df_loader_read_stmt_text = concat(df_loader_read_stmt_text, g_buf_name);
  df_loader_read_stmt_text = concat(df_loader_read_stmt_text, access_function);
  df_loader_read_stmt_text = concat(df_loader_read_stmt_text, " = ");
  df_loader_read_stmt_text = concat(df_loader_read_stmt_text, acc_name);
  df_loader_read_stmt_text = concat(df_loader_read_stmt_text, access_function);  

  // generate_scanner_code(
  //   vsa, prog,
  //   read_in_loader, dfc_loader_level,
  //   acc_name, acc_nrows,
  //   loader_buf_size,
  //   df_loader_read_stmt_text,
  //   df_loader_read_module_name
  // );

  Stmt *loader_read_stmt = psa_create_helper_stmt_raw(
      dfc_loader_level + acc_nrows, read_in_loader,
      df_loader_read_stmt_text, 
      STMT_UNKNOWN, PSA_DF_LOADER_READ, loop_num);

  // Stmt *loader_read_stmt = psa_create_helper_stmt(
  //   anchor_stmt, dfc_loader_level, df_loader_read_stmt_text,
  //   STMT_UNKNOWN, PSA_DF_LOADER_READ, loop_num
  // );  

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Bounding box: %s\n", loader_lw_buf_size);
// #endif  
  
  PlutoConstraints *read_in_loader_feed = NULL;
  read_in_loader_feed = compute_read_in(racc_stmts[0], dfc_engine_level, prog);

  l_buf_name = concat("l_buf_", acc_name);
  g_buf_name = concat("g_buf_", acc_name);
  fifo_name = concat("fifo_", acc_name);
  access_function = "";
  for (i = dfc_engine_level; i < dfc_engine_level + acc_nrows; i++) {
    access_function = concat(access_function, "[");
    char iter_tmp[100];
    sprintf(iter_tmp, "d%d", i + 1);
    access_function = concat(access_function, iter_tmp);
    access_function = concat(access_function, "]");
  }

  char *df_loader_feed_module_name = "df_loader_feed";
  char *df_loader_feed_stmt_text = "";

  df_loader_feed_stmt_text = concat(df_loader_feed_stmt_text, fifo_name);
  df_loader_feed_stmt_text = concat(df_loader_feed_stmt_text, ".write(");
  df_loader_feed_stmt_text = concat(df_loader_feed_stmt_text, g_buf_name);
  df_loader_feed_stmt_text = concat(df_loader_feed_stmt_text, access_function);
  df_loader_feed_stmt_text = concat(df_loader_feed_stmt_text, ")");

  // generate_scanner_code(
  //   vsa, prog,
  //   read_in_loader_feed, dfc_engine_level,
  //   acc_name, acc_nrows,
  //   loader_buf_size,
  //   df_loader_feed_stmt_text,
  //   df_loader_feed_module_name
  // );
  Stmt *loader_feed_stmt = psa_create_helper_stmt_raw(
      dfc_engine_level + acc_nrows, read_in_loader_feed,
      df_loader_feed_stmt_text, 
      STMT_UNKNOWN, PSA_DF_LOADER_FEED, loop_num
      );      

  // create the program 
  char *df_loader_module_name = "df_loader";

  psa_gen_df_loader_code(
    loader_read_stmt, loader_feed_stmt,
    dfc_loader_level,
    df_loader_module_name,    
    acc_name, acc_nrows,
    loader_buf_size
  );

  pluto_constraints_free(read_in_loader);
  pluto_constraints_free(read_in_loader_feed);  
  free(loader_buf_size);
  free(engine_buf_size);

  Stmt *df_engine_feed_stmt = NULL;
  Stmt *df_engine_read_stmt = NULL;
  Stmt *df_loader_feed_stmt = NULL;
  Stmt *df_loader_read_stmt = NULL;

#define NUM_READ_IN_STMTS 4  
  Stmt **read_in_stmts = 
    (Stmt **)malloc(NUM_READ_IN_STMTS * sizeof(Stmt *));

  read_in_stmts[0] = df_engine_feed_stmt;
  read_in_stmts[1] = df_engine_read_stmt;
  read_in_stmts[2] = df_loader_feed_stmt;
  read_in_stmts[3] = df_loader_read_stmt;
  
  return read_in_stmts;
}

void psa_gen_df_loader_code(
  Stmt *read_write_stmt,
  Stmt *feed_collect_stmt,
  unsigned level,
  char *module_name,
  char *acc_name, unsigned acc_nrows,
  char *buf_size
) {
  char base_name[1024];
  strcpy(base_name, "");
  sprintf(base_name + strlen(base_name), "module_code/%s_%s_code", acc_name, module_name);

  char file_name[1024];
  strcpy(file_name, base_name);
  sprintf(file_name + strlen(file_name), ".c");

  char cloog_file_name[1024];
  strcpy(cloog_file_name, base_name);
  sprintf(cloog_file_name + strlen(cloog_file_name), ".cloog");

  FILE *fp = fopen(file_name, "w");
  assert(fp != NULL);

  fprintf(fp, "data_t_%s l_buf_%s[%s];\n", acc_name, acc_name, buf_size);

  PlutoProg *prog = pluto_prog_alloc();
  Stmt **sep_stmts = (Stmt **)malloc(2 * sizeof(Stmt *));

  pluto_add_given_stmt(prog, read_write_stmt);
  pluto_add_given_stmt(prog, feed_collect_stmt);
  sep_stmts[0] = read_write_stmt;
  sep_stmts[1] = feed_collect_stmt;

  // seperate these stmts
  pluto_separate_stmts(prog, sep_stmts, 2, level, 0);

  // print out
  FILE *cloogfp = fopen(cloog_file_name, "w+");
  assert(cloogfp != NULL);

  pluto_gen_cloog_file(cloogfp, prog);
  rewind(cloogfp);
  fflush(cloogfp);

  psa_generate_declarations(prog, fp);
  pluto_gen_cloog_code(prog, PLMAX(read_write_stmt->dim, feed_collect_stmt->dim),
      1, cloogfp, fp);

  fclose(fp);
  fclose(cloogfp);
}

void psa_gen_dc_loader_code(
  Stmt *read_write_stmt,
  Stmt *feed_collect_stmt,
  unsigned level,
  char *module_name,
  char *acc_name, unsigned acc_nrows,
  char *buf_size
) {
  char base_name[1024];
  strcpy(base_name, "");
  sprintf(base_name + strlen(base_name), "module_code/%s_%s_code", acc_name, module_name);

  char file_name[1024];
  strcpy(file_name, base_name);
  sprintf(file_name + strlen(file_name), ".c");

  char cloog_file_name[1024];
  strcpy(cloog_file_name, base_name);
  sprintf(cloog_file_name + strlen(cloog_file_name), ".cloog");

  FILE *fp = fopen(file_name, "w");
  assert(fp != NULL);

  fprintf(fp, "data_t_%s l_buf_%s[%s];\n", acc_name, acc_name, buf_size);

  PlutoProg *prog = pluto_prog_alloc();
  Stmt **sep_stmts = (Stmt **)malloc(2 * sizeof(Stmt *));

  pluto_add_given_stmt(prog, read_write_stmt);
  pluto_add_given_stmt(prog, feed_collect_stmt);
  //sep_stmts[0] = read_write_stmt;
  //sep_stmts[1] = feed_collect_stmt;
  sep_stmts[0] = feed_collect_stmt;
  sep_stmts[1] = read_write_stmt;

  // seperate these stmts
  pluto_separate_stmts(prog, sep_stmts, 2, level, 0);

  // print out
  FILE *cloogfp = fopen(cloog_file_name, "w+");
  assert(cloogfp != NULL);

  pluto_gen_cloog_file(cloogfp, prog);
  rewind(cloogfp);
  fflush(cloogfp);

  psa_generate_declarations(prog, fp);
  pluto_gen_cloog_code(prog, PLMAX(read_write_stmt->dim, feed_collect_stmt->dim),
      1, cloogfp, fp);

  fclose(fp);
  fclose(cloogfp);
}

/*
 * Transform the space loop if needed
 * TODO: temporary solution
 */
void pluto_prog_df_transform(PlutoProg *prog, VSA *vsa, 
                             int *dfc_engine_level, int *dfc_loader_level,
                             struct stmt_access_pair *acc_stmt,
                             int array_part_level, int space_level, int time_level) {
  int i;
  char *acc_name = acc_stmt->acc->name;  

  for (i = 0; i < vsa->op_num; i++) {
    if (!strcmp(vsa->op_names[i], acc_name)) {
      break;
    }
  }
  char *acc_channel_dir = vsa->op_channel_dirs[i];
  int array_dim = prog->array_dim;

  if (array_dim == 2) {
    if (!strcmp(acc_channel_dir, "D")) {
      pluto_interchange(prog, space_level, space_level + 1);      

      *dfc_engine_level = space_level + 1;      
    } else if (!strcmp(acc_channel_dir, "R")) {      
      
      *dfc_engine_level = space_level + 1;
    } else {
      fprintf(stdout, "[PSA] Error! Not supported!\n");
      exit(1);
    }
  } else if (array_dim == 1) {
    if (!strcmp(acc_channel_dir, "D")) {      
      *dfc_engine_level = space_level + 1;
    } else if (!strcmp(acc_channel_dir, "R")) {      
      *dfc_engine_level = space_level;
    } else {
      fprintf(stdout, "[PSA] Error! Not supporterd!\n");
      exit(1);
    }
  }

  *dfc_loader_level = space_level;
}

void generate_scanner_code(
  VSA *vsa, PlutoProg *prog,
  PlutoConstraints *read_in_write_out, int copy_level,
  char *acc_name, int acc_nrows,
  char *buf_size,
  char *stmt_text,
  char *module_name
) {
  int i;
  char base_name[1024];
  strcpy(base_name, "");
  sprintf(base_name + strlen(base_name), "module_code/%s_%s_code", acc_name, module_name);  

  char file_name[1024];
  strcpy(file_name, base_name);
  sprintf(file_name + strlen(file_name), ".c");

  char cloog_file_name[1024];
  strcpy(cloog_file_name, base_name);
  sprintf(cloog_file_name + strlen(cloog_file_name), ".cloog");

  FILE *fp = fopen(file_name, "w");
  assert(fp != NULL);

  fprintf(fp, "data_t_%s l_buf_%s[%s];\n", acc_name, acc_name, buf_size);

  char **iters = (char **)malloc((copy_level + acc_nrows) * sizeof(char *));  
  for (i = 0; i < copy_level + acc_nrows; i++) {
    iters[i] = malloc(13);
    sprintf(iters[i], "d%d", i + 1);
  }

  FILE *cloogfp = NULL;
  PlutoProg *print_prog = pluto_prog_alloc();

  for (i = 0; i < prog->npar; i++) {
    pluto_prog_add_param(print_prog, prog->params[i], print_prog->npar);
  }

  for (i = 0; i < copy_level + acc_nrows; i++) {
    pluto_prog_add_hyperplane(print_prog, 0, H_UNKNOWN, PSA_H_UNKNOWN);
  }  

  PlutoMatrix *trans;
  trans = pluto_matrix_identity(copy_level + acc_nrows);
  for (i = 0; i < prog->npar + 1; i++) {
    pluto_matrix_add_col(trans, trans->ncols);
  }

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] nrows: %d ncols: %d\n", dc_trans->nrows, dc_trans->ncols);
// #endif
  
  pluto_add_stmt(print_prog, read_in_write_out, trans, iters, stmt_text, ORIG);

  cloogfp = fopen(cloog_file_name, "w+");
  assert(cloogfp != NULL);
  
  pluto_gen_cloog_file(cloogfp, print_prog);
  rewind(cloogfp);
  fflush(cloogfp);

  // Generete the code
  psa_generate_declarations(print_prog, fp);
  pluto_gen_cloog_code(print_prog, 1, copy_level + acc_nrows, cloogfp, fp);

  pluto_matrix_free(trans);    
  pluto_prog_free(print_prog);

  fclose(fp);
  fclose(cloogfp);        
}