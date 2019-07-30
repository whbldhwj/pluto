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
      if (vsa->acc_var_map[acc->sym_id]->ei == 1) {
        int num_tmp;
        URE **UREs_tmp = create_RAR_UREs(acc, prog, vsa, &num_tmp);        
        UREs = URE_append(UREs, &URE_num, UREs_tmp, num_tmp);
      }
    }
    // URE for the stmt
    int num_tmp;
    URE **UREs_tmp = stmt_to_UREs(stmt, prog, vsa, &num_tmp);
    UREs = URE_append(UREs, &URE_num, UREs_tmp, num_tmp);

    // UREs for the write access
    PlutoAccess *acc = stmt->writes[0];
    UREs_tmp = create_drain_UREs(acc, prog, vsa, &num_tmp);
    UREs = URE_append(UREs, &URE_num, UREs_tmp, num_tmp);
  }

  vsa->URE_num = URE_num;
  vsa->UREs = UREs;
}

URE **create_RAR_UREs(PlutoAccess *acc, PlutoProg *prog, VSA *vsa, int *URE_num) {
  *URE_num = 0;
  return NULL;
}

URE **create_drain_UREs(PlutoAccess *acc, PlutoProg *prog, VSA *vsa, int *URE_num) {
  *URE_num = 0;
  return NULL;
}

URE **stmt_to_UREs(Stmt *stmt, PlutoProg *prog, VSA *vsa, int *URE_num) {
  *URE_num = 0;
  return NULL;
}

URE **URE_append(URE **list1, int *num1, URE **list2, int num2) {
  list1 = realloc(list1, (*num1 + num2) * sizeof(URE*));
  for (int i = *num1; i < *num1 + num2; i++) {
    list1[i] = list2[i - *num1];
  }
  *num1 = *num1 + num2;
  return list1;
}
