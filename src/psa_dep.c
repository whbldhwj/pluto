/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 * 
 * Author: Jie Wang
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <math.h>

#include "pluto.h"
#include "math_support.h"
#include "constraints.h"
#include "psa_dep.h"
#include "psa_helpers.h"
#include "program.h"

#include "osl/macros.h"
#include "osl/scop.h"
#include "osl/body.h"
#include "osl/relation_list.h"
#include "osl/extensions/arrays.h"
#include "osl/extensions/dependence.h"
#include "osl/extensions/loop.h"
#include "osl/extensions/pluto_unroll.h"
#include "osl/extensions/scatnames.h"

#include "cloog/cloog.h"

#include "candl/candl.h"
#include "candl/scop.h"
#include "candl/options.h"
#include "candl/dependence.h"

#include <isl/id.h>
#include <isl/map.h>
#include <isl/mat.h>
#include <isl/set.h>
#include <isl/space.h>
#include <isl/flow.h>
#include <isl/union_map.h>
#include <isl/val.h>

#include "pet.h"

/*
 * A program can be transformed to systolic array if and only if it satisfies the following constraints:
 * - uniform dependency
 * - single fully permuatable band
 * - no further child band under the common permutable band
 */
bool systolic_array_legal_checker(PlutoProg *prog) {
  /* single fully permutable band */
  unsigned nbands = 0;
  int err_type = 0;
  unsigned nloops = 0;
  Ploop **loops;

  Band **bands = pluto_get_outermost_permutable_bands(prog, &nbands);
  if (nbands != 1) {
    err_type = 1;
  }
  if (!err_type) {
    /* no child band under the current band */
    loops = pluto_get_loops_under(
        bands[0]->loop->stmts, bands[0]->loop->nstmts,
        bands[0]->loop->depth, prog, &nloops);
    if (nloops > bands[0]->width) {
      err_type = 2;
    }
  }
  if (!err_type) {
    /* uniform dependency */
    bool is_uniform = systolic_array_dep_checker(prog);
    if (!is_uniform) {
      err_type = 3;
    }
  }

  pluto_bands_free(bands, nbands);
  for (int i = 0; i < nloops; i++) {
    pluto_loop_free(loops[i]);
  }
  free(loops);

  if (err_type == 1) {
    fprintf(stdout, "[PSA] More than one outer permutable band detected.\n");
    return 0;   
  } else if (err_type == 2){
    fprintf(stdout, "[PSA] More than one child permutable band detected.\n");
    return 0;
  } else if (err_type == 3) {
    fprintf(stdout, "[PSA] Non-uniform dependency detectec.\n");
    return 0;
  } else {
    return 1;
  }
}

/*
 * Detecting non-uniform dependences and exit the program if any non-uniform
 * dependences is detected. 
 */
bool systolic_array_dep_checker(PlutoProg *prog) {
  int ndeps = prog->ndeps;
  Dep **deps = prog->deps;

  /* Check the uniformity of dependences */  
  for (int i = 0; i < ndeps; i++) {
    Dep *dep = deps[i];
    Stmt *src_stmt = prog->stmts[dep->src];
    Stmt *dest_stmt = prog->stmts[dep->dest];
    PlutoConstraints *tdpoly = pluto_get_transformed_dpoly(dep, src_stmt, dest_stmt);
    pluto_constraints_simplify(tdpoly);

    assert(tdpoly->ncols == src_stmt->trans->nrows + dest_stmt->trans->nrows + prog->npar + 1);
    // We look at the dependence polyhedral at rows where both src_stmt and dest_stmt's coeff is non-zero
    // Meanwhile:
    // 1. Src iter and dest iters should be at the same location
    // 2. Src iter and dest iters should have the opposite coeffcicients
    // 3. The coefficients of global paramters should be zero
    int src_niter = src_stmt->trans->nrows;
    int dest_niter = dest_stmt->trans->nrows;
    for (int row = 0; row < tdpoly->nrows; row++) {
      int src_coeff_sum = 0;
      for (int col = 0; col < src_niter; col++) {
        src_coeff_sum += abs(tdpoly->val[row][col]);        
      }
      int dest_coeff_sum = 0;
      for (int col = src_niter; col < src_niter + dest_niter; col++) {
        dest_coeff_sum += abs(tdpoly->val[row][col]);
      }
      if (src_coeff_sum > 0 && dest_coeff_sum > 0) {
        for (int col = 0; col < src_niter; col++) {
          if (tdpoly->val[row][col] != 0) {
            if (tdpoly->val[row][col + src_niter] == 0) {
              fprintf(stdout, "[PSA] Dep %d is non-uniform. Error type: 1\n", i);
              pluto_constraints_pretty_print(stdout, tdpoly);
              return false;
            } else {
              if (tdpoly->val[row][col] + tdpoly->val[row][col + src_niter] != 0) {
                fprintf(stdout, "[PSA] Dep %d is non-uniform. Error type: 2\n", i);
                pluto_constraints_pretty_print(stdout, tdpoly);
                return false;
              } else {
                int par_id;
                for (par_id = 0; par_id < prog->npar; par_id++) {
                  if (tdpoly->val[row][src_niter + dest_niter + par_id] != 0) {                    
                    break;
                  }
                }
                if (par_id == prog->npar)
                  return true;
                else {
                  fprintf(stdout, "[PSA] Dep %d is non-uniform. Error type: 3\n", i);
                  pluto_constraints_pretty_print(stdout, tdpoly);
                  return false;                                  
                }
              }
            }
          }
        }
      }
    }
    /* Free Memory */
    pluto_constraints_free(tdpoly);
    /* Free Memory */
  }
  return true;
}

