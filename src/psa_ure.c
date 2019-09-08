#include "psa_ure.h"

/*
 * This function extracts the I/O network definition for T2S
 * and the I/O network optimization 
 * The current algorithm:
 * If the variable has drain variable, allocate drainer, collector, unloader, and desrializer for it
 * Else allocate serializer, loader, and feeder.
 */
void vsa_t2s_IO_extract(PlutoProg *prog, VSA *vsa) {
//  char ***IO_func_list = NULL; // organized by variable name;
//  int * num_func_per_var;
  Graph *adg = prog->adg;
//  IO_func_list = (char ***)malloc(adg->num_ccs * sizeof(char **));
//  num_func_per_var = (int *)malloc(adg->num_ccs * sizeof(int));
  struct var_pair **adg_var_map = vsa->adg_var_map;

  int t2s_IO_func_num = 0;
  char **t2s_IO_func_names = NULL;
  int t2s_IO_build_num = 0;
  char **t2s_IO_build_calls = NULL;

  for (int cc_id = 0; cc_id < adg->num_ccs; cc_id++) {    
    if (adg_var_map[cc_id]->d) {
//      IO_func_list[cc_id] = (char **)malloc(4 * sizeof(char *));
//      num_func_per_var[cc_id] = 4;
      t2s_IO_func_names = realloc(t2s_IO_func_names, (t2s_IO_func_num + 4) * sizeof(char *));
      t2s_IO_build_calls = realloc(t2s_IO_build_calls, (t2s_IO_build_num + 1) * sizeof(char *));
      char *drainer = strdup(adg_var_map[cc_id]->var_name);
      char *collector = strdup(adg_var_map[cc_id]->var_name);
      char *unloader = strdup(adg_var_map[cc_id]->var_name);      
      char *deserializer = strdup(adg_var_map[cc_id]->var_name);
//      sprintf(drainer, "%s_drainer", adg_var_map[cc_id]->var_name);
//      sprintf(collector, "%s_collector", adg_var_map[cc_id]->var_name);
//      sprintf(unloader, "%s_unloader", adg_var_map[cc_id]->var_name);
//      sprintf(deserializer, "%s_deserializer(Place::Host)", adg_var_map[cc_id]->var_name);
      drainer = concat(drainer, "_drainer");
      collector = concat(collector, "_collector");
      unloader = concat(unloader, "_unloader");
      deserializer = concat(deserializer, "_deserializer(Place::Host)");
      t2s_IO_func_names[t2s_IO_func_num + 0] = strdup(drainer);
      t2s_IO_func_names[t2s_IO_func_num + 1] = strdup(collector);
      t2s_IO_func_names[t2s_IO_func_num + 2] = strdup(unloader);
      t2s_IO_func_names[t2s_IO_func_num + 3] = strdup(deserializer);     
      t2s_IO_func_num += 4;
      char build_API[200];
      sprintf(build_API, "isolate_producer_chain(%s, %s, %s, %s %s)",
          adg_var_map[cc_id]->var_name,
          drainer, collector, unloader, deserializer);
      t2s_IO_build_calls[t2s_IO_build_num] = strdup(build_API);
      t2s_IO_build_num++;

      free(drainer);
      free(collector);
      free(unloader);
      free(deserializer);
    } else {
//      IO_func_list[cc_id] = (char **)malloc(3 * sizeof(char *));
//      num_func_per_var[cc_id] = 3;
      t2s_IO_func_names = realloc(t2s_IO_func_names, (t2s_IO_func_num + 3) * sizeof(char *));
      t2s_IO_build_calls = realloc(t2s_IO_build_calls, (t2s_IO_build_num + 1) * sizeof(char *));
      char *serializer = strdup(adg_var_map[cc_id]->var_name);
      char *loader = strdup(adg_var_map[cc_id]->var_name);
      char *feeder = strdup(adg_var_map[cc_id]->var_name);
//      sprintf(serializer, "%s_serializer(Place::Host)", adg_var_map[cc_id]->var_name);
//      sprintf(loader, "%s_loader", adg_var_map[cc_id]->var_name);
//      sprintf(feeder, "%s_feeder", adg_var_map[cc_id]->var_name);
      serializer = concat(serializer, "_serializer(Place::Host)");
      loader = concat(loader, "_loader");
      feeder = concat(feeder, "_feeder");
      t2s_IO_func_names[t2s_IO_func_num + 0] = strdup(serializer);
      t2s_IO_func_names[t2s_IO_func_num + 1] = strdup(loader);
      t2s_IO_func_names[t2s_IO_func_num + 2] = strdup(feeder);
      t2s_IO_func_num += 3;
      char build_API[200];
      sprintf(build_API, "isolate_consumer_chain(%s, %s, %s, %s)",
          adg_var_map[cc_id]->var_name,
          feeder, loader, serializer);
      t2s_IO_build_calls[t2s_IO_build_num] = strdup(build_API);
      t2s_IO_build_num++;

      free(serializer);
      free(loader);
      free(feeder);
    }
  }
  
  vsa->t2s_IO_func_num = t2s_IO_func_num;
  vsa->t2s_IO_func_names = t2s_IO_func_names;
  vsa->t2s_IO_build_num = t2s_IO_build_num;
  vsa->t2s_IO_build_calls = t2s_IO_build_calls;
}

/* Append one URE list to another list */
URE **URE_append(URE **list1, int *num1, URE **list2, int num2) {
  list1 = realloc(list1, (*num1 + num2) * sizeof(URE*));
  for (int i = *num1; i < *num1 + num2; i++) {
    list1[i] = list2[i - *num1];
  }
  *num1 = *num1 + num2;
  return list1;
}

/*  Add one URE to one URE list */
URE **URE_add(URE **list, int *num, URE *ele) {
  list = realloc(list, (*num + 1) * sizeof(URE *));
  list[*num] = ele;
  *num = *num + 1;
  return list;
}

// Compare the scalar column at the last row of the scattering function
// Return 0 if both equal, which means that that are executed at the same time
// Return 1 if the first stmt executes later than the second
// Return -1 if the first stmt executes earlier than the second
int t2s_compare_stmt_order(Stmt *stmt1, Stmt *stmt2, int band_width) {
  int scalar1 = stmt1->trans->val[band_width][stmt1->trans->ncols - 1];
  int scalar2 = stmt2->trans->val[band_width][stmt2->trans->ncols - 1];
  if (scalar1 == scalar2)
    return 0;
  else if (scalar1 > scalar2) 
    return 1;
  else if (scalar1 < scalar2)
    return -1;
}

/*
 * This function returns the original access function in the form of A(t1, t3).
 */
char *create_orig_acc_str(Stmt *stmt, PlutoAccess *acc, PlutoProg *prog, VSA *vsa) {
  // compute the new access function
  PlutoMatrix *new_acc = pluto_matrix_dup(acc->mat);

  int npar = prog->npar;
  char **params = prog->params;
  char **iters = vsa->t2s_iters;

  char *acc_str = "";
  acc_str = concat(acc_str, acc->name);
  acc_str = concat(acc_str, "(");
  for (int row = 0; row < new_acc->nrows; row++) {
    if (row > 0) {
      acc_str = concat(acc_str, ", ");
    }
    bool first_exp = true;
    // special case: if the whole row is zero
    bool all_zeros = true;
    for (int col = 0; col < new_acc->ncols; col++) {
      if (new_acc->val[row][col] != 0) {
        all_zeros = false;
        break;
      }
    }

    if (all_zeros) {
      acc_str = concat(acc_str, "0");
    } else {
//      if (divs[row] != 1) {
//        acc_str = concat(acc_str, "(");
//      }    
      for (int col = 0; col < new_acc->ncols; col++) {
        if (new_acc->val[row][col] != 0) {
          char exp[20];
          if (col < stmt->dim) {
            if (new_acc->val[row][col] == 1)
              sprintf(exp, "%s", iters[col]);
            else if (new_acc->val[row][col] == -1) 
              sprintf(exp, "%s", iters[col]);
            else if (new_acc->val[row][col] > 0)
              sprintf(exp, "%d * %s", new_acc->val[row][col], iters[col]);
            else 
              sprintf(exp, "%d * %s", -new_acc->val[row][col], iters[col]);
          } else if (col < stmt->dim + npar) {
            if (new_acc->val[row][col] == 1)
              sprintf(exp, "%s", params[col - stmt->dim]);
            else if (new_acc->val[row][col] == -1)
              sprintf(exp, "%s", params[col - stmt->dim]);
            else if (new_acc->val[row][col] > 0)
              sprintf(exp, "%d * %s", new_acc->val[row][col], params[col - stmt->dim]);
            else
              sprintf(exp, "%d * %s", -new_acc->val[row][col], params[col - stmt->dim]);
          } else {
            if (new_acc->val[row][col] == 1)
              sprintf(exp, "1");
            else if (new_acc->val[row][col] == -1)
              sprintf(exp, "1");
            else 
              sprintf(exp, "%d", abs(new_acc->val[row][col]));
          }
          if (first_exp) {
            if (new_acc->val[row][col] < 0)
              acc_str = concat(acc_str, "-");
            acc_str = concat(acc_str, exp);
            first_exp = !first_exp;
          } else {
            if (new_acc->val[row][col] == -1)
              acc_str = concat(acc_str, " - ");
            else
              acc_str = concat(acc_str, " + ");
            acc_str = concat(acc_str, exp);          
          }
        }      
      }
//      if (divs[row] != 1) {
//        acc_str = concat(acc_str, ") / ");
//        char exp[20];
//        sprintf(exp, "%d", divs[row]);
//        acc_str = concat(acc_str, exp);
//      }
    }
  }   
  acc_str = concat(acc_str, ")");

  pluto_matrix_free(new_acc);
//  free(divs);

  return acc_str;
}

