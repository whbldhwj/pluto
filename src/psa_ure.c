#include "psa_ure.h"

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
 * This function returns the transformed access function in the form of A(t1, t3).
 */
char *create_new_acc_str(Stmt *stmt, PlutoAccess *acc, PlutoProg *prog, VSA *vsa) {
  // compute the new access function
  int *divs;
  PlutoMatrix *new_acc = pluto_get_new_access_func(stmt, acc->mat, &divs);

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
    if (divs[row] != 1) {
      acc_str = concat(acc_str, "(");
    }
    for (int col = 0; col < new_acc->ncols; col++) {
      if (new_acc->val[row][col] != 0) {
        char exp[20];
        if (col < stmt->trans->nrows) {
          if (new_acc->val[row][col] == 1)
            sprintf(exp, "%s", iters[col]);
          else
            sprintf(exp, "(%d) * %s", new_acc->val[row][col], iters[col]);
        } else if (col < stmt->trans->nrows + npar) {
          if (new_acc->val[row][col] == 1)
            sprintf(exp, "%s", params[col - stmt->trans->nrows]);
          else
            sprintf(exp, "(%d) * %s", new_acc->val[row][col], params[col - stmt->trans->nrows]);
        } else {
          if (new_acc->val[row][col] == 1)
            sprintf(exp, "%d", new_acc->val[row][col]);
          else
            sprintf(exp, "(%d)", new_acc->val[row][col]);
        }
        if (first_exp) {
          acc_str = concat(acc_str, exp);
          first_exp = ~first_exp;
        } else {
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
  acc_str = concat(acc_str, ")");

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
  for (int i = 0; i < prog->nstmts - 1; i++) {
    for (int j = 0; j < prog->nstmts - 1 - i; j++) {
      Stmt *stmt_first = prog->stmts[j];
      Stmt *stmt_second = prog->stmts[j + 1];
      int seq = t2s_compare_stmt_order(stmt_first, stmt_second, band_width);
      if (seq > 0) {
        Stmt *tmp = stmt_first;
        prog->stmts[j] = stmt_second;
        prog->stmts[j + 1] = tmp;
      }
    }
  }

  // scan through each statement
  for (int stmt_id = 0; stmt_id < prog->nstmts; stmt_id++) {
    Stmt *stmt = prog->stmts[stmt_id];
    // UREs for the read access
    // TODO: For now, we assume all read accesses come with RAR, will extend it later
    for (int i = 0; i < stmt->nreads; i++) {
      PlutoAccess *acc = stmt->reads[i];
      if (vsa->acc_var_map[acc->sym_id]->ei == 0) {
        create_RAR_UREs(stmt, acc, prog, vsa);
      }
    }
    // URE for the stmt
    stmt_to_UREs(stmt, prog, vsa);

    // UREs for the write access
    PlutoAccess *acc = stmt->writes[0];
    create_drain_UREs(stmt, acc, prog, vsa);
  }
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
 * This function builds the iteration domain for loop instances that first read the data
 * based on the RAR dependence
 * target_iters = read_iters - (RAR dest_iters);
 * The iteration domain is printed following the Halide syntax
 */ 
char *create_RAR_domain_str(Stmt *stmt, PlutoAccess *acc, PlutoProg *prog, VSA *vsa) {
  PlutoConstraints *new_domain = pluto_get_new_domain(stmt);
  PlutoConstraints *full_domain = pluto_constraints_dup(new_domain);
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
      pluto_constraints_project_out_isl_single(&tdpoly, 0, src_stmt->trans->nrows);
#ifdef PSA_URE_DEBUG
//      fprintf(stdout, "[Debug] RAR poly after projecting out the src iters:\n");
//      pluto_constraints_pretty_print(stdout, tdpoly);
#endif     
      // Subtracting out the iters that read data from other source iters
      new_domain = pluto_constraints_subtract(new_domain, tdpoly);
#ifdef PSA_URE_DEBUG
//      fprintf(stdout, "[Debug] RAR read-in iters:\n");
//      pluto_constraints_pretty_print(stdout, new_domain);
#endif     
      pluto_constraints_free(tdpoly);
    }
  }

  // comparing the constraints of RAR read-in iters and the original iters
  // if there is the same constraint of RAR read-in iters in the original iters, 
  // eliminate that constraints
  int new_id, full_id;
  new_id = 0;
  while(new_id < new_domain->nrows) {
    PlutoConstraints *row1 = pluto_constraints_select_row(new_domain, new_id);
    for (full_id = 0; full_id < full_domain->nrows; full_id++) {
      PlutoConstraints *row2 = pluto_constraints_select_row(full_domain, full_id);
      if (pluto_constraints_are_equal(row1, row2)) {
        pluto_constraints_remove_row(new_domain, new_id);
        break;
      }
    }
    if (full_id == full_domain->nrows) {
      new_id++;
    }
  }
#ifdef PSA_URE_DEBUG
//  fprintf(stdout, "[Debug] RAR read-in iters (opted):\n");
//  pluto_constraints_pretty_print(stdout, new_domain);
#endif      

  // print out the constraints in Halide syntax
  char *const_str = pluto_constraints_to_t2s_format(new_domain, vsa, stmt->trans->nrows, prog->npar, prog->params);

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
      pluto_constraints_project_out_isl_single(&tdpoly, src_stmt->trans->nrows, dest_stmt->trans->nrows);
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
    // comparing the constraints of WAW write-out iters and the original iters
    // if there is the same constraint of WAW write-out iters in the original iters,
    // eliminate that constraints
    int new_id, full_id;
    new_id = 0;
    while(new_id < new_domain->nrows) {
      PlutoConstraints *row1 = pluto_constraints_select_row(new_domain, new_id);
      for (full_id = 0; full_id < full_domain->nrows; full_id++) {
        PlutoConstraints *row2 = pluto_constraints_select_row(full_domain, full_id);
        if (pluto_constraints_are_equal(row1, row2)) {
          pluto_constraints_remove_row(new_domain, new_id);
          break;
        }
      }
      if (full_id == full_domain->nrows) {
        new_id++;
      }
    }
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
  int nrows = cst->nrows;
  int ncols = cst->ncols;
  int iter_num = vsa->t2s_iter_num;
  char **iters = vsa->t2s_iters;
  for (int i = 0; i < nrows; i++) {
    if (i > 0) {
      ret_str = concat(ret_str, " && ");
    }

    bool is_first = true;
    for (int j = 0; j < ncols; j++) {
      if (cst->val[i][j] != 0) {
        char exp[20];
        if (!is_first) {
          ret_str = concat(ret_str, " + ");
        }
        if (j < niter) {
          if (cst->val[i][j] == 1) {
            sprintf(exp, "%s", iters[j]);
            ret_str = concat(ret_str, exp);
          } else if (cst->val[i][j] == -1) {
            sprintf(exp, "-%s", iters[j]);
            ret_str = concat(ret_str, exp);
          } else {
            sprintf(exp, "(%d) * %s", cst->val[i][j], iters[j]);
            ret_str = concat(ret_str, exp);
          }
        } else if (j < niter + nparam) {
          if (cst->val[i][j] == 1) {
            sprintf(exp, "%s", params[j]);
            ret_str = concat(ret_str, exp);
          } else if (cst->val[i][j] == -1) {
            sprintf(exp, "-%s", params[j]);
            ret_str = concat(ret_str, exp);
          } else {
            sprintf(exp, "(%d) * %s", cst->val[i][j], params[j]);
            ret_str = concat(ret_str, exp);
          }         
        } else {
          if (cst->val[i][j] == 1) {
            ret_str = concat(ret_str, "1");
          } else if (cst->val[i][j] == -1) {
            ret_str = concat(ret_str, "-1");
          } else {
            sprintf(exp, "(%d)", cst->val[i][j]);
            ret_str = concat(ret_str, exp);
          }
        }              
        if (is_first) {
          is_first = !is_first;
        }
      }      
    }
    if (cst->is_eq[i] == 1) {
      ret_str = concat(ret_str, " == 0");
    } else {
      ret_str = concat(ret_str, " >= 0");
    }
  }

  return ret_str;
}

void create_RAR_UREs(Stmt *stmt, PlutoAccess *acc, PlutoProg *prog, VSA *vsa) {
  int URE_num = vsa->URE_num;
  // URE **UREs = (URE **)malloc(2 * sizeof(URE *));

  // update clause 
  URE* update_URE = (URE *)malloc(sizeof(URE));
  char *var_name = vsa->acc_var_map[acc->sym_id]->var_name;
  char *var_ref = vsa->acc_var_map[acc->sym_id]->var_ref;
  IterExp **var_iters = vsa->acc_var_map[acc->sym_id]->var_iters;
  
  IterExp **var_iters_RHS = (IterExp **)malloc(vsa->t2s_iter_num * sizeof(IterExp *));
  for (int i = 0; i < prog->ndeps; i++) {
    Dep *dep = prog->deps[i];
    if (IS_RAR(dep->type)) {
      if (dep->src_acc == acc || dep->dest_acc == acc) {
        for (int iter_id = 0; iter_id < vsa->t2s_iter_num; iter_id++) {
          var_iters_RHS[iter_id] = (IterExp *)malloc(sizeof(IterExp));
          int diff;
          if (dep->disvec[iter_id] == DEP_DIS_MINUS_ONE)
            diff = -1;
          else if (dep->disvec[iter_id] == DEP_DIS_ZERO)
            diff = 0;
          else if (dep->disvec[iter_id] == DEP_DIS_PLUS_ONE)
            diff = 1;

          var_iters_RHS[iter_id]->iter_name = strdup(var_iters[iter_id]->iter_name);
          var_iters_RHS[iter_id]->iter_offset = var_iters[iter_id]->iter_offset - diff;
        }
      }
    }
  }

  char var_ref_RHS[50];
  sprintf(var_ref_RHS, "%s(%s)", var_name, get_iter_str(var_iters_RHS, vsa->t2s_iter_num));
  update_URE->id = vsa->URE_num;
  //update_URE->name = strdup(var_name);  
  update_URE->name = create_URE_name(get_vsa_URE_names(vsa->UREs, vsa->URE_num), vsa->URE_num, var_name);
  // generate the text
  char *text = "";
  text = concat(text, var_ref);
  text = concat(text, " = ");
  text = concat(text, var_ref_RHS);
  text = concat(text, ";");
  update_URE->text = strdup(text);

  //UREs[0] = update_URE;
  //*URE_num = *URE_num + 1;
  vsa->UREs = URE_add(vsa->UREs, &vsa->URE_num, update_URE);

  // init clause
  URE *init_URE = (URE *)malloc(sizeof(URE));
  init_URE->id = vsa->URE_num;
//  char *URE_name = strdup(var_name);
//  URE_name = concat(URE_name, ".update(0)");
//  init_URE->name = strdup(URE_name);
  init_URE->name = create_URE_name(get_vsa_URE_names(vsa->UREs, vsa->URE_num), vsa->URE_num, var_name);
  // generate the text
  // create the new access string e.g., A(t1, t3)
  char *new_acc_str = create_new_acc_str(stmt, acc, prog, vsa);
  // create the init domain string 
  // e.g., t2 == 0 && t1 >= 0 && t1 <= 3 && t3 >= 0 && t3 <= 3
  // compute the RAR init domain  
  char *domain_str = create_RAR_domain_str(stmt, acc, prog, vsa);
  //char *domain_str = "";

  text = "";
  text = concat(text, var_ref);
  if (strcmp(domain_str, "")) {
    text = concat(text, " = select(");
    text = concat(text, domain_str);
    text = concat(text, ", ");  
    text = concat(text, new_acc_str);
    text = concat(text, ");");
  } else {
    text = concat(text, " = ");
    text = concat(text, new_acc_str);
    text = concat(text, ";");
  }
  init_URE->text = strdup(text);

  //UREs[1] = init_URE;
  //*URE_num = *URE_num + 1;
  vsa->UREs = URE_add(vsa->UREs, &vsa->URE_num, init_URE);

  //return UREs;
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

/* 
 * Replace the array access with variable references
 */
void stmt_to_UREs(Stmt *stmt, PlutoProg *prog, VSA *vsa) {
  char *new_text = "";
  int wacc_cnt = 0;
  int racc_cnt = 0;
  char *text = stmt->text;
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
      memcpy(substr, text + loc, strlen(tmp_acc_name) * sizeof(char));
      substr[strlen(tmp_acc_name)] = '\0';
      if (!strcmp(substr, tmp_acc_name)) {
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

#ifdef PSA_URE_DEBUG  
  fprintf(stdout, "[Debug] %s\n", new_text);
#endif  

  char *var_name = vsa->acc_var_map[stmt->writes[0]->sym_id]->var_name;
  URE *stmt_URE = (URE *)malloc(sizeof(URE));
  stmt_URE->text = strdup(new_text);
  stmt_URE->name = create_URE_name(get_vsa_URE_names(vsa->UREs, vsa->URE_num), vsa->URE_num, var_name);
  stmt_URE->id = vsa->URE_num;
  //*URE_num = *URE_num + 1;
  //UREs[0] = stmt_URE;
  vsa->UREs = URE_add(vsa->UREs, &vsa->URE_num, stmt_URE);

//  return UREs;
}