//bool systolic_array_dep_checker(PlutoProg *prog) {
//  /* Declaration */
//  int i, npar, ndeps;
//  Dep **deps;
//  bool is_uniform = true;
//
//  // nstmts = prog->nstmts;
//  ndeps = prog->ndeps;
//  deps = prog->deps;
//  // nvar = prog->nvar;
//  npar = prog->npar;
//
//  /* Check uniformity of each dependence */
//  /* The current implementation only considers multiple statements in perfectly 
//   * neseted loops. 
//   * TODO: Consider how to add support for multiple statement in imperfect nested
//   * loops.
//   */
//  for (i = 0; i < ndeps; i++) {
//    Dep *dep = deps[i];
//    PlutoConstraints *dpolytope = dep->dpolytope;
//    int src_stmt_id = dep->src;
//    int dest_stmt_id = dep->dest;
//    int src_stmt_niter = prog->stmts[src_stmt_id]->dim_orig;
//    int dest_stmt_niter = prog->stmts[dest_stmt_id]->dim_orig;
//
//    int row, col;
//    /*
//     * Case 1: src_coeff 
//     */
//    for (row = 0; row < dpolytope->nrows; row++) {
//      /* src iters should only have at most one iter with coeffcients as 1/-1 */
//      int src_coeff_sum = 0;
//      for (col = 0; col < src_stmt_niter; col++) {
//        src_coeff_sum += abs(dpolytope->val[row][col]);
//      }
//      if (src_coeff_sum > 1) {
//        is_uniform = false;
//        return is_uniform;
//      }
//      /* dest iters should only have at most one iter with coeffcients as 1/-1 */
//      int dest_coeff_sum = 0;
//      for (col = src_stmt_niter; col < src_stmt_niter + dest_stmt_niter; col++) {
//        dest_coeff_sum += abs(dpolytope->val[row][col]);
//      }
//      if (dest_coeff_sum > 1) {
//        is_uniform = false;
//        return is_uniform;
//      }
//      /* When both src and dest both have coefficients as 1/-1, they should be at
//       * the same position, and the coeff for global parameters should be zero,
//       * and the coeff for the const should be 1/-1/0.
//       */
//      if (src_coeff_sum == 1 && dest_coeff_sum == 1) {
//        for (col = 0; col < src_stmt_niter; col++) {
//          if (dpolytope->val[row][col] == 0)
//            continue;
//          else {
//            if (dpolytope->val[row][src_stmt_niter + col] == 0) {
//              is_uniform = false;
//              return is_uniform;
//            }
//            int param_col;
//            for (param_col = 0; param_col < npar; param_col++) {
//              if (dpolytope->val[row][src_stmt_niter + dest_stmt_niter + param_col] != 0) {
//                is_uniform = false;
//                return is_uniform;
//              }
//            }
//            if (abs(dpolytope->val[row][dpolytope->ncols - 1]) > 1) {
//              is_uniform = false;
//              return is_uniform;
//            }
//          }
//        }
//      }
//    }
//  }
//
//  return is_uniform;
//}