/*
 * This function returns the transformed access function in the form of A(t1, t3).
 */
char *create_new_acc_str(Stmt *stmt, PlutoAccess *acc, PlutoProg *prog, VSA *vsa) {
  // compute the new access function
  int *divs;
  PlutoMatrix *new_acc = pluto_get_new_access_func(stmt, acc->mat, &divs);
//  pluto_matrix_print(stdout, new_acc);

  int npar = prog->npar;
  char **params = prog->params;
  char **iters = vsa->t2s_iters;

  char *acc_str = "";
  acc_str = concat(acc_str, acc->name);
  acc_str = concat(acc_str, "(");
  for (int row = 0; row < new_acc->nrows; row++) {
    if (row > 0) {
      acc_str = concat(acc_str, ", ");
    }
    bool first_exp = true;
    // special case: if the whole row is zero
    bool all_zeros = true;
    for (int col = 0; col < new_acc->ncols; col++) {
      if (new_acc->val[row][col] != 0) {
        all_zeros = false;
        break;
      }
    }

    if (all_zeros) {
      acc_str = concat(acc_str, "0");
    } else {
      if (divs[row] != 1) {
        acc_str = concat(acc_str, "(");
      }    
      for (int col = 0; col < new_acc->ncols; col++) {
        if (new_acc->val[row][col] != 0) {
          char exp[20];
          if (col < stmt->trans->nrows) {
            if (new_acc->val[row][col] == 1)
              sprintf(exp, "%s", iters[col]);
            else if (new_acc->val[row][col] == -1) 
              sprintf(exp, "%s", iters[col]);
            else if (new_acc->val[row][col] > 0)
              sprintf(exp, "%d * %s", new_acc->val[row][col], iters[col]);
            else 
              sprintf(exp, "%d * %s", -new_acc->val[row][col], iters[col]);
          } else if (col < stmt->trans->nrows + npar) {
            if (new_acc->val[row][col] == 1)
              sprintf(exp, "%s", params[col - stmt->trans->nrows]);
            else if (new_acc->val[row][col] == -1)
              sprintf(exp, "%s", params[col - stmt->trans->nrows]);
            else if (new_acc->val[row][col] > 0)
              sprintf(exp, "%d * %s", new_acc->val[row][col], params[col - stmt->trans->nrows]);
            else
              sprintf(exp, "%d * %s", -new_acc->val[row][col], params[col - stmt->trans->nrows]);
          } else {
            if (new_acc->val[row][col] == 1)
              sprintf(exp, "1");
            else if (new_acc->val[row][col] == -1)
              sprintf(exp, "1");
            else 
              sprintf(exp, "%d", abs(new_acc->val[row][col]));
          }
          if (first_exp) {
            if (new_acc->val[row][col] < 0)
              acc_str = concat(acc_str, "-");
            acc_str = concat(acc_str, exp);
            first_exp = !first_exp;
          } else {
            if (new_acc->val[row][col] == -1)
              acc_str = concat(acc_str, " - ");
            else
              acc_str = concat(acc_str, " + ");
            acc_str = concat(acc_str, exp);          
          }
        }      
      }
      if (divs[row] != 1) {
        acc_str = concat(acc_str, ") / ");
        char exp[20];
        sprintf(exp, "%d", divs[row]);
        acc_str = concat(acc_str, exp);
      }
    }
  }   
  acc_str = concat(acc_str, ")");

  pluto_matrix_free(new_acc);
  free(divs);

  return acc_str;
}

/*
 * This function returns the transformed access function in the form of t1, t2, t3.
 * We add a hack for T2S. T2S doesn't allow pure constant index such as t1, 0, t3.
 * To solve this, we will look into the transformed domain of the stmt, and check if 
 * there is equality such as t2 == 2. In that case, we will generate the index as t1, t2 - 2, t3
 */
char *create_new_acc_ref_str(Stmt *stmt, PlutoAccess *acc, PlutoProg *prog, VSA *vsa) {
  // compute the new access function
  int *divs;
  PlutoMatrix *new_acc = pluto_get_new_access_func(stmt, acc->mat, &divs);
  PlutoConstraints *new_domain = pluto_get_new_domain(stmt);
  /* Check if there is any equality */
  PlutoConstraints *eq_row = NULL;
  int eq_row_iter_loc = -1;
  for (int i = 0; i < new_domain->nrows; i++) {
    if (new_domain->is_eq[i]) {
//      // check if it only involves one iterator
//      int iter_cnt = 0;
//      for (int j = 0; j < vsa->t2s_iter_num; j++) {
//        if (new_domain->val[i][j] != 0) {
//          iter_cnt++;
//          eq_row_iter_loc = j;
//        }
//      }
//      if (iter_cnt == 1) {
//        eq_row = pluto_constraints_select_row(new_domain, i);        
//        break;
//      }

      /* Check if it only involves constant */
      bool is_valid = true;
      for (int j = stmt->trans->nrows; j < stmt->trans->nrows + prog->npar; j++) {
        if (new_domain->val[i][j] != 0) {
          is_valid = false;
          break;
        }
      }
      if (new_domain->val[i][new_domain->ncols - 1] == 0)
        is_valid = false;
      if (is_valid) {
        eq_row = pluto_constraints_select_row(new_domain, i);
        break;
      }
    }
  }

  int npar = prog->npar;
  char **params = prog->params;
  char **iters = vsa->t2s_iters;

  char *acc_str = "";
  for (int row = 0; row < new_acc->nrows; row++) {
    if (row > 0) {
      acc_str = concat(acc_str, ", ");
    }
    bool first_exp = true;

    // check if the row is a constant row
    bool constant_row = true;
    for (int col = 0; col < stmt->trans->nrows; col++) {
      if (new_acc->val[row][col] != 0) {
        constant_row = false;
        break;
      }
    }
    if (constant_row) {
      // if the row is a constant row
      // check if there is eq_row available
      // if not, do nothing
      // otherwise, process it
      if (eq_row) {
        for (int col = 0; col < stmt->trans->nrows; col++) {
          eq_row->val[0][col] = (int)((float)eq_row->val[0][col] / (-eq_row->val[0][eq_row->ncols - 1]) * new_acc->val[row][new_acc->ncols - 1]);
        }
        for (int col = stmt->trans->nrows; col < stmt->trans->nrows + prog->npar; col++) {
          eq_row->val[0][col] = new_acc->val[row][col];
        }
        eq_row->val[0][eq_row->ncols - 1] = 0;

//        for (int col = stmt->trans->nrows; col < eq_row->ncols; col++) {
//          eq_row->val[0][col] = (-1) * (eq_row->val[0][col] / (-eq_row->val[0][eq_row_iter_loc]) - new_acc->val[row][col]);
//        }
//        eq_row->val[0][eq_row_iter_loc] = 1;      
        // replace it in the original acc row
        for (int col = 0; col < new_acc->ncols; col++) {
          new_acc->val[row][col] = eq_row->val[0][col];
        }
      }
    }

//    // special case: if the whole row is zero
//    bool all_zeros = true;
//    for (int col = 0; col < new_acc->ncols; col++) {
//      if (new_acc->val[row][col] != 0) {
//        all_zeros = false;
//        break;
//      }
//    }

//    if (all_zeros) {
//      acc_str = concat(acc_str, "0");
//    } else {
      if (divs[row] != 1) {
        acc_str = concat(acc_str, "(");
      }
      for (int col = 0; col < new_acc->ncols; col++) {
        if (new_acc->val[row][col] != 0) {
          char exp[20];
          if (col < stmt->trans->nrows) {
            if (abs(new_acc->val[row][col]) == 1)
              sprintf(exp, "%s", iters[col]);
            else
              sprintf(exp, "%d * %s", abs(new_acc->val[row][col]), iters[col]);
          } else if (col < stmt->trans->nrows + npar) {
            if (abs(new_acc->val[row][col]) == 1)
              sprintf(exp, "%s", params[col - stmt->trans->nrows]);
            else
              sprintf(exp, "%d * %s", abs(new_acc->val[row][col]), params[col - stmt->trans->nrows]);
          } else {
            sprintf(exp, "%d", abs(new_acc->val[row][col]));
          }
          if (first_exp) {
            if (new_acc->val[row][col] < 0)
              acc_str = concat(acc_str, "-");
            acc_str = concat(acc_str, exp);
            first_exp = !first_exp;
          } else {
            if (new_acc->val[row][col] < 0)
              acc_str = concat(acc_str, " - ");
            else
              acc_str = concat(acc_str, " + ");
            acc_str = concat(acc_str, exp);          
          }
        }      
      }
      if (divs[row] != 1) {
        acc_str = concat(acc_str, ") / ");
        char exp[20];
        sprintf(exp, "%d", divs[row]);
        acc_str = concat(acc_str, exp);
      }
//    }
  }  

  pluto_matrix_free(new_acc);
  pluto_constraints_free(new_domain);
  free(divs);
  if (eq_row != NULL)
    pluto_constraints_free(eq_row);

  return acc_str;
}


/* This function generates UREs from the program, the fields in VSA are:
 * - UREs (URE statements)
 * - URE_num (total number of UREs)
 * - URE_names (name of URE)
 * Algorithm: 
 * We proceed statement by statement and replace the access function with the 
 * corresponding variable reference.
 * Meanwhile, we will add the necessary statements.
 * If the access is an external variable with read access, we will add the init clause and update clause.
 * Update clause, A(i,j,k) = A(i,j,k-1)
 * Init caluse, A(i,j,k) = select(.., A(i,k))
 * This two clauses will be marged together, as:
 * A(i,j,k) = select(..., A(i,k), A(i,j,k-1))
 * If the access is an exeternal variable with write access, we will add the drain caluse.
 * Drain caluse, D(i,j,k) = C(i,j,k)
 * If the access is an intermeidate variable with write access, we will add the drain clause.
 * Drain caluse, D(i,j,k) = select(..., C(i,j))
 * For each R/W access, we will also need to calculate the first read/last write conditon
 * to put in the select condition.
 * This is done by the following precedure:
 * For the read access:
 * read_iters - (RAR dest iters)
 * For the write access:
 * write_iters - (WAW src iters)
 */
