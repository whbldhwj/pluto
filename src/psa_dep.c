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
 * Detecting non-uniform dependences and exit the program if any non-uniform
 * dependences is detected. 
 */
bool systolic_array_dep_checker(PlutoProg *prog) {
  /* Declaration */
  int i, npar, ndeps;
  Dep **deps;
  bool is_uniform = true;

  // nstmts = prog->nstmts;
  ndeps = prog->ndeps;
  deps = prog->deps;
  // nvar = prog->nvar;
  npar = prog->npar;

  /* Check uniformity of each dependence */
  /* The current implementation only considers multiple statements in perfectly 
   * neseted loops. 
   * TODO: Consider how to add support for multiple statement in imperfect nested
   * loops.
   */
  for (i = 0; i < ndeps; i++) {
    Dep *dep = deps[i];
    PlutoConstraints *dpolytope = dep->dpolytope;
    int src_stmt_id = dep->src;
    int dest_stmt_id = dep->dest;
    int src_stmt_niter = prog->stmts[src_stmt_id]->dim_orig;
    int dest_stmt_niter = prog->stmts[dest_stmt_id]->dim_orig;

    int row, col;
    /*
     * Case 1: src_coeff 
     */
    for (row = 0; row < dpolytope->nrows; row++) {
      /* src iters should only have at most one iter with coeffcients as 1/-1 */
      int src_coeff_sum = 0;
      for (col = 0; col < src_stmt_niter; col++) {
        src_coeff_sum += abs(dpolytope->val[row][col]);
      }
      if (src_coeff_sum > 1) {
        is_uniform = false;
        return is_uniform;
      }
      /* dest iters should only have at most one iter with coeffcients as 1/-1 */
      int dest_coeff_sum = 0;
      for (col = src_stmt_niter; col < src_stmt_niter + dest_stmt_niter; col++) {
        dest_coeff_sum += abs(dpolytope->val[row][col]);
      }
      if (dest_coeff_sum > 1) {
        is_uniform = false;
        return is_uniform;
      }
      /* When both src and dest both have coefficients as 1/-1, they should be at
       * the same position, and the coeff for global parameters should be zero,
       * and the coeff for the const should be 1/-1/0.
       */
      if (src_coeff_sum == 1 && dest_coeff_sum == 1) {
        for (col = 0; col < src_stmt_niter; col++) {
          if (dpolytope->val[row][col] == 0)
            continue;
          else {
            if (dpolytope->val[row][src_stmt_niter + col] == 0) {
              is_uniform = false;
              return is_uniform;
            }
            int param_col;
            for (param_col = 0; param_col < npar; param_col++) {
              if (dpolytope->val[row][src_stmt_niter + dest_stmt_niter + param_col] != 0) {
                is_uniform = false;
                return is_uniform;
              }
            }
            if (abs(dpolytope->val[row][dpolytope->ncols - 1]) > 1) {
              is_uniform = false;
              return is_uniform;
            }
          }
        }
      }
    }
  }

  return is_uniform;
}

/*
 * Filter out RAR dependences with scalar variables.
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
  prog->ndeps = new_ndeps;
  prog->deps = (Dep **)malloc(prog->ndeps * sizeof(Dep *));
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
  
  /* clean up */
  free(is_scalar_dep);
  for (i = 0; i < ndeps; i++) {
    free(deps[i]);
  }
  free(deps);
}