/*
 * This function analyzes the self-temporal-reuse of each external read access.
 * The detected reuse dependence will be added as the RAR dependences to the program.
 * Depending on the number of RAR dependences found for each array reference, there could be
 * numerous array candidates generated.
 */ 
PlutoProg **psa_reuse_analysis(PlutoProg *prog, int *num_reuse_progs) {
  // Step 1: Calculate the self-temporal-reuse for each external read access.
  Dep ***rar_deps = NULL;
  struct stmt_access_pair ***racc_stmts; // indexed by data variable
  int *num_stmts_per_racc; // indexed by data variable
  int num_read_data;
  racc_stmts = get_read_access_with_stmts(
      prog->stmts, prog->nstmts, &num_read_data, &num_stmts_per_racc
      );

#ifdef PSA_DEP_DEBUG
  fprintf(stdout, "enter reuse analysis.\n");
#endif

  int num_reuse_operand = 0;
  int *ndep_per_racc = NULL;

  for (int i = 0; i < num_read_data; i++) {        
    for (int j = 0; j < num_stmts_per_racc[i]; j++) {
      PlutoAccess *acc = racc_stmts[i][j]->acc;
      Stmt *stmt = racc_stmts[i][j]->stmt;
//#ifdef PSA_DEP_DEBUG
//      fprintf(stdout, "curr ref: %s\n", acc->name);
//#endif      

      // Scan all dependences and if the current acc is assoicated with a RAW dependency, it will be skipped
      int dep_id;
      for (dep_id = 0; dep_id < prog->ndeps; dep_id++) {
        Dep *dep = prog->deps[dep_id];
        if (dep->type != OSL_DEPENDENCE_RAW)
          continue;

        Stmt *src_stmt = prog->stmts[dep->src];
        Stmt *dest_stmt = prog->stmts[dep->dest];

//        if (acc == dep->dest_acc)
        if (!pluto_access_cmp(acc, dep->dest_acc))
          break;
      }

      if (dep_id != prog->ndeps) 
        continue;

      // Compute the null space of the access function
      PlutoMatrix *acc_mat = acc->mat;
      // peel off the param and constant columns
      int npar = prog->npar;
      PlutoMatrix *trunc_acc_mat = pluto_matrix_alloc(acc_mat->nrows, acc_mat->ncols - npar - 1); 
      for (int row = 0; row < trunc_acc_mat->nrows; row++)
        for (int col = 0; col < trunc_acc_mat->ncols; col++) {
          trunc_acc_mat->val[row][col] = acc_mat->val[row][col];
        }

      isl_mat *isl_acc_mat = pluto_matrix_to_isl_mat(trunc_acc_mat);      
      isl_mat *isl_null_mat = isl_mat_right_kernel(isl_acc_mat); // compute the right kernel of the matrix
      PlutoMatrix *null_space = pluto_matrix_from_isl_mat(isl_null_mat);

      /* Free Memory */
      isl_ctx *ctx = isl_mat_get_ctx(isl_null_mat);
      isl_mat_free(isl_null_mat);
      isl_ctx_free(ctx);
      pluto_matrix_free(trunc_acc_mat);
      /* Free Memory */

#ifdef PSA_DEP_DEBUG
      fprintf(stdout, "curr ref: %s\n", acc->name);
      fprintf(stdout, "access function:\n");
      pluto_matrix_print(stdout, acc_mat);
      fprintf(stdout, "null space:\n");
      pluto_matrix_print(stdout, null_space);
#endif

      int nsol = null_space->ncols;
      if (nsol > 0) {
        num_reuse_operand++;
        rar_deps = (Dep ***)realloc(rar_deps, num_reuse_operand * sizeof(Dep **));
        rar_deps[num_reuse_operand - 1] = (Dep **)malloc(nsol * sizeof(Dep *));
        construct_rar_dep(rar_deps[num_reuse_operand - 1], null_space, acc, stmt, prog);
        ndep_per_racc = (int *)realloc(ndep_per_racc, num_reuse_operand * sizeof(int));
        ndep_per_racc[num_reuse_operand - 1] = nsol;
      }

      /* Free Memory */
      pluto_matrix_free(null_space);
      /* Free Memory */
    }
  }

  /* Free Memory */
  for (int i = 0; i < num_read_data; i++) {
    for (int j = 0; j < num_stmts_per_racc[i]; j++) {
      free(racc_stmts[i][j]);
    }
    free(racc_stmts[i]);
  }
  free(racc_stmts);
  racc_stmts = NULL;

  free(num_stmts_per_racc);
  num_stmts_per_racc = NULL;
  /* Free Memory */

#ifdef PSA_DEP_DEBUG
  fprintf(stdout, "printing out added RAR deps\n");
  for (int i = 0; i < num_reuse_operand; i++)
    for (int j = 0; j < ndep_per_racc[i]; j++) {
      Dep *dep = rar_deps[i][j];
      fprintf(stdout, "************************\n");    
      fprintf(stdout, "[PSA] Src stmt ID: %d\n", dep->src);
      fprintf(stdout, "[PSA] Dest stmt ID: %d\n", dep->dest);
      if (IS_WAR(dep->type)) {
        fprintf(stdout, "[PSA] Dep type: WAR\n");
      } else if (IS_WAW(dep->type)) {
        fprintf(stdout, "[PSA] Dep type: WAW\n");
      } else if (IS_RAW(dep->type)) {
        fprintf(stdout, "[PSA] Dep type: RAW\n");
      } else if (IS_RAR(dep->type)) {
        fprintf(stdout, "[PSA] Dep type: RAR\n");
      }
      PlutoAccess *acc = dep->src_acc;
      fprintf(stdout, "[PSA] Arr name: %s\n", acc->name);

      PlutoConstraints* dpolytope = dep->dpolytope;
      pluto_constraints_pretty_print(stdout, dpolytope);
      fprintf(stdout, "************************\n");
    }
#endif

  // Step 2: Build the program variants
  int prog_num = 1;
  PlutoProg **new_progs = NULL;

  // If there is reuse
  if (num_reuse_operand > 0) {
    for (int i = 0; i < num_reuse_operand; i++) 
      prog_num *= ndep_per_racc[i];
  
    int **rar_dep_list = (int **)malloc(prog_num * sizeof(int*));
    for (int i = 0; i < prog_num; i++) {
      rar_dep_list[i] = (int*)malloc(num_reuse_operand * sizeof(int));
    }
  
    // generate different combinations of rar dep indexes
    int dup_times = prog_num;
    int rep_times = 1;
  
#ifdef PSA_DEP_DEBUG
    fprintf(stdout, "%d\n", prog_num);
#endif
  
    for (int i = 0; i < num_reuse_operand; i++) {
      dup_times = dup_times / ndep_per_racc[i];
      for (int p = 0; p < rep_times; p++) {
        for (int j = 0; j < ndep_per_racc[i]; j++)
          for (int q = 0; q < dup_times; q++) {
#ifdef PSA_DEP_DEBUG
            fprintf(stdout, "(%d, %d)\n", p * ndep_per_racc[i] * dup_times + j * dup_times + q, i);
#endif
            rar_dep_list[p * ndep_per_racc[i] * dup_times + j * dup_times + q][i] = j;
          }
      }
  
      rep_times *= ndep_per_racc[i];
    }
  
    new_progs = (PlutoProg **)malloc(prog_num * sizeof(PlutoProg *));
    for (int i = 0; i < prog_num; i++) {
      new_progs[i] = pluto_prog_dup(prog);
      new_progs[i]->deps = realloc(new_progs[i]->deps, (new_progs[i]->ndeps + num_reuse_operand) * sizeof(Dep *));
      for (int j = new_progs[i]->ndeps; j < new_progs[i]->ndeps + num_reuse_operand; j++) {
#ifdef PSA_DEP_DEBUG
        fprintf(stdout, "(%d, %d)\n", j - new_progs[i]->ndeps, rar_dep_list[i][j - new_progs[i]->ndeps]);
#endif
        new_progs[i]->deps[j] = pluto_dep_dup(rar_deps[j - new_progs[i]->ndeps][rar_dep_list[i][j - new_progs[i]->ndeps]]);
        new_progs[i]->deps[j]->id = j;
      }
      new_progs[i]->ndeps = new_progs[i]->ndeps + num_reuse_operand;
  
      // reassociate the dep and stmts
      reassociate_dep_stmt_acc(new_progs[i]);
    }
#ifdef PSA_DEP_DEBUG  
    fprintf(stdout, "[PSA] Print out the dependences.\n");
    fprintf(stdout, "[PSA] Total number of dependences: %d\n", new_progs[0]->ndeps);
    for (int i = 0; i < new_progs[0]->ndeps; i++) {
      Dep *dep = new_progs[0]->deps[i];
      fprintf(stdout, "***********************\n");
      fprintf(stdout, "[PSA] Dependences ID: %d\n", i);
      fprintf(stdout, "[PSA] Src stmt ID: %d\n", dep->src);
      fprintf(stdout, "[PSA] Dest stmt ID: %d\n", dep->dest);
      if (IS_WAR(dep->type)) {
        fprintf(stdout, "[PSA] Dep type: WAR\n");
      } else if (IS_WAW(dep->type)) {
        fprintf(stdout, "[PSA] Dep type: WAW\n");
      } else if (IS_RAW(dep->type)) {
        fprintf(stdout, "[PSA] Dep type: RAW\n");
      } else if (IS_RAR(dep->type)) {
        fprintf(stdout, "[PSA] Dep type: RAR\n");
      }
      PlutoAccess *acc = dep->src_acc;
      fprintf(stdout, "[PSA] Arr name: %s\n", acc->name);    
  
      PlutoConstraints* dpolytope = dep->dpolytope;
      pluto_constraints_pretty_print(stdout, dpolytope);
      fprintf(stdout, "***********************\n");
    }
#endif
  
    *num_reuse_progs = prog_num;

    /* Free Memory */
    for (int i = 0; i < prog_num; i++) {
      free(rar_dep_list[i]);
    }
    free(rar_dep_list);
    rar_dep_list = NULL;
    /* Free Memory */
  } else {
    new_progs = (PlutoProg **)malloc(prog_num * sizeof(PlutoProg *));
    new_progs[0] = prog;
    *num_reuse_progs = prog_num;
  }

  /* Free Memory */
  for (int i = 0; i < num_reuse_operand; i++) {
    for (int j = 0; j < ndep_per_racc[i]; j++) {
      pluto_dep_free(rar_deps[i][j]);
    }
    free(rar_deps[i]);
  }
  free(rar_deps);
  free(ndep_per_racc);
  /* Free Memory */

  return new_progs;
}