void vsa_URE_extract(PlutoProg *prog, VSA *vsa) {
  int band_width = vsa->t2s_iter_num; 

  int URE_num = 0;
  URE **UREs = NULL;  

  // Sort the statements based on the lexicographic order
  // This is based on the assumption that all the statements have the scheduling functions with
  // the same dimension and are all put under the same nested loops;
  // We use the bubble sorting to sort the statements
  // TODO: To fix. What happens when there is no scalar dimension
//  for (int i = 0; i < prog->nstmts - 1; i++) {
//    for (int j = 0; j < prog->nstmts - 1 - i; j++) {
//      Stmt *stmt_first = prog->stmts[j];
//      Stmt *stmt_second = prog->stmts[j + 1];
//      int seq = t2s_compare_stmt_order(stmt_first, stmt_second, band_width);
//      if (seq > 0) {
//        Stmt *tmp = stmt_first;
//        prog->stmts[j] = stmt_second;
//        prog->stmts[j + 1] = tmp;
//      }
//    }
//  }

  // create RAR UREs
  // scan through adg, for external variable CC with RAR:
  // - unionize the iterations of all stmts and dpoly
  // - generate one single URE
  if (prog->options->dsa != 2) {
    int num_ccs = prog->adg->num_ccs;
    for (int i = 0; i < num_ccs; i++) {
      int cc_id = prog->adg->ccs[i].id;
      bool is_inter = vsa->adg_var_map[cc_id]->ei;
      bool is_CC_RAR = 0;
      if (!is_inter) {
        // external variable
        //PlutoAccess *acc = vsa->acc_var_map[prog->adg->ccs[cc_id].vertices[0]]->acc;
        PlutoAccess *acc = vsa->acc_var_map[prog->adg->ccs[cc_id].dom_id]->acc;
        for (int n = 0; n < prog->ndeps; n++) {
          if (IS_RAR(prog->deps[n]->type)) {
            if (prog->deps[n]->src_acc == acc) {
              is_CC_RAR = 1;
              break;
            }
          }
        }
      }
      if (is_CC_RAR) {
        create_RAR_UREs(cc_id, prog, vsa);
      }
    }
  }

  // convert stmts to UREs
  // generate one URE from the stmt, if there is any exisisting URE
  // with the same URE name, merge two using the select clause
  for (int stmt_id = 0; stmt_id < prog->nstmts; stmt_id++) {
    Stmt *stmt = prog->stmts[stmt_id];
    stmt_to_UREs(stmt, prog, vsa); 
  }
  
  // create drain UREs
  // TODO
  if (prog->options->dsa == 0) {
    for (int stmt_id = 0; stmt_id < prog->nstmts; stmt_id++) {
      Stmt *stmt = prog->stmts[stmt_id];
      PlutoAccess *acc = stmt->writes[0];
      create_drain_UREs(stmt, acc, prog, vsa);
    }
  } else {
    // scan through adg, for external variable CC with write acc,
    // create one collection URE
    int num_ccs = prog->adg->num_ccs;
    for (int i = 0; i < num_ccs; i++) {
      int cc_id = prog->adg->ccs[i].id;
      bool is_inter = vsa->adg_var_map[cc_id]->ei;
      bool is_CC_collect = 0;
      if (!is_inter) {
        // external variable
        is_CC_collect = vsa->acc_var_map[prog->adg->ccs[cc_id].vertices[0]]->d;
      }
      if (is_CC_collect) {
        create_collect_UREs(cc_id, prog, vsa);
      }
    }
  }

//  // scan through each statement
//  for (int stmt_id = 0; stmt_id < prog->nstmts; stmt_id++) {
//    Stmt *stmt = prog->stmts[stmt_id];
//    // UREs for the read access
//    // TODO: For now, we assume all read accesses come with RAR, will extend it later
//    if (prog->options->dsa != 2) {
//      for (int i = 0; i < stmt->nreads; i++) {
//        PlutoAccess *acc = stmt->reads[i];
//        if (vsa->acc_var_map[acc->sym_id]->ei == 0) {
//          create_RAR_UREs(stmt, acc, prog, vsa);
//        }
//      }
//    }
//    // URE for the stmt
//    stmt_to_UREs(stmt, prog, vsa);
//
//    // UREs for the write access
//    if (prog->options->dsa == 0) {
//      PlutoAccess *acc = stmt->writes[0];
//      create_drain_UREs(stmt, acc, prog, vsa);
//    }
//  }
}

char *create_URE_name(char **URE_names, int URE_num, char *var_name) {  
  int update_level = 0;  
  for (int i = 0; i < URE_num; i++) {
    char *cur_name = URE_names[i];
    if (strlen(cur_name) >= strlen(var_name)) {
      char cur_name_prefix[strlen(cur_name) + 1];
      char ch;
      int loc = 0;
      while((ch = cur_name[loc]) != '\0') {
        if (ch == '.')
          break;
        else {
          cur_name_prefix[loc] = cur_name[loc];
          loc++;
        }
      }
      cur_name_prefix[loc] = '\0';
      if (!strcmp(cur_name_prefix, var_name))
        update_level++;
    }
  }
  char *URE_name = "";
  if (update_level == 0) {
    URE_name = concat(URE_name, var_name);
  } else {
    URE_name = concat(URE_name, var_name);
    URE_name = concat(URE_name, ".update(");
    char tmp[10];
    sprintf(tmp, "%d", update_level - 1);
    URE_name = concat(URE_name, tmp);
    URE_name = concat(URE_name, ")");
  }

  return URE_name;
}

/*
 * This function creates the string of the iteration domain of the statement
 */
char *create_stmt_domain_str(Stmt *stmt, PlutoProg *prog, VSA *vsa) {
  PlutoConstraints *new_domain = pluto_get_new_domain(stmt);

  // craete a union of all statements' iterations' iteratation domains which is used to simplify the URE domain
  PlutoConstraints *anchor_domain = get_anchor_domain(prog);

  PlutoConstraints *simplify_new_domain = pluto_constraints_simplify_context_isl(new_domain, anchor_domain);

//  pluto_constraints_pretty_print(stdout, new_domain);
//  pluto_constraints_pretty_print(stdout, anchor_domain);
//  pluto_constraints_pretty_print(stdout, simplify_new_domain);

  // print out the constraints in Halide syntax
  char *const_str = pluto_constraints_to_t2s_format(simplify_new_domain, vsa, stmt->trans->nrows, prog->npar, prog->params);

  pluto_constraints_free(new_domain);
  pluto_constraints_free(simplify_new_domain);
  pluto_constraints_free(anchor_domain);

  return const_str;
}

/*
 * This function builds the iteration domain for loop instances that first read the data
 * based on the RAR dependence
 * target_iters = read_iters - (RAR dest_iters);
 * If there is no RAR dependence for this access function, naturally we will return the 
 * original iteration domain.
 */ 
PlutoConstraints *get_RAR_domain(Stmt *stmt, PlutoAccess *acc, PlutoProg *prog, VSA *vsa) {
  PlutoConstraints *new_domain = pluto_get_new_domain(stmt);
#ifdef PSA_URE_DEBUG
//  fprintf(stdout, "[Debug] RAR iteration domain:\n");
//  pluto_constraints_pretty_print(stdout, new_domain);
#endif  
  for (int i = 0; i < prog->ndeps; i++) {
    Dep *dep = prog->deps[i];
    if (IS_RAR(dep->type) && dep->src_acc == acc) {
      /* Check if the dependency is empty */
      if (pluto_constraints_is_empty(dep->dpolytope))
        continue;

      Stmt *src_stmt = prog->stmts[dep->src];
      Stmt *dest_stmt = prog->stmts[dep->dest];
      PlutoConstraints *tdpoly = pluto_get_transformed_dpoly(dep, src_stmt, dest_stmt);
      // Project out the src iters
      pluto_constraints_project_out_isl_single(tdpoly, 0, src_stmt->trans->nrows);
#ifdef PSA_URE_DEBUG
//      fprintf(stdout, "[Debug] RAR poly after projecting out the src iters:\n");
//      pluto_constraints_pretty_print(stdout, tdpoly);
#endif     
      // Subtracting out the iters that read data from other source iters
      //new_domain = pluto_constraints_subtract(new_domain, tdpoly);
      new_domain = pluto_constraints_difference_isl(new_domain, tdpoly);
#ifdef PSA_URE_DEBUG
//      fprintf(stdout, "[Debug] RAR read-in iters:\n");
//      pluto_constraints_pretty_print(stdout, new_domain);
#endif     
      pluto_constraints_free(tdpoly);
    }
  }

  if (pluto_constraints_is_empty(new_domain)) {
    pluto_constraints_free(new_domain);
    return NULL;
  }

  return new_domain;
}

/*
 * This function builds the iteration domain for loop instances that first read the data
 * based on the RAR dependence
 * target_iters = read_iters - (RAR dest_iters);
 * The iteration domain is printed following the Halide syntax
 */ 
