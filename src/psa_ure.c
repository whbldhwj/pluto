#include "psa_ure.h"

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
        int num_tmp;
        URE **UREs_tmp = create_RAR_UREs(acc, prog, vsa, &num_tmp);        
        UREs = URE_append(UREs, &URE_num, UREs_tmp, num_tmp);

        // update VSA
        vsa->URE_num = URE_num;
        vsa->UREs = UREs;
      }
    }
    // URE for the stmt
    int num_tmp;
    URE **UREs_tmp = stmt_to_UREs(stmt, prog, vsa, &num_tmp);
    UREs = URE_append(UREs, &URE_num, UREs_tmp, num_tmp);

    // update VSA
    vsa->URE_num = URE_num;
    vsa->UREs = UREs;

    // UREs for the write access
    PlutoAccess *acc = stmt->writes[0];
    UREs_tmp = create_drain_UREs(acc, prog, vsa, &num_tmp);
    UREs = URE_append(UREs, &URE_num, UREs_tmp, num_tmp);

    // update VSA
    vsa->URE_num = URE_num;
    vsa->UREs = UREs;
  }
}

char *create_URE_name(char **URE_names, int URE_num, char *var_name) {  
  int update_level = 0;  
  for (int i = 0; i < URE_num; i++) {
    char *cur_name = URE_names[i];
    if (strlen(cur_name) >= strlen(var_name)) {
      char cur_name_prefix[strlen(var_name) + 1];
      memcpy(cur_name_prefix, cur_name, strlen(var_name) * sizeof(char));
      cur_name_prefix[strlen(var_name)] = '\0';
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

URE **create_RAR_UREs(PlutoAccess *acc, PlutoProg *prog, VSA *vsa, int *URE_num) {
  *URE_num = 0;
  URE **UREs = (URE **)malloc(2 * sizeof(URE *));

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
  update_URE->id = *URE_num;
  update_URE->name = strdup(var_name);  
  // generate the text
  char *text = "";
  text = concat(text, var_ref);
  text = concat(text, " = ");
  text = concat(text, var_ref_RHS);
  text = concat(text, ";");
  update_URE->text = strdup(text);

  UREs[0] = update_URE;
  *URE_num = *URE_num + 1;

  // init clause
  //URE init_URE;

  return UREs;
}

URE **create_drain_UREs(PlutoAccess *acc, PlutoProg *prog, VSA *vsa, int *URE_num) {
  *URE_num = 0;
  return NULL;
}

/* 
 * Replace the array access with variable references
 */
URE **stmt_to_UREs(Stmt *stmt, PlutoProg *prog, VSA *vsa, int *URE_num) {
  *URE_num = 0;
  URE **UREs = (URE **)malloc(sizeof(URE *));

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
  stmt_URE->id = *URE_num;
  *URE_num = *URE_num + 1;
  UREs[0] = stmt_URE;

  return UREs;
}

URE **URE_append(URE **list1, int *num1, URE **list2, int num2) {
  list1 = realloc(list1, (*num1 + num2) * sizeof(URE*));
  for (int i = *num1; i < *num1 + num2; i++) {
    list1[i] = list2[i - *num1];
  }
  *num1 = *num1 + num2;
  return list1;
}