PlutoConstraints *construct_rar_dep_polytope(PlutoMatrix *null_space, int idx, Stmt *stmt) {
  PlutoConstraints *new_polytope;
  // add dependence relationship
  PlutoMatrix *dep_mat = pluto_matrix_alloc(null_space->nrows, stmt->dim * 2 + stmt->domain->ncols - stmt->dim);
  pluto_matrix_set(dep_mat, 0);
  for (int i = 0; i < null_space->nrows; i++) {
    if (null_space->val[i][idx] != 0) {
      dep_mat->val[i][i] = -1;
      dep_mat->val[i][i + stmt->dim] = 1;
      dep_mat->val[i][dep_mat->ncols - 1] = -null_space->val[i][idx];
    } else {
      dep_mat->val[i][i] = -1;
      dep_mat->val[i][i + stmt->dim] = 1;
      dep_mat->val[i][dep_mat->ncols - 1] = 0;
    }
  }
  new_polytope = pluto_constraints_from_equalities(dep_mat);
  
  // add iteration domain
  PlutoConstraints *src_domain = pluto_constraints_dup(stmt->domain);
  for (int j = 0; j < stmt->dim; j++) {
    pluto_constraints_add_dim(src_domain, stmt->dim, "");
  }
  PlutoConstraints *dest_domain = pluto_constraints_dup(stmt->domain);
  for (int j = 0; j < stmt->dim; j++) {
    pluto_constraints_add_dim(dest_domain, 0, "");
  }

  // merge constraints
  new_polytope = pluto_constraints_add(new_polytope, src_domain); 
  new_polytope = pluto_constraints_add(new_polytope, dest_domain);

  // simplify the pluto constraint
  pluto_constraints_simplify(new_polytope);

  /* Free Memory */
  pluto_constraints_free(src_domain);
  pluto_constraints_free(dest_domain);
  pluto_matrix_free(dep_mat);
  /* Free Memory */

  return new_polytope;
}