char *create_RAR_domain_str(Stmt *stmt, PlutoAccess *acc, PlutoProg *prog, VSA *vsa) {
  PlutoConstraints *new_domain = pluto_get_new_domain(stmt);
#ifdef PSA_URE_DEBUG
//  fprintf(stdout, "[Debug] RAR iteration domain:\n");
//  pluto_constraints_pretty_print(stdout, new_domain);
#endif  
  for (int i = 0; i < prog->ndeps; i++) {
    Dep *dep = prog->deps[i];
    if (IS_RAR(dep->type) && dep->src_acc == acc) {
      Stmt *src_stmt = prog->stmts[dep->src];
      Stmt *dest_stmt = prog->stmts[dep->dest];
      PlutoConstraints *tdpoly = pluto_get_transformed_dpoly(dep, src_stmt, dest_stmt);
      // Project out the src iters
      pluto_constraints_project_out_isl_single(tdpoly, 0, src_stmt->trans->nrows);
#ifdef PSA_URE_DEBUG
//      fprintf(stdout, "[Debug] RAR poly after projecting out the src iters:\n");
//      pluto_constraints_pretty_print(stdout, tdpoly);
#endif     
      // Subtracting out the iters that read data from other source iters
      //new_domain = pluto_constraints_subtract(new_domain, tdpoly);
      new_domain = pluto_constraints_difference_isl(new_domain, tdpoly);
#ifdef PSA_URE_DEBUG
//      fprintf(stdout, "[Debug] RAR read-in iters:\n");
//      pluto_constraints_pretty_print(stdout, new_domain);
#endif     
      pluto_constraints_free(tdpoly);
    }
  }

  if (pluto_constraints_is_empty(new_domain)) {
    pluto_constraints_free(new_domain);
    return NULL;
  } else {
    /* Simplify the constraints, temporarily commented out */
//    // comparing the constraints of RAR read-in iters and the original iters
//    // if there is the same constraint of RAR read-in iters in the original iters, 
//    // eliminate that constraints
//    int new_id, full_id;
//    new_id = 0;
//    while(new_id < new_domain->nrows) {
//      PlutoConstraints *row1 = pluto_constraints_select_row(new_domain, new_id);
//      for (full_id = 0; full_id < full_domain->nrows; full_id++) {
//        PlutoConstraints *row2 = pluto_constraints_select_row(full_domain, full_id);
//        if (pluto_constraints_are_equal(row1, row2)) {
//          pluto_constraints_remove_row(new_domain, new_id);
//          break;
//        }
//      }
//      if (full_id == full_domain->nrows) {
//        new_id++;
//      }
//    }
  }
#ifdef PSA_URE_DEBUG
//  fprintf(stdout, "[Debug] RAR read-in iters (opted):\n");
//  PlutoConstraints *cst_print = new_domain;
//  while(cst_print != NULL) {
//    pluto_constraints_pretty_print(stdout, cst_print);
//    cst_print = cst_print->next;
//  }
#endif      

  // print out the constraints in Halide syntax
  char *const_str = pluto_constraints_to_t2s_format(new_domain, vsa, stmt->trans->nrows, prog->npar, prog->params);

  pluto_constraints_free(new_domain);
  return const_str;
}

/* 
 * This function builds the iteration domain for loop instances that last write the data 
 * based on the WAW dependence
 * target_iters = write_iters - (WAW src_iters);
 * The iteration domain is printed following the Halide syntax
 */
char *create_WAW_domain_str(Stmt *stmt, PlutoAccess *acc, PlutoProg *prog, VSA *vsa) {
  PlutoConstraints *new_domain = pluto_get_new_domain(stmt);
  PlutoConstraints *full_domain = pluto_constraints_dup(new_domain);
#ifdef PSA_URE_DEBUG
//  fprintf(stdout, "[Debug] WAW iteration domain:\n") ;
//  pluto_constraints_pretty_print(stdout, new_domain);
#endif

  for (int i = 0; i < prog->ndeps; i++) {
    Dep *dep = prog->deps[i];
    if (IS_WAW(dep->type) && dep->src_acc == acc) {
      Stmt *src_stmt = prog->stmts[dep->src];
      Stmt *dest_stmt = prog->stmts[dep->dest];
      PlutoConstraints *tdpoly = pluto_get_transformed_dpoly(dep, src_stmt, dest_stmt);
      // Project out the dest iters
      pluto_constraints_project_out_isl_single(tdpoly, src_stmt->trans->nrows, dest_stmt->trans->nrows);
#ifdef PSA_URE_DEBUG
//      fprintf(stdout, "[Debug] WAW poly after projecting out the dest iters:\n");
//      pluto_constraints_pretty_print(stdout, tdpoly);
#endif      
      new_domain = pluto_constraints_subtract(new_domain, tdpoly);
#ifdef PSA_URE_DEBUG
//      fprintf(stdout, "[Debug] WAW write-out iters:\n");
//      pluto_constraints_pretty_print(stdout, new_domain);
#endif      
      pluto_constraints_free(tdpoly);
    }
  }

#ifdef PSA_URE_DEBUG
//  fprintf(stdout, "[Debug] Constraints empty: %d", pluto_constraints_is_empty(new_domain));
#endif  

  if (pluto_constraints_is_empty(new_domain)) {
    pluto_constraints_free(new_domain);
    pluto_constraints_free(full_domain);
    return NULL;
  } else {
    /* Simplify the constraints, temporarily commented out */
//    // comparing the constraints of WAW write-out iters and the original iters
//    // if there is the same constraint of WAW write-out iters in the original iters,
//    // eliminate that constraints
//    int new_id, full_id;
//    new_id = 0;
//    while(new_id < new_domain->nrows) {
//      PlutoConstraints *row1 = pluto_constraints_select_row(new_domain, new_id);
//      for (full_id = 0; full_id < full_domain->nrows; full_id++) {
//        PlutoConstraints *row2 = pluto_constraints_select_row(full_domain, full_id);
//        if (pluto_constraints_are_equal(row1, row2)) {
//          pluto_constraints_remove_row(new_domain, new_id);
//          break;
//        }
//      }
//      if (full_id == full_domain->nrows) {
//        new_id++;
//      }
//    }
  }

  char *const_str = pluto_constraints_to_t2s_format(new_domain, vsa, stmt->trans->nrows, prog->npar, prog->params);

  pluto_constraints_free(new_domain);
  pluto_constraints_free(full_domain);

  return const_str;
}

/* 
 * Print out the constraints to Halide format that will be used
 * in the select condition in T2S
 */
char *pluto_constraints_to_t2s_format(const PlutoConstraints *cst, VSA *vsa, int niter, int nparam, char **params) {
  if (cst->nrows == 0) {
    return "";
  }

  char *ret_str = "";
  int cst_num = 0;
  PlutoConstraints *cur_cst = cst;
  while(cur_cst != NULL) {
    cst_num++;
    cur_cst = cur_cst->next;
  }

  cur_cst = cst;
  for (int cst_id = 0; cst_id < cst_num; cst_id++) {
    if (cst_num > 0 && cst_id > 0)
      ret_str = concat(ret_str, " || ");
    if (cst_num > 0) 
      ret_str = concat(ret_str, "(");

    int nrows = cur_cst->nrows;
    int ncols = cur_cst->ncols;
    int iter_num = vsa->t2s_iter_num;
    /* Scalar hyperplane dimensions */
    int nscalar_iter = niter - iter_num;
  
    char **iters = vsa->t2s_iters;
    bool is_first_cst = true;
    for (int i = 0; i < nrows; i++) {
      // Test if this is the constraints on the scalar hyp
      bool is_scalar_hyp_cst = false;
      for (int j = iter_num; j < niter; j++) {
        if (cur_cst->val[i][j] != 0) {
          is_scalar_hyp_cst = true;
          break;
        }
      }
      if (is_scalar_hyp_cst)
        continue;
  
      if (i > 0 && !is_first_cst) {
        ret_str = concat(ret_str, " && ");
      }
  
      if (is_first_cst)
        is_first_cst = !is_first_cst;

      bool is_first = true;
      for (int j = 0; j < ncols; j++) {
        if (cur_cst->val[i][j] != 0) {
          char exp[20];
          if (!is_first) {
            if (cur_cst->val[i][j] > 0)
              ret_str = concat(ret_str, " + ");
            else
              ret_str = concat(ret_str, " - ");
          } else {
            if (cur_cst->val[i][j] < 0) 
              ret_str = concat(ret_str, "-");
          }
          if (j < iter_num) {
            if (cur_cst->val[i][j] == 1) {
              sprintf(exp, "%s", iters[j]);
              ret_str = concat(ret_str, exp);
            } else if (cur_cst->val[i][j] == -1) {
              sprintf(exp, "%s", iters[j]);
              ret_str = concat(ret_str, exp);
            } else if (cur_cst->val[i][j] > 0) {
              sprintf(exp, "%d * %s", cur_cst->val[i][j], iters[j]);
              ret_str = concat(ret_str, exp);
            } else {
              sprintf(exp, "%d * %s", -cur_cst->val[i][j], iters[j]);
              ret_str = concat(ret_str, exp);
            }
          } else if (j < niter){
            continue;
          } else if (j < niter + nparam) {
            if (cur_cst->val[i][j] == 1) {
              sprintf(exp, "%s", params[j]);
              ret_str = concat(ret_str, exp);
            } else if (cur_cst->val[i][j] == -1) {
              sprintf(exp, "%s", params[j]);
              ret_str = concat(ret_str, exp);
            } else if (cur_cst->val[i][j] > 0){
              sprintf(exp, "%d * %s", cur_cst->val[i][j], params[j]);
              ret_str = concat(ret_str, exp);
            } else {
              sprintf(exp, "%d * %s", -cur_cst->val[i][j], params[j]);
              ret_str = concat(ret_str, exp);
            }
          } else {
            if (cur_cst->val[i][j] == 1) {
              ret_str = concat(ret_str, "1");
            } else if (cur_cst->val[i][j] == -1) {
              ret_str = concat(ret_str, "1");
            } else if (cur_cst->val[i][j] > 0){
              sprintf(exp, "%d", cur_cst->val[i][j]);
              ret_str = concat(ret_str, exp);
            } else {
              sprintf(exp, "%d", -cur_cst->val[i][j]);
              ret_str = concat(ret_str, exp);
            }
          }              
          if (is_first) {
            is_first = !is_first;
          }
        }      
      }
      if (cur_cst->is_eq[i] == 1) {
        ret_str = concat(ret_str, " == 0");
      } else {
        ret_str = concat(ret_str, " >= 0");
      }
    }

    if (cst_num > 0) 
      ret_str = concat(ret_str, ")");

    cur_cst = cst->next;
  }

  return ret_str;
}

