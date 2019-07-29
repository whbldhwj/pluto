#include "psa_ure.h"

//void update_ivar_ref(int acc_id, struct stmt_access_var_pair **acc_var_map, VSA *vsa) {
//  int i;
//  char *var_ref = acc_var_map[acc_id]->var_ref;
//  char *var_name = acc_var_map[acc_id]->var_name;
//  if (var_ref)
//    return;
//
//  PlutoAccess *acc = acc_var_map[acc_id]->acc;
//  for (i = 0; i < prog->ndeps; i++) {
//    Dep *dep = prog->deps[i];
//    if (IS_RAW(dep->type)) {
//      if (dep->src_acc == acc || dep->dest_acc == acc)
//        break;
//    }
//  }
//  // This the first access function in the current CC
//  if (i == prog->ndeps) {
//    int niters = vsa->t2s_iter_num;
//    char **iters = vsa->t2s_iters;
//    char *iters = "t1";
//    for (int j = 1; j < niters; j++) {
//      char iter_tmp[6];
//      sprintf(iter_tmp, ", t%d", j + 1);
//      iters = concat(iters, iter_tmp);
//    }
//    char var_ref[50];
//    sprintf(var_ref, "%s(%s)", var_name, iters);
//  } else {
//    
//  }
//}

/* This function generates UREs from the program, the fields in VSA are:
 * - UREs (URE statements)
 * - URE_num (total number of UREs)
 * - URE_names (name of URE)
 * Algorithm: 
 * First, we update the var reference for each access function.
 * For the external variable, we will assign it to (i,j,k)
 * For the intermediate variable, we will start with one of the access function in the CC
 * assign it to (i,j,k). Next, we will assign the other access functions based on 
 * We proceed statement by statement. If the access is an intermediate access,
 * The read access reference + dep_vec = write access reference
 * Second, we will add the necessary statements.
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
//  // Update the variable reference
//  struct stmt_access_var_pair **acc_var_map = vsa->acc_var_map;
//  Graph *adg = prog->adg;
//
//  int iter_num = vsa->t2s_iter_num;
//  char **iters = vsa->t2s_iters;
//  int total_accs = adg->nVertices;
//
//  for (int cc_id = 0; cc_id < adg->num_ccs; cc_id++) {
//    CC cc = adg->ccs[cc_id];
//    if (cc.size == 1) {
//      // external variable, skipped
//    } else {
//      // intermediate variable, needs to be updated
//      int acc_num = 0;
//      int acc_id = 0;
//      update_ivar_ref(0, &stmt_access_var_pair);
//      PlutoAccess *acc = stmt_access_var_pair[cc.vertices[0]];
//      if stmt_access_var_pair[]
//
//      while (acc_num < cc.size) {
//        PlutoAccess *acc = stmt_access_var_pair[cc.vertices[acc_id]];
//        // check if acc is associated with
//      }
//    }
//  }
}