/*
 * This function constructs rar deps from each independent basis in null space.
 */ 
void construct_rar_dep(Dep **deps, PlutoMatrix *null_space, PlutoAccess *acc, Stmt *stmt, PlutoProg *prog) {
  for (int i = 0; i < null_space->ncols; i++) {
    deps[i] = pluto_dep_alloc();
    Dep *dep = deps[i];
    dep->id = i;
    dep->type = OSL_DEPENDENCE_RAR;
    dep->src = stmt->id;
    dep->dest = stmt->id;
    dep->src_acc = acc;
    dep->dest_acc = acc;

    dep->dpolytope = construct_rar_dep_polytope(null_space, i, stmt);
    dep->bounding_poly = pluto_constraints_dup(dep->dpolytope);

    pluto_constraints_set_names_range(
        dep->dpolytope, stmt->iterators, 0, 0, stmt->dim);
    // suffix the destination iterators with a '
    char **dnames = malloc(stmt->dim * sizeof(char *));
    for (int j = 0; j < stmt->dim; j++) {
      dnames[j] = malloc(strlen(stmt->iterators[j]) + 2);
      strcpy(dnames[j], stmt->iterators[j]);
      strcat(dnames[j], "'");
    }
    pluto_constraints_set_names_range(
        dep->dpolytope, dnames, stmt->dim, 0, stmt->dim);
    for (int j = 0; j < stmt->dim; j++)
      free(dnames[j]);
    free(dnames);

    pluto_constraints_set_names_range(
        dep->dpolytope, prog->params, stmt->dim + stmt->dim, 0, prog->npar);

    /* Get rid of rows that are all zeros */
    int r, c;
    bool *remove = (bool *)malloc(sizeof(bool) * dep->dpolytope->nrows);
    for (r = 0; r < dep->dpolytope->nrows; r++) {
      for (c = 0; c < dep->dpolytope->ncols; c++ ) {
        if (dep->dpolytope->val[r][c] != 0)
          break;
      }
      if (c == dep->dpolytope->ncols) 
        remove[r] = true;
      else
        remove[r] = false;
    }
    int orig_nrows = dep->dpolytope->nrows;
    int del_count = 0;
    for (r = 0; r < orig_nrows; r++) {
      if (remove[r]) {
        pluto_constraints_remove_row(dep->dpolytope, r - del_count);
        del_count++;
      }
    }
    free(remove);
  }
}