void URE_init(URE *ure) {
  ure->name = NULL;
  ure->id = -1;
  ure->wrap_level = 1;
  ure->text = NULL;
  ure->select_cond = NULL;
  ure->select_LHS = NULL;
  ure->select_RHS = NULL;
  ure->LHS = NULL;
}

void URE_free(URE *ure) {
  free(ure->text);
  free(ure->name);
  free(ure->LHS);
  for (int i = 0; i < ure->wrap_level; i++) {
    free(ure->select_cond[i]);
    free(ure->select_LHS[i]);
    free(ure->select_RHS[i]);
  }
  free(ure->select_cond);
  free(ure->select_LHS);
  free(ure->select_RHS);

  free(ure);
}

PlutoConstraints *get_anchor_domain(PlutoProg *prog) {
  PlutoConstraints *domain = NULL;
  for (int i = 0; i < prog->nstmts; i++) {
    PlutoConstraints *new_domain = pluto_get_new_domain(prog->stmts[i]);
    if (domain == NULL) {
      if (new_domain != NULL)
        domain = pluto_constraints_dup(new_domain);
    } else {
      if (new_domain != NULL) {
//        pluto_constraints_pretty_print(stdout, domain);
//        pluto_constraints_pretty_print(stdout, new_domain);
        domain = pluto_constraints_unionize_isl(domain, new_domain);
      }
    }
    pluto_constraints_free(new_domain);
  }
  return domain;
}

void create_RAR_UREs(int cc_id, PlutoProg *prog, VSA *vsa) {
  // create a union of all statements' iteration domains which is used to simplify the URE domain
  PlutoConstraints *anchor_domain = get_anchor_domain(prog);

  int URE_num = vsa->URE_num;
  // unionize the domain str of all stmts
  PlutoConstraints *union_domain = NULL;
  for (int i = 0; i < prog->adg->ccs[cc_id].size; i++) {
    PlutoAccess *acc = vsa->acc_var_map[prog->adg->ccs[cc_id].vertices[i]]->acc;
    Stmt *stmt = vsa->acc_var_map[prog->adg->ccs[cc_id].vertices[i]]->stmt;
    PlutoConstraints *rar_domain = get_RAR_domain(stmt, acc, prog, vsa);
    if (union_domain == NULL) { 
      if (rar_domain != NULL)
        union_domain = pluto_constraints_dup(rar_domain);
    } else {
      if (rar_domain != NULL)
        union_domain = pluto_constraints_unionize_isl(union_domain, rar_domain);
    }
    pluto_constraints_free(rar_domain);
  }

  PlutoConstraints *new_union_domain = pluto_constraints_simplify_context_isl(union_domain, anchor_domain);
//  pluto_constraints_pretty_print(stdout, new_union_domain);

  char *domain_str = pluto_constraints_to_t2s_format(new_union_domain, vsa, prog->num_hyperplanes, prog->npar, prog->params);
//  PlutoAccess *anchor_acc = vsa->acc_var_map[prog->adg->ccs[cc_id].vertices[prog->adg->ccs[cc_id].size - 1]]->acc;
//  Stmt *anchor_stmt = vsa->acc_var_map[prog->adg->ccs[cc_id].vertices[prog->adg->ccs[cc_id].size - 1]]->stmt;
  int anchor_acc_id = prog->adg->ccs[cc_id].dom_id;
  PlutoAccess *anchor_acc = vsa->acc_var_map[anchor_acc_id]->acc;
  Stmt *anchor_stmt = vsa->acc_var_map[anchor_acc_id]->stmt;

  char *var_name = vsa->acc_var_map[anchor_acc->sym_id]->var_name;
  URE *merge_URE = (URE *)malloc(sizeof(URE));
  URE_init(merge_URE);
  merge_URE->id = vsa->URE_num;  
  merge_URE->wrap_level = 1;
  merge_URE->name = create_URE_name(get_vsa_URE_names(vsa->UREs, vsa->URE_num), vsa->URE_num, var_name);
  merge_URE->select_cond = realloc(merge_URE->select_cond, merge_URE->wrap_level * sizeof(char *));
  merge_URE->select_LHS = realloc(merge_URE->select_LHS, merge_URE->wrap_level * sizeof(char *));
  merge_URE->select_RHS = realloc(merge_URE->select_RHS, merge_URE->wrap_level * sizeof(char *));

  char *new_acc_str;
  if (anchor_stmt->untouched) 
    new_acc_str = create_orig_acc_str(anchor_stmt, anchor_acc, prog, vsa);
  else
    new_acc_str = create_new_acc_str(anchor_stmt, anchor_acc, prog, vsa);
  char *var_ref = vsa->acc_var_map[anchor_acc->sym_id]->var_ref;
  IterExp **var_iters = vsa->acc_var_map[anchor_acc->sym_id]->var_iters;

  IterExp **var_iters_RHS = (IterExp **)malloc(vsa->t2s_iter_num * sizeof(IterExp *));
  for (int i = 0; i < prog->ndeps; i++) {
    Dep *dep = prog->deps[i];
    if (IS_RAR(dep->type)) {
      if (dep->src_acc == anchor_acc || dep->dest_acc == anchor_acc) {
        for (int iter_id = 0; iter_id < vsa->t2s_iter_num; iter_id++) {
          var_iters_RHS[iter_id] = (IterExp *)malloc(sizeof(IterExp));
          int diff = dep->disvec[iter_id];

          var_iters_RHS[iter_id]->iter_name = strdup(var_iters[iter_id]->iter_name);
          var_iters_RHS[iter_id]->iter_offset = var_iters[iter_id]->iter_offset - diff;
        }
      }
    }
  }
  char var_ref_RHS[50];
  char *str_tmp = get_iter_str(var_iters_RHS, vsa->t2s_iter_num);
  sprintf(var_ref_RHS, "%s(%s)", var_name, str_tmp);
  free(str_tmp);
  
  merge_URE->select_cond[merge_URE->wrap_level - 1] = strdup(domain_str);
  merge_URE->select_LHS[merge_URE->wrap_level - 1] = strdup(new_acc_str);
  merge_URE->select_RHS[merge_URE->wrap_level - 1] = strdup(var_ref_RHS);
  merge_URE->LHS = strdup(var_ref); 

  merge_URE->text = create_URE_text(merge_URE);

  vsa->UREs = URE_add(vsa->UREs, &vsa->URE_num, merge_URE);

  for (int i = 0; i < vsa->t2s_iter_num; i++) {
    free(var_iters_RHS[i]->iter_name);
    free(var_iters_RHS[i]);
  }

  free(new_acc_str);
  free(var_iters_RHS);
  pluto_constraints_free(anchor_domain);
  pluto_constraints_free(union_domain);
  pluto_constraints_free(new_union_domain);
  free(domain_str);
}

char *create_URE_text(URE *ure) {
  char *text = "";
  text = concat(text, ure->LHS);
  text = concat(text, " = ");
    
  for (int i = 0; i < ure->wrap_level; i++) {
    text = concat(text, "select(");
    text = concat(text, ure->select_cond[i]);
    text = concat(text, ", ");
    text = concat(text, ure->select_LHS[i]);
    text = concat(text, ", ");
  }
  for (int i = ure->wrap_level - 1; i >= 0; i--) {
    if (i == ure->wrap_level - 1) {
      text = concat(text, ure->select_RHS[i]);
    }
    text = concat(text, ")");
  }
  text = concat(text, ";");

  return text;
}

void create_collect_UREs(int cc_id, PlutoProg *prog, VSA *vsa) {
  char *LHS = "";
  LHS = concat(LHS, "APP(");
  for (int i = 0; i < vsa->t2s_iter_num; i++) {
    if (i > 0)
      LHS = concat(LHS, ", ");
    LHS = concat(LHS, vsa->t2s_iters[i]);
  }
  LHS = concat(LHS, ")");

  char *RHS = strdup(vsa->acc_var_map[prog->adg->ccs[cc_id].vertices[0]]->var_ref);

  char *text = "";
  text = concat(text, LHS);
  text = concat(text, " = ");
  text = concat(text, RHS);
  text = concat(text, ";");

  URE *collect_URE = (URE *)malloc(sizeof(URE));
  URE_init(collect_URE);
  collect_URE->id = vsa->URE_num;
  collect_URE->name = strdup("APP");
  collect_URE->text = strdup(text);
  collect_URE->wrap_level = 0;

  vsa->UREs = URE_add(vsa->UREs, &vsa->URE_num, collect_URE);

  free(LHS);
  free(RHS);
  free(text);
}

void create_drain_UREs(Stmt *stmt, PlutoAccess *acc, PlutoProg *prog, VSA *vsa) {
//  *URE_num = 0;
//  URE **UREs = (URE **)malloc(2 * sizeof(URE *));

  char *domain_str = create_WAW_domain_str(stmt, acc, prog, vsa);
  if (domain_str) {
    // update caluse
    URE* update_URE = (URE *)malloc(sizeof(URE));
    char *var_name = vsa->acc_var_map[acc->sym_id]->var_name;
    char *var_ref = vsa->acc_var_map[acc->sym_id]->var_ref;
    IterExp **var_iters = vsa->acc_var_map[acc->sym_id]->var_iters;
  
    char *dvar_name = vsa->acc_var_map[acc->sym_id]->dvar_name;
    char *dvar_ref = vsa->acc_var_map[acc->sym_id]->dvar_ref;
    update_URE->id = vsa->URE_num;
    update_URE->name = create_URE_name(get_vsa_URE_names(vsa->UREs, vsa->URE_num), vsa->URE_num, dvar_name);
  
    // generate the text
    char *text = "";
    text = concat(text, dvar_ref);
    if (strcmp(domain_str, "")) {
      text = concat(text, " = select(");
      text = concat(text, domain_str);
      text = concat(text, ", ");
      text = concat(text, var_ref);
      text = concat(text, ");");
    } else {
      text = concat(text, " = ");
      text = concat(text, var_ref);
      text = concat(text, ";");      
    } 
    update_URE->text = strdup(text);
  
    vsa->UREs = URE_add(vsa->UREs, &vsa->URE_num, update_URE);

    // write clause
    URE *write_URE = (URE *)malloc(sizeof(URE));
    write_URE->id = vsa->URE_num;
    write_URE->name = create_URE_name(get_vsa_URE_names(vsa->UREs, vsa->URE_num), vsa->URE_num, dvar_name);

    // generate the text
    char *new_acc_str = create_new_acc_str(stmt, acc, prog, vsa);
    text = "";
    text = concat(text, new_acc_str);
    text = concat(text, " = ");
    text = concat(text, dvar_ref);
    text = concat(text, "; // delete it in URE list and merge_UREs args during execution");
    write_URE->text = strdup(text);
  
    vsa->UREs = URE_add(vsa->UREs, &vsa->URE_num, write_URE);
  } 
}

void stmt_to_UREs(Stmt *stmt, PlutoProg *prog, VSA *vsa) {
  char *domain_str = create_stmt_domain_str(stmt, prog, vsa);

  int wacc_cnt = 0;
  int racc_cnt = 0;
  char *text = strdup(stmt->text);

  // PET will assign out-of-order IDs to access functions in the statement
  // In order to parse all references, we will iteratively run multiple 
  // times untill we substitute all references in the original statement
  while(wacc_cnt != stmt->nwrites || racc_cnt != stmt->nreads) {
    char *new_text = "";
    char ch;
    int loc = 0;
    while((ch = text[loc]) != '\0') {      
      PlutoAccess *tmp_acc;
      if (wacc_cnt != stmt->nwrites)
        tmp_acc = stmt->writes[wacc_cnt];
      else if (racc_cnt != stmt->nreads) 
        tmp_acc = stmt->reads[racc_cnt];
      else
        tmp_acc = NULL;
      
      if (tmp_acc != NULL) {
        char *tmp_acc_name;
        tmp_acc_name = tmp_acc->name;
        char substr[strlen(tmp_acc_name) + 1];
        memcpy(substr, text + loc, PLMIN(strlen(tmp_acc_name), strlen(text) - loc)* sizeof(char));
        substr[strlen(tmp_acc_name)] = '\0';
        if (!strcmp(substr, tmp_acc_name) && text[loc + strlen(tmp_acc_name)] == '[') {
          new_text = concat(new_text, vsa->acc_var_map[tmp_acc->sym_id]->var_ref);
          loc += strlen(tmp_acc_name);
          // skip the brackets
          int dim = tmp_acc->mat->nrows;
          int dim_cnt = 0;
          while((ch = text[loc]) != '\0') {
            if (ch == ']')
              dim_cnt += 1;
            loc++;
            if (dim_cnt == dim)
              break;
          }
          if (wacc_cnt != stmt->nwrites)
            wacc_cnt++;
          else if (racc_cnt != stmt->nreads)
            racc_cnt++;
        } else {
          char ch_str[2];
          ch_str[0] = ch;
          ch_str[1] = '\0';
          new_text = concat(new_text, ch_str);
          loc++;
        }
      } else {
        char ch_str[2];
        ch_str[0] = ch;
        ch_str[1] = '\0';
        new_text = concat(new_text, ch_str);
        loc++;
      }

    }
    if (text)
      free(text);
    text = strdup(new_text);   
    free(new_text);
  }

  char *var_name = vsa->acc_var_map[stmt->writes[0]->sym_id]->var_name;
  URE *stmt_URE = NULL;
  bool new_URE = true;
  // check if this variable is already defined
  for (int i = 0; i < vsa->URE_num; i++) {
    if (!strcmp(vsa->UREs[i]->name, var_name)) {
      stmt_URE = vsa->UREs[i];
      new_URE = false;
      break;
    }
  }
  if (stmt_URE) {
    stmt_URE->wrap_level++;
    stmt_URE->select_cond = realloc(stmt_URE->select_cond, stmt_URE->wrap_level * sizeof(char *));
    stmt_URE->select_LHS = realloc(stmt_URE->select_LHS, stmt_URE->wrap_level * sizeof(char *));
    stmt_URE->select_RHS = realloc(stmt_URE->select_RHS, stmt_URE->wrap_level * sizeof(char *));
    stmt_URE->select_cond[stmt_URE->wrap_level - 1] = strdup(domain_str);
    stmt_URE->select_RHS[stmt_URE->wrap_level - 1] = strdup(vsa->acc_var_map[stmt->writes[0]->sym_id]->var_ref);
  } else {
    stmt_URE = (URE *)malloc(sizeof(URE));
    URE_init(stmt_URE);
    stmt_URE->id = vsa->URE_num;
    char **URE_names = get_vsa_URE_names(vsa->UREs, vsa->URE_num);
    stmt_URE->name = create_URE_name(URE_names, vsa->URE_num, var_name);
    stmt_URE->LHS = strdup(vsa->acc_var_map[stmt->writes[0]->sym_id]->var_ref);
    stmt_URE->wrap_level = 1;
    stmt_URE->select_cond = realloc(stmt_URE->select_cond, stmt_URE->wrap_level * sizeof(char *));
    stmt_URE->select_LHS = realloc(stmt_URE->select_LHS, stmt_URE->wrap_level * sizeof(char *));
    stmt_URE->select_RHS = realloc(stmt_URE->select_RHS, stmt_URE->wrap_level * sizeof(char *));
    stmt_URE->select_cond[stmt_URE->wrap_level - 1] = strdup(domain_str);
    stmt_URE->select_RHS[stmt_URE->wrap_level - 1] = strdup(stmt_URE->LHS);

    for (int i = 0; i < vsa->URE_num; i++) {
      free(URE_names[i]);
    }
    free(URE_names);
  }

  char *new_text = "";
  char ch;
  int loc = 0;
  while((ch = text[loc]) != ';') {
    if (ch == '=') {
      while(text[++loc] == ' ') {
        
      }
      while((ch = text[loc]) != ';') {
        char tmp[2];
        tmp[0] = ch;
        tmp[1] = '\0';
        new_text = concat(new_text, tmp);          

        loc++;
      }
    } else {
      loc++;
    }
  }

  stmt_URE->select_LHS[stmt_URE->wrap_level - 1] = strdup(new_text);
  char *str_tmp = create_URE_text(stmt_URE);
  free(stmt_URE->text);
  stmt_URE->text = str_tmp;

  if (new_URE)
    vsa->UREs = URE_add(vsa->UREs, &vsa->URE_num, stmt_URE);

  free(text);
  free(new_text);
  free(domain_str);
}

/* 
 * Replace the array access with variable references
 */