/*
 * Filter out RAR dependences with scalar variables.
 * Note: This function only processes the non-transitive dependences
 */
void rar_scalar_filter(PlutoProg *prog) {
  /* Declaration */
  int i, j, ndeps;
  Dep **deps;
  // char **data_names;
  
  ndeps = prog->ndeps;
  // ndata = prog->num_data;
  deps = prog->deps;
  // data_names = prog->data_names;

  bool *is_scalar_dep = (bool *)malloc(ndeps * sizeof(bool));

//#ifdef JIE_DEBUG
//  for (i = 0; i < ndata; i++) {
//    fprintf(stdout, "[Debug] array name: %s\n", data_names[i]);
//  }
//#endif  

  /* The scalar dep has only one row in access function.
   * And all elements are zeros. */
  int nscalar_deps = 0;
  for (i = 0; i < ndeps; i++) {
    Dep *dep = deps[i];
    if (IS_RAR(dep->type)) {
      PlutoAccess *acc = dep->src_acc;
//#ifdef JIE_DEBUG      
      //fprintf(stdout, "[Debug] acc_nrows: %d\n", acc->mat->nrows);
      //fprintf(stdout, "[Debug] acc_ncols: %d\n", acc->mat->ncols);
//#endif
      if (acc->mat->nrows == 1) {
        for (j = 0; j < acc->mat->ncols; j++) {
          if (acc->mat->val[0][j] != 0) {
            break;
          } 
        }
        if (j == acc->mat->ncols) {
          is_scalar_dep[i] = true;
          nscalar_deps++;
        } else {
          is_scalar_dep[i] = false;
        }
      } else {
        is_scalar_dep[i] = false;
      }
    } else {
      is_scalar_dep[i] = false;
    }
  }

// #ifdef JIE_DEBUG
//   for (i = 0; i < ndeps; i++) {
//     fprintf(stdout, "[Debug] is_scalar: %d\n", is_scalar_dep[i]);
//   }
// #endif

  /* Delete the scalar deps in the program.
   * Update ndeps and deps in the program. */
  int new_ndeps = ndeps - nscalar_deps;
  prog->deps = (Dep **)malloc(new_ndeps * sizeof(Dep *));
  prog->ndeps = 0;
  for (i = 0; i < ndeps; i++) {
    if (!is_scalar_dep[i]) {
      prog->deps[prog->ndeps] = pluto_dep_dup(deps[i]);
      prog->ndeps++;
    }
  }
// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] new ndeps: %d\n", prog->ndeps);
// #endif
 
  /* Now we need to update the access function pointers in both stmts and deps */
  reassociate_dep_stmt_acc(prog);

  /* clean up */
  free(is_scalar_dep);
  for (i = 0; i < ndeps; i++) {
    pluto_dep_free(deps[i]);
//    free(deps[i]);
  }
  free(deps);
}