//void stmt_to_UREs(Stmt *stmt, PlutoProg *prog, VSA *vsa) {
//  char *domain_str = create_stmt_domain_str(stmt, prog, vsa);
//
//  int wacc_cnt = 0;
//  int racc_cnt = 0;
//  char *text = strdup(stmt->text);
//
//  // PET will assign out-of-order IDs to access functions in the statement
//  // In order to parse all references, we will iteratively run multiple 
//  // times untill we substitute all references in the original statement
//  while(wacc_cnt != stmt->nwrites || racc_cnt != stmt->nreads) {
//    char *new_text = "";
//    char ch;
//    int loc = 0;
//    while((ch = text[loc]) != '\0') {      
//      PlutoAccess *tmp_acc;
//      if (wacc_cnt != stmt->nwrites)
//        tmp_acc = stmt->writes[wacc_cnt];
//      else if (racc_cnt != stmt->nreads) 
//        tmp_acc = stmt->reads[racc_cnt];
//      else
//        tmp_acc = NULL;
//      
//      if (tmp_acc != NULL) {
//        char *tmp_acc_name;
//        tmp_acc_name = tmp_acc->name;
//        char substr[strlen(tmp_acc_name) + 1];
//        memcpy(substr, text + loc, strlen(tmp_acc_name) * sizeof(char));
//        substr[strlen(tmp_acc_name)] = '\0';
//        if (!strcmp(substr, tmp_acc_name) && text[loc + strlen(tmp_acc_name)] == '[') {
//          new_text = concat(new_text, vsa->acc_var_map[tmp_acc->sym_id]->var_ref);
//          loc += strlen(tmp_acc_name);
//          // skip the brackets
//          int dim = tmp_acc->mat->nrows;
//          int dim_cnt = 0;
//          while((ch = text[loc]) != '\0') {
//            if (ch == ']')
//              dim_cnt += 1;
//            loc++;
//            if (dim_cnt == dim)
//              break;
//          }
//          if (wacc_cnt != stmt->nwrites)
//            wacc_cnt++;
//          else if (racc_cnt != stmt->nreads)
//            racc_cnt++;
//        } else {
//          char ch_str[2];
//          ch_str[0] = ch;
//          ch_str[1] = '\0';
//          new_text = concat(new_text, ch_str);
//          loc++;
//        }
//      } else {
//        char ch_str[2];
//        ch_str[0] = ch;
//        ch_str[1] = '\0';
//        new_text = concat(new_text, ch_str);
//        loc++;
//      }
//
//    }
//    text = strdup(new_text);    
//  }
//
//  // insert the select domain
//  char *new_text = "";
//  char ch;
//  int loc = 0;
//  while((ch = text[loc]) != '\0') {
//    if (ch == ';') {
//      new_text = concat(new_text, ")");
//    }
//    char tmp[2];
//    tmp[0] = ch;
//    tmp[1] = '\0';
//    new_text = concat(new_text, tmp);
//    if (ch == '=') {
//      /* Add the select condition */
//      new_text = concat(new_text, " select(");
//      new_text = concat(new_text, domain_str);
//      new_text = concat(new_text, ",");
//    }
//    loc++;
//  }
//
//#ifdef PSA_URE_DEBUG  
////  fprintf(stdout, "[Debug] %s\n", new_text);
//#endif  
//
//  char *var_name = vsa->acc_var_map[stmt->writes[0]->sym_id]->var_name;
//  URE *stmt_URE = (URE *)malloc(sizeof(URE));
//  stmt_URE->text = strdup(new_text);
//  stmt_URE->name = create_URE_name(get_vsa_URE_names(vsa->UREs, vsa->URE_num), vsa->URE_num, var_name);
//  stmt_URE->id = vsa->URE_num;
//  //*URE_num = *URE_num + 1;
//  //UREs[0] = stmt_URE;
//  vsa->UREs = URE_add(vsa->UREs, &vsa->URE_num, stmt_URE);
//
////  return UREs;
//}

/*
 * This function parses iteration information
 * The fields to be fulfilled:
 * - iter_name
 * - lb
 * - ub
 * - stride
 * - type: P: array partitioning, T: time, S: space
 */
void vsa_t2s_meta_iter_extract(PlutoProg *prog, VSA *vsa) {
  // initialize the meta iterators
  Iter **t2s_meta_iters = (Iter **)malloc(sizeof(Iter *) * vsa->t2s_iter_num);
//  for (int i = 0; i < vsa->t2s_iter_num; i++) {
//    t2s_meta_iters[i] = (Iter *)malloc(sizeof(Iter));
//  }

  // parse the AST tree to update the iters
  for (int i = 0; i < vsa->t2s_iter_num; i++) {
    Iter **tmp_iters = (Iter **)malloc(sizeof(Iter *) * 1);
    tmp_iters[0] = (Iter *)malloc(sizeof(Iter));

//    // Build the anchor statement
//    // The anchor statement is with maximum dimension, and the iteration
//    // domain is the union of all the statements
//    Stmt *anchor_stmt = get_new_anchor_stmt(prog->stmts, prog->nstmts);
//
//    // Generate a temporary program
//    PlutoProg *print_prog = pluto_prog_alloc();
//    for (int i = 0; i < prog->npar; i++) {
//      pluto_prog_add_param(print_prog, prog->params[i], print_prog->npar);
//    }
//    for (int i = 0; i < anchor_stmt->dim; i++) {
//      pluto_prog_add_hyperplane(print_prog, 0, H_UNKNOWN, PSA_H_UNKNOWN);
//    }
//    print_prog->stmts = (Stmt **)malloc(sizeof(Stmt *));
//    print_prog->nstmts = 1;
//    print_prog->stmts[0] = anchor_stmt;
    PlutoProg *print_prog = pluto_prog_dup(prog);

    // permute the iterators to outermost
    for (int j = i; j >= 1; j--) {
      pluto_interchange(print_prog, j, j - 1);
    }
 
//    // debug
//    char code_name[10];
//    sprintf(code_name, "debug%d", i);
//    pluto_print_program(print_prog, "kernel.c", code_name);

    // generate temporary CLooG file
    char *cloog_file_name = ".t2s.tmp.cloog";
    FILE *cloog_fp;
    cloog_fp = fopen(cloog_file_name, "w+");
    if (!cloog_fp) {
      fprintf(stderr, "[PSA] Can't open .cloog file: '%s'\n", cloog_file_name);
      free(cloog_file_name);
      return 0;
    }
    pluto_gen_cloog_file(cloog_fp, print_prog);
    rewind(cloog_fp);
    
    // build the clast AST tree
    struct clast_stmt *root;
    CloogOptions *cloogOptions;
    CloogState *state;
  
    state = cloog_state_malloc();
    cloogOptions = cloog_options_malloc(state);
  
    root = psa_create_cloog_ast_tree(print_prog, print_prog->num_hyperplanes, 1, cloog_fp, &cloogOptions);
    fclose(cloog_fp);
    
    int iter_cnt = 0;
    clast_parse_t2s_meta_iters(tmp_iters, 1, &iter_cnt, root, cloogOptions);

    cloog_clast_free(root);
    cloog_options_free(cloogOptions);
    cloog_state_free(state);

    t2s_meta_iters[i] = tmp_iters[0];
    char iter_name[20];
    sprintf(iter_name, "t%d", i + 1);
    free(t2s_meta_iters[i]->iter_name);
    t2s_meta_iters[i]->iter_name = strdup(iter_name);
    free(tmp_iters);

    pluto_prog_free(print_prog);
  }

  for (int i = 0; i < vsa->t2s_iter_num; i++) {
    if (i < vsa->array_part_band_width)
      t2s_meta_iters[i]->type = 'A';
#ifdef ASYNC_ARRAY
    else if (i < vsa->array_part_band_width + vsa->space_band_width)
      t2s_meta_iters[i]->type = 'S';
    else
      t2s_meta_iters[i]->type = 'T';
#endif
#ifdef SYNC_ARRAY
    else if (i < vsa->array_part_band_width + vsa->time_band_width)
      t2s_meta_iters[i]->type = 'T';
    else
      t2s_meta_iters[i]->type = 'S';
#endif    
  }

  vsa->t2s_meta_iters = t2s_meta_iters;
}

/*
 * This function parses iteration information from the 
 * CLAST AST tree built by CLooG.
 * The fields to be fulfilled:
 * - iter_name
 * - lb
 * - ub
 * - stride
 * - type: P: array partitioning, T: time, S: space
 */
//void vsa_t2s_meta_iter_extract(PlutoProg *prog, VSA *vsa) {
//  // Build the anchor statement
//  // The anchor statement is with maximum dimension, and the iteration
//  // domain is the union of all the statements
//  Stmt *anchor_stmt = get_new_anchor_stmt(prog->stmts, prog->nstmts);
//
//  // Generate a temporary program
//  PlutoProg *print_prog = pluto_prog_alloc();
//  for (int i = 0; i < prog->npar; i++) {
//    pluto_prog_add_param(print_prog, prog->params[i], print_prog->npar);
//  }
//  for (int i = 0; i < anchor_stmt->dim; i++) {
//    pluto_prog_add_hyperplane(print_prog, 0, H_UNKNOWN, PSA_H_UNKNOWN);
//  }
//  print_prog->stmts = (Stmt **)malloc(sizeof(Stmt *));
//  print_prog->nstmts = 1;
//  print_prog->stmts[0] = anchor_stmt;
//
//  // initialize the meta iterators
//  Iter **t2s_meta_iters = (Iter **)malloc(sizeof(Iter *) * vsa->t2s_iter_num);
//  for (int i = 0; i < vsa->t2s_iter_num; i++) {
//    t2s_meta_iters[i] = (Iter *)malloc(sizeof(Iter));
//  }
//
//  for (int i = 0; i < vsa->t2s_iter_num; i++) {
//    if (i < vsa->array_part_band_width)
//      t2s_meta_iters[i]->type = 'A';
//#ifdef ASYNC_ARRAY
//    else if (i < vsa->array_part_band_width + vsa->space_band_width)
//      t2s_meta_iters[i]->type = 'S';
//    else
//      t2s_meta_iters[i]->type = 'T';
//#endif
//#ifdef SYNC_ARRAY
//    else if (i < vsa->array_part_band_width + vsa->time_band_width)
//      t2s_meta_iters[i]->type = 'T';
//    else
//      t2s_meta_iters[i]->type = 'S';
//#endif    
//  }
//
//  // generate temporary CLooG file
//  char *cloog_file_name = ".t2s.tmp.cloog";
//  FILE *cloog_fp;
//  cloog_fp = fopen(cloog_file_name, "w+");
//  if (!cloog_fp) {
//    fprintf(stderr, "[PSA] Can't open .cloog file: '%s'\n", cloog_file_name);
//    free(cloog_file_name);
//    return 0;
//  }
//  pluto_gen_cloog_file(cloog_fp, print_prog);
//  rewind(cloog_fp);
//  
//  // build the clast AST tree
//  struct clast_stmt *root;
//  CloogOptions *cloogOptions;
//  CloogState *state;
//
//  state = cloog_state_malloc();
//  cloogOptions = cloog_options_malloc(state);
//
//  root = psa_create_cloog_ast_tree(print_prog, print_prog->num_hyperplanes, 1, cloog_fp, &cloogOptions);
//  fclose(cloog_fp);
//
//#ifdef PSA_URE_DEBUG
//  FILE *file_debug;
//  file_debug = fopen(".ure_debug", "w");
//  clast_pprint(file_debug, root, 0, cloogOptions);
//  fclose(file_debug);
//#endif
//
//  // parse the AST tree to update the iters
//  int iter_cnt = 0;
//  clast_parse_t2s_meta_iters(t2s_meta_iters, vsa->t2s_iter_num, &iter_cnt, root, cloogOptions);
//
//  cloog_clast_free(root);
//  cloog_options_free(cloogOptions);
//  cloog_state_free(state);
//
//  vsa->t2s_meta_iters = t2s_meta_iters;
//
//  pluto_prog_free(print_prog);
//}