/*
 * After allocating or duplicating new dependences, we need to reassociate the access functions
 * that each depedence points to to the original access functions pointed by stmts so that 
 * stmts and deps point to the same of copy of access function. This is vital for the following code which may 
 * depend on the direct comparison of access function pointer addresses.
 */ 
void reassociate_dep_stmt_acc(PlutoProg *prog) {
  /* dependences */
  for (int i = 0; i < prog->ndeps; i++) {
    Dep *dep = prog->deps[i];
    PlutoAccess *src_acc = dep->src_acc;
    PlutoAccess *dest_acc = dep->dest_acc;
    int src_stmt_id = dep->src;
    int dest_stmt_id = dep->dest;
    
    bool src_detected = false;
    bool dest_detected = false;

    for (int j = 0; j < prog->stmts[src_stmt_id]->nreads; j++) {
      if (!pluto_access_cmp(src_acc, prog->stmts[src_stmt_id]->reads[j]) 
          && src_acc != prog->stmts[src_stmt_id]->reads[j]) {
        pluto_access_free(src_acc);
        dep->src_acc = prog->stmts[src_stmt_id]->reads[j];
        src_detected = true;
        break;
      }
    }
    if (!src_detected) {
      for (int j = 0; j < prog->stmts[src_stmt_id]->nwrites; j++) {
        if (!pluto_access_cmp(src_acc, prog->stmts[src_stmt_id]->writes[j])
            && src_acc != prog->stmts[src_stmt_id]->writes[j]) {
          pluto_access_free(src_acc);
          dep->src_acc = prog->stmts[src_stmt_id]->writes[j];
          break;
        }
      }
    }
    for (int j = 0; j < prog->stmts[dest_stmt_id]->nreads; j++) {
      if (!pluto_access_cmp(dest_acc, prog->stmts[dest_stmt_id]->reads[j])
          && dest_acc != prog->stmts[dest_stmt_id]->reads[j]) {
        pluto_access_free(dest_acc);
        dep->dest_acc = prog->stmts[dest_stmt_id]->reads[j];
        dest_detected = true;
        break;
      }
    }
    if (!dest_detected) {
      for (int j = 0; j < prog->stmts[dest_stmt_id]->nwrites; j++) {
        if (!pluto_access_cmp(dest_acc, prog->stmts[dest_stmt_id]->writes[j])
            && dest_acc != prog->stmts[dest_stmt_id]->writes[j]) {
          pluto_access_free(dest_acc);
          dep->dest_acc = prog->stmts[dest_stmt_id]->writes[j];
          break;
        }
      }  
    }
  }

  /* transitive dependences */
  for (int i = 0; i < prog->ntransdeps; i++) {
    Dep *dep = prog->transdeps[i];
    PlutoAccess *src_acc = dep->src_acc;
    PlutoAccess *dest_acc = dep->dest_acc;
    int src_stmt_id = dep->src;
    int dest_stmt_id = dep->dest;

    bool src_detected = false;
    bool dest_detected = false;

    for (int j = 0; j < prog->stmts[src_stmt_id]->nreads; j++) {
      if (!pluto_access_cmp(src_acc, prog->stmts[src_stmt_id]->reads[j]) 
          && src_acc != prog->stmts[src_stmt_id]->reads[j]) {
        pluto_access_free(src_acc);
        dep->src_acc = prog->stmts[src_stmt_id]->reads[j];
        src_detected = true;
        break;
      }
    }
    if (!src_detected) {
      for (int j = 0; j < prog->stmts[src_stmt_id]->nwrites; j++) {
        if (!pluto_access_cmp(src_acc, prog->stmts[src_stmt_id]->writes[j])
            && src_acc != prog->stmts[src_stmt_id]->writes[j]) {
          pluto_access_free(src_acc);
          dep->src_acc = prog->stmts[src_stmt_id]->writes[j];
          break;
        }
      }
    }
    for (int j = 0; j < prog->stmts[dest_stmt_id]->nreads; j++) {
      if (!pluto_access_cmp(dest_acc, prog->stmts[dest_stmt_id]->reads[j])
          && dest_acc != prog->stmts[dest_stmt_id]->reads[j]) {
        pluto_access_free(dest_acc);
        dep->dest_acc = prog->stmts[dest_stmt_id]->reads[j];
        dest_detected = true;
        break;
      }
    }
    if (!dest_detected) {
      for (int j = 0; j < prog->stmts[dest_stmt_id]->nwrites; j++) {
        if (!pluto_access_cmp(dest_acc, prog->stmts[dest_stmt_id]->writes[j])
            && dest_acc != prog->stmts[dest_stmt_id]->writes[j]) {
          pluto_access_free(dest_acc);
          dep->dest_acc = prog->stmts[dest_stmt_id]->writes[j];
          break;
        }
      }  
    }
  }
 
}