/*
 * This funtion parse iterator information from the CLAST AST tree.
 * It will stop after parsting iter_num iterators.
 */
void clast_parse_t2s_meta_iters(Iter **iters, int iter_num, int *iter_cnt, struct clast_stmt *root, CloogOptions *options) {
  iter_parse_stmt_list(options, iters, iter_num, iter_cnt, root);
}

void iter_parse_stmt_list(struct cloogoptions *options, Iter **iters, int iter_num, int *iter_cnt, struct clast_stmt *s) {
  if (*iter_cnt < iter_num) {
    for ( ; s; s = s->next) {
      if (CLAST_STMT_IS_A(s, stmt_root)) 
        continue;
      if (CLAST_STMT_IS_A(s, stmt_ass))
        iter_parse_assignment(options, iters, iter_num, iter_cnt, (struct clast_assignment *)s);
      else if (CLAST_STMT_IS_A(s, stmt_user)) 
        iter_parse_user_stmt(options, iters, iter_num, iter_cnt, (struct clast_user_stmt *)s);
      else if (CLAST_STMT_IS_A(s, stmt_for))
        iter_parse_for(options, iters, iter_num, iter_cnt, (struct clast_for *)s);
      else if (CLAST_STMT_IS_A(s, stmt_guard))
        iter_parse_guard(options, iters, iter_num, iter_cnt, (struct clast_guard *)s);
      else if (CLAST_STMT_IS_A(s, stmt_block))
        iter_parse_stmt_list(options, iters, iter_num, iter_cnt, ((struct clast_block *)s)->body);
      else {
        assert(0);
      }
    }
  }
}

void iter_parse_assignment(struct cloogoptions *options, Iter **iters, int iter_num, int *iter_cnt, struct clast_assignment *a) {
  return;
}

void iter_parse_user_stmt(struct cloogoptions *options, Iter **iters, int iter_num, int *iter_cnt, struct clast_user_stmt *a) {
  return;
}

void iter_parse_for(struct cloogoptions *options, Iter **iters, int iter_num, int *iter_cnt, struct clast_for *f) {
  if (*iter_cnt < iter_num) {
    iters[*iter_cnt]->iter_name = strdup(f->iterator);
    if (f->LB) {    
      char *str_tmp = clast_expr_to_str(options, f->LB);
      iters[*iter_cnt]->lb = strdup(str_tmp);
      free(str_tmp);
    } 
    if (f->UB) {
      char *str_tmp = clast_expr_to_str(options, f->UB); 
      iters[*iter_cnt]->ub = strdup(str_tmp);
      free(str_tmp);
    }
    iters[*iter_cnt]->stride = clast_int_to_str(f->stride);
    *iter_cnt = *iter_cnt + 1;
    iter_parse_stmt_list(options, iters, iter_num, iter_cnt, f->body);
  } else {
    return;
  }
}

void iter_parse_guard(struct cloogoptions *options, Iter **iters, int iter_num, int *iter_cnt, struct clast_guard *g) {
  return;
}

char *clast_expr_to_str(struct cloogoptions *i, struct clast_expr *e) {
  if (!e)
    return;
  char *str = "";
  char *tmp_str = NULL;
  switch (e->type) {
    case clast_expr_name:
      tmp_str = clast_name_to_str((struct clast_name *)e);
      str = concat(str, tmp_str);
      free(tmp_str);
      break;
    case clast_expr_term:
      tmp_str = clast_term_to_str(i, (struct clast_term *)e);
      str = concat(str, tmp_str);
      free(tmp_str);
      break;
    case clast_expr_red:
      tmp_str = clast_reduction_to_str(i, (struct clast_reduction *)e);
      str = concat(str, tmp_str);
      free(tmp_str);
      break;
    case clast_expr_bin:
      tmp_str = clast_binary_to_str(i, (struct clast_binary *)e);
      str = concat(str, tmp_str);
      free(tmp_str);
      break;
    default:
      assert(0);
  }
  return str;
}

char *clast_name_to_str(struct clast_name *n) {
  char *str = "";
  char str_tmp[100];
  sprintf(str_tmp, "%s", n->name);
  str = concat(str, str_tmp);
  return str;
}

char *clast_term_to_str(struct cloogoptions *i, struct clast_term *t) {
  char *str = "";
  if (t->var) {
    int group = t->var->type == clast_expr_red &&
      ((struct clast_reduction*) t->var)->n > 1;
    if (cloog_int_is_one(t->val))
      ;
    else if (cloog_int_is_neg_one(t->val))
      str = concat(str, "-");
    else {
      char *str_tmp = clast_int_to_str(t->val);
      str = concat(str, str_tmp);
      free(str_tmp);
      str = concat(str, "*");
    }
    if (group)
      str = concat(str, "(");
    char *str_tmp = clast_expr_to_str(i, t->var);
    str = concat(str, str_tmp);
    free(str_tmp);
    if (group)
      str = concat(str, ")");
  } else {
    char *str_tmp = clast_int_to_str(t->val);
    str = concat(str, str_tmp);
    free(str_tmp);
  }
  return str;
}

char *clast_reduction_to_str(struct cloogoptions *i, struct clast_reduction *r) {
  char *str = "";
  char *str_tmp;
  switch (r->type) {
    case clast_red_sum:      
      str_tmp = clast_sum_to_str(i, r);
      str = concat(str, str_tmp);
      break;
    case clast_red_min:
    case clast_red_max:
      if (r->n == 1) {
        str_tmp = clast_expr_to_str(i, r->elts[0]);
        str = concat(str, str_tmp);
        free(str_tmp);
        break;
      }
      if (i->language == CLOOG_LANGUAGE_FORTRAN) {
        str_tmp = clast_minmax_f_to_str(i, r);
        str = concat(str, str_tmp);
        free(str_tmp);
      } else {
        str_tmp = clast_minmax_c_to_str(i, r);
        str = concat(str, str_tmp);        
        free(str_tmp);
      }
      break;
    default:
      assert(0);
  }
  return str;
}

char *clast_sum_to_str(struct cloogoptions *opt, struct clast_reduction *r) {
  char *str = "";
  int i;
  struct clast_term *t;

  assert(r->n >= 1);
  assert(r->elts[0]->type == clast_expr_term);
  t = (struct clast_term *) r->elts[0];
  char *str_tmp = clast_term_to_str(opt, t);
  str = concat(str, str_tmp);
  free(str_tmp);

  for (i = 1; i < r->n; ++i) {
    assert(r->elts[i]->type == clast_expr_term);
    t = (struct clast_term *)r->elts[i];
    if (cloog_int_is_pos(t->val)) 
      str = concat(str, "+");
    char *str_tmp = clast_term_to_str(opt, t);
    str = concat(str, str_tmp);
    free(str_tmp);
  }

  return str;
}

char *clast_minmax_f_to_str(struct cloogoptions *info, struct clast_reduction *r) {
  assert(0);
  return NULL;
}

char *clast_minmax_c_to_str(struct cloogoptions *info, struct clast_reduction *r) {
  char *str = "";
  int i;
  for (i = 1; i < r->n; ++i) {
    str = concat(str, r->type == clast_red_max ? "max(" : "min(");
  }
  if (r->n > 0) {
    str = concat(str, clast_expr_to_str(info, r->elts[0]));
  }
  for (i = 1; i < r->n; ++i) {
    str = concat(str, ",");
    str = concat(str, clast_expr_to_str(info, r->elts[i]));
    str = concat(str, ")");
  }
  return str;
}

char *clast_binary_to_str(struct cloogoptions *i, struct clast_binary *b) {
  char *str = "";
  const char *s1 = NULL, *s2 = NULL, *s3 = NULL;
  int group = b->LHS->type == clast_expr_red &&
    ((struct clast_reduction*) b->LHS)->n > 1;
  if (i->language == CLOOG_LANGUAGE_FORTRAN) {

  } else {
    switch (b->type) {
      case clast_bin_fdiv:
        s1 = "floord(", s2 = ",", s3 = ")";
        break;
      case clast_bin_cdiv:
        s1 = "ceild(", s2 = ",", s3 = ")";
        break;
      case clast_bin_div:
        if (group)
          s1 = "(", s2 = ")/", s3 = "";
        else
          s1 = "", s2 = "/", s3 = "";
        break;
      case clast_bin_mod:
        if (group)
          s1 = "(", s2 = ")%", s3 = "";
        else
          s1 = "", s2 = "%", s3 = "";
        break;
    }    
  }
  str = concat(str, s1);
  str = concat(str, clast_expr_to_str(i, b->LHS));
  str = concat(str, s2);
  str = concat(str, clast_int_to_str(b->RHS));
  str = concat(str, s3);

  return str;
}

/* Cloog uses GMP library to handle integers */
char *clast_int_to_str(cloog_int_t t) {
//  char *str = "";
//  char str_tmp[100];
//  sprintf(str_tmp, "%d", t);
//  str = concat(str, str_tmp);
//  return str;
  char *str = "";
  char *s;
  cloog_int_print_gmp_free_t gmp_free;
  s = mpz_get_str(0, 10, t);
  str = concat(str, s);
  mp_get_memory_functions(NULL, NULL, &gmp_free);
  (*gmp_free)(s, strlen(s)+1);

  return str;
}