/*
 * Filter out RAR dependences.
 */
void rar_filter(PlutoProg *prog) {
  /* Declaration */
  int i, j, ndeps;
  Dep **deps;
  
  ndeps = prog->ndeps;
  deps = prog->deps;

  bool *is_rar_dep = (bool *)malloc(ndeps * sizeof(bool));
  int nrar_deps = 0;

  for (i = 0; i < ndeps; i++) {
    Dep *dep = deps[i];
    if (IS_RAR(dep->type)) {
      is_rar_dep[i] = 1;
      nrar_deps++;
    } else {
      is_rar_dep[i] = 0;
    }
  }

  /* Delete the rar deps in the program */
  int new_ndeps = ndeps - nrar_deps;
  prog->deps = (Dep **)malloc(new_ndeps * sizeof(Dep *));
  prog->ndeps = 0;
  for (i = 0; i < ndeps; i++) {
    if (!is_rar_dep[i]) {
      prog->deps[prog->ndeps++] = pluto_dep_dup(deps[i]);
    }
  }

  /* clean up */
  free(is_rar_dep);
  for (i = 0; i < ndeps; i++) {
    pluto_dep_free(deps[i]);
  }
  free(deps);

  /* trans deps */
  ndeps = prog->ntransdeps;
  deps = prog->transdeps;

  is_rar_dep = (bool *)malloc(ndeps * sizeof(bool));
  nrar_deps = 0;

  for (i = 0; i < ndeps; i++) {
    Dep *dep = deps[i];
    if (IS_RAR(dep->type)) {
      is_rar_dep[i] = 1;
      nrar_deps++;
    } else {
      is_rar_dep[i] = 0;
    }
  }

  /* Delete the rar deps in the program */
  new_ndeps = ndeps - nrar_deps;
  prog->transdeps = (Dep **)malloc(new_ndeps * sizeof(Dep *));
  prog->ntransdeps = 0;
  for (i = 0; i < ndeps; i++) {
    if (!is_rar_dep[i]) {
      prog->transdeps[prog->ntransdeps++] = pluto_dep_dup(deps[i]);
    }
  }

  /* clean up */
  free(is_rar_dep);
  for (i = 0; i < ndeps; i++) {
    pluto_dep_free(deps[i]);
  }
  free(deps);
 
  /* Now we need to update the access function pointers in both stmts and deps */
  reassociate_dep_stmt_acc(prog);
}

