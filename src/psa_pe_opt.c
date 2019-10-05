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
#include "transforms.h"
#include "psa_dep.h"
#include "psa_array.h"
#include "psa_partition.h"
#include "psa_pe_opt.h"
#include "psa_helpers.h"

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

/* Read tiles sizes from simd_tile.sizes */
int psa_read_simd_tile_sizes(
  int *tile_sizes,
  int num_tile_dims
) {
  int i;
  FILE *tsfile = fopen("simd_tile.sizes", "r");

  if (!tsfile) {
    fprintf(stdout, "[PSA] WARNING: simd_tile.sizes not found!\n");
    return 0;
  }

  for (i = 0; i < num_tile_dims && !feof(tsfile); i++) {
    fscanf(tsfile, "%d", &tile_sizes[i]);
  }

  if (i < num_tile_dims) {
    fprintf(stdout, "[PSA] WARNING: not enough SIMD vectorization tile sizes provided!"
                    "The required number: %d\n", num_tile_dims);
    fclose(tsfile);
    return 0;
  }

  fclose(tsfile);
  return 1;
}

/* Read tiles sizes from lh_tile.sizes */
int psa_read_latency_hiding_tile_sizes(
  int *tile_sizes,
  int num_tile_dims
) {
  int i;
  FILE *tsfile = fopen("lh_tile.sizes", "r");

  if (!tsfile) {
    fprintf(stdout, "[PSA] WARNING: lh_tile.sizes not found!\n");
    return PSA_FAILURE;
  }

  for (i = 0; i < num_tile_dims && !feof(tsfile); i++) {
    fscanf(tsfile, "%d", &tile_sizes[i]);
  }

  if (i < num_tile_dims) {
    fprintf(stdout, "[PSA] WARNING: not enough latency hiding tile sizes provided!"
                    "The required number: %d\n", num_tile_dims);
    fclose(tsfile);
    return PSA_FAILURE;
  }

  fclose(tsfile);
  return PSA_SUCCESS;
}

void psa_latency_hiding_misc_pretty_print(
  const PlutoProg *prog,
  int num_parallel_time_loop,
  Ploop **parallel_time_loops,
  int num_parallel_space_loop,
  Ploop **parallel_space_loops
) {
  int nstmts, i, j;
  nstmts = prog->nstmts;

  for (i = 0; i < nstmts; i++) {    
    Stmt *stmt = prog->stmts[i];

    int *mark_loops = (int *)malloc(stmt->trans->nrows * sizeof(int));
    /* initialization */
    for (j = 0; j < stmt->trans->nrows; j++)
      mark_loops[j] = 0;

    for (j = 0; j < num_parallel_time_loop; j++) {
      Ploop *loop = parallel_time_loops[j];
      /* detect if the stmt is under the loop */
      if (pluto_stmt_is_member_of(stmt->id, loop->stmts, loop->nstmts))
        mark_loops[loop->depth] = 1;
    }
    for (j = 0; j < num_parallel_space_loop; j++) {
      Ploop *loop = parallel_space_loops[j];
      /* detect if the stmt is under the loop */
      if (pluto_stmt_is_member_of(stmt->id, loop->stmts, loop->nstmts))
        mark_loops[loop->depth] = 1;
    }

    fprintf(stdout, "T(S%d): ", stmt->id + 1);
    int level;
    fprintf(stdout, "(");
    for (level = 0; level < stmt->trans->nrows; level++) {
      pluto_stmt_print_hyperplane(stdout, stmt, level);
      if (level <= stmt->trans->nrows - 2)
        fprintf(stdout, ", ");
    }
    fprintf(stdout, ")\n");
    
    fprintf(stdout, "loop types (");
    for (level = 0; level < stmt->trans->nrows; level++) {
      if (level > 0)
        fprintf(stdout, ", ");
      if (stmt->hyp_types[level] == H_SCALAR)
        fprintf(stdout, "scalar");
      else if (stmt->hyp_types[level] == H_LOOP)
        fprintf(stdout, "loop");
      else if (stmt->hyp_types[level] == H_TILE_SPACE_LOOP)
        fprintf(stdout, "tloop");
      else
        fprintf(stdout, "unknown");
      // the candidate loops comes with an asterisk as the suffix
      if (mark_loops[level])
        fprintf(stdout, "*");
    }
    fprintf(stdout, ")\n");

    fprintf(stdout, "psa loop types (");
    for (level = 0; level < stmt->trans->nrows; level++) {
      if (level > 0)
        fprintf(stdout, ", ");
      if (IS_PSA_SCALAR(stmt->psa_hyp_types[level]))
        fprintf(stdout, "scalar");
      else if (IS_PSA_SPACE_LOOP(stmt->psa_hyp_types[level]))
        fprintf(stdout, "space_loop");
      else if (IS_PSA_TIME_LOOP(stmt->psa_hyp_types[level]))
        fprintf(stdout, "time_loop");
      else if (IS_PSA_ARRAY_PART_LOOP(stmt->psa_hyp_types[level]))
        fprintf(stdout, "array_part_loop");
      else if (IS_PSA_TASK_INTER_LOOP(stmt->psa_hyp_types[level]))
        fprintf(stdout, "task_inter_loop");
      else if (IS_PSA_SIMD_LOOP(stmt->psa_hyp_types[level]))
        fprintf(stdout, "simd_loop");
      else
        fprintf(stdout, "unknown");
    }
    fprintf(stdout, ")\n");

    free(mark_loops);
  }
}

/*
 * For loops in each innermost loop bands (considering only time loops), 
 * move the sync-free (RAW, WAW, WAR) loops inside. If there is no sync-free
 * loop in the current loop bands, see if we could extend it to space loops
 * and tile it twice and move it inside.
 * NOTE: currently only consider space loops
 */
int psa_latency_hiding_optimize_band(Band *band, PlutoProg *prog) {  
  unsigned i, j;
  unsigned nloops;
  Ploop **loops;

  /* Grasp all the loops in the current band */
//  loops = pluto_get_loops_under(
//      band->loop->stmts, band->loop->nstmts,
//      band->loop->depth, prog, &nloops);
  loops = psa_get_loops_in_band(band, prog, &nloops);

  /* Mark the hyp properties */
  unsigned first_space_hyp, first_time_hyp;
  bool first_array_part_hyp_found = false;
  bool first_space_hyp_found = false;
  bool first_time_hyp_found = false;    
  for (i = 0; i < prog->num_hyperplanes; i++) {
    if (IS_PSA_ARRAY_PART_LOOP(prog->hProps[i].psa_type) && !first_array_part_hyp_found) {
      // first_array_part_hyp = i;
      first_array_part_hyp_found = true;
    }
    if (IS_PSA_SPACE_LOOP(prog->hProps[i].psa_type) && !first_space_hyp_found) {
      first_space_hyp = i;
      first_space_hyp_found = true;
    }
    if (IS_PSA_TIME_LOOP(prog->hProps[i].psa_type) && !first_time_hyp_found) {
      first_time_hyp = i;
      first_time_hyp_found = true;
    }
  }

  // detect parallel time/space loops
  unsigned num_parallel_time_loop = 0;
  Ploop **parallel_time_loops = NULL;
  unsigned num_parallel_space_loop = 0;
  Ploop **parallel_space_loops = NULL;

  for (i = 0; i < nloops; i++) {
    Ploop *loop = loops[i];
    if (pluto_loop_is_parallel(prog, loop)) {
      if (loop->depth >= first_time_hyp) {
        num_parallel_time_loop++;
        parallel_time_loops = realloc(parallel_time_loops, num_parallel_time_loop * sizeof(Ploop *));
        parallel_time_loops[num_parallel_time_loop - 1] = loop;
      } else if (loop->depth >= first_space_hyp){
        num_parallel_space_loop++;
        parallel_space_loops = realloc(parallel_space_loops, num_parallel_space_loop * sizeof(Ploop *));
        parallel_space_loops[num_parallel_space_loop - 1] = loop;
      }
    }
  }

#ifdef PSA_LATENCY_HIDING_DEBUG
  fprintf(stdout, "[Debug] array_dim: %d\n", prog->array_dim);
  fprintf(stdout, "[Debug] num_time_loops: %d num_space_loops: %d\n", num_parallel_time_loop, num_parallel_space_loop);
  fprintf(stdout, "[Debug] time_depth: %d space_depth: %d\n", parallel_time_loops[0]->depth, parallel_space_loops[0]->depth);
#endif

#ifdef PRINT_LATENCY_HIDING_MISC
  psa_latency_hiding_misc_pretty_print(
      prog, 
      num_parallel_time_loop, parallel_time_loops,
      num_parallel_space_loop, parallel_space_loops);
#endif

  if (num_parallel_time_loop) {
    /* Permute sync-free loop innermost */
    for (i = 0; i < num_parallel_time_loop; i++) {
      Ploop *loop = parallel_time_loops[i];
      // pluto_make_innermost_loop(loop, prog);
      psa_make_innermost_loop_band(loop, band, prog);
      
      int last_depth = band->loop->depth + band->width - 1;

//      int last_depth = prog->num_hyperplanes - 1;
//      for (j = 0; j < loop->nstmts; j++) {
//        Stmt *stmt = loop->stmts[j];
//        int d;
//        for (d = loop->depth; d < stmt->trans->nrows; d++) {
//          if (pluto_is_hyperplane_scalar(stmt, d)) {
//            break;
//          }
//        }
//        last_depth = PLMIN(last_depth, d - 1);
//      }      

      // update the hyperplane properties
      prog->hProps[last_depth].psa_type = PSA_H_TASK_INTER_LOOP;
      for (j = 0; j < loop->nstmts; j++) {
        Stmt *stmt = loop->stmts[j];
        stmt->psa_hyp_types[last_depth] = PSA_H_TASK_INTER_LOOP;
      }
    } 
    /* Free Memory */
    for (int i = 0; i < nloops; i++) {
      pluto_loop_free(loops[i]);
    }
    free(loops);
    free(parallel_time_loops);
    free(parallel_space_loops);
    /* Free Memory */

    return PSA_SUCCESS;
  } else if (num_parallel_space_loop) {
    /* Tile the sync-free space loop and permute them innermost */
    int tile_sizes[num_parallel_space_loop];
    for (i = 0; i < num_parallel_space_loop; i++) {
      tile_sizes[i] = DEFAULT_LATENCY_HIDING_TILE_FACTOR;
    }

    psa_read_latency_hiding_tile_sizes(tile_sizes, num_parallel_space_loop);

    /* Complete the array_row_il_factor and array_col_il_factor */
    for (i = 0; i < prog->array_dim; i++) {
      prog->array_il_factor[i] = 1;
    }

    for (i = 0; i < num_parallel_space_loop; i++) {
      prog->array_il_factor[parallel_space_loops[i]->depth - first_space_hyp] = tile_sizes[i];
    }

    for (i = 0; i < num_parallel_space_loop; i++) {
      Ploop *loop = parallel_space_loops[i];
//      psa_tile_loop(prog, loop, tile_sizes[i], H_TILE_SPACE_LOOP, PSA_H_SPACE_LOOP);        
      psa_tile_loop_constant(prog, loop, tile_sizes[i], H_TILE_SPACE_LOOP, PSA_H_SPACE_LOOP);

      int num_intra_tile_loops = 0;
      Ploop **intra_tile_loops = pluto_get_loops_immediately_inner(loop, prog, &num_intra_tile_loops);
      assert(num_intra_tile_loops == 1);

      band->width++;
      psa_make_innermost_loop_band(intra_tile_loops[0], band, prog);
      // pluto_make_innermost_loop(intra_tile_loops[0], prog);

//      int last_depth = prog->num_hyperplanes - 1;
//      for (j = 0; j < loop->nstmts; j++) {
//        Stmt *stmt = loop->stmts[j];
//        int d;
//        for (d = loop->depth; d < stmt->trans->nrows; d++) {
//          if (pluto_is_hyperplane_scalar(stmt, d)) {
//            break;
//          }
//        }
//        last_depth = PLMIN(last_depth, d - 1);
//      }
      int last_depth = band->loop->depth + band->width - 1;

      // update the hyperplane properties
      prog->hProps[last_depth].psa_type = PSA_H_TASK_INTER_LOOP;
      for (j = 0; j < loop->nstmts; j++) {
        Stmt *stmt = loop->stmts[j];
        stmt->psa_hyp_types[last_depth] = PSA_H_TASK_INTER_LOOP;
      }

      /* Free Memory */
      pluto_loops_free(intra_tile_loops, num_intra_tile_loops);        
      /* Free Memory */
    }        

    /* Free Memory */
    for (int i = 0; i < nloops; i++) {
      pluto_loop_free(loops[i]);
    }
    free(loops);
    free(parallel_time_loops);
    free(parallel_space_loops);
    /* Free Memory */

    return PSA_SUCCESS;
  } else {
    /* No opportunity for task interleaving */

    /* Free Memory */
    for (int i = 0; i < nloops; i++) {
      pluto_loop_free(loops[i]);
    }
    free(loops);
    free(parallel_time_loops);
    free(parallel_space_loops);
    /* Free Memory */
    return PSA_FAILURE;
  } 
}

bool is_stride_zero_one(Stmt *stmt, PlutoAccess *acc, int depth, int *is_transform, int *transform_dim) {
  int i, j, *divs;
  PlutoMatrix *newacc = pluto_get_new_access_func(stmt, acc->mat, &divs);
  assert(depth <= newacc->ncols - 1);

  /* Scalars */
  /* Fix the bug in the original Pluto */
  /* the scalar access function has one row with all zeros */
  if (newacc->nrows == 1) {
    for (i = 0; i < newacc->ncols; i++) {
      if (newacc->val[0][i] != 0) {
        break;
      }
    }
    /* Skip the scalar functions */
    if (i == newacc->ncols) {
      *is_transform = 0;
      *transform_dim = -1;
      return 1;
    }
  }

  /* There could be multiple rows with stride 0/1 access.
   * Between rows, we favor the last row over other rows, as this eliminates the needs of layout transformation
   * Inside the same row, we favor stride-0 access over stride-1 access, as this minimizes the hardware overheads.
   * For example, reduce the needs of bank partitioning.
   */
  /* stride-0 access */
  for (i = 0; i < newacc->nrows; i++) {
    if (newacc->val[i][depth] != 0)
      break;
  }
  if (i == newacc->nrows) {
    *is_transform = 0;
    *transform_dim = -1;
    return 1;
  }
  /* stride-1 access */
  for (i = newacc->nrows - 1; i >= 0; i--) {
    if (newacc->val[i][depth] == 1) {
      for (j = 0; j < newacc->nrows; j++) {
        if (j == i)
          continue;
        if (newacc->val[j][depth] != 0)
          break;
      }
      if (j == newacc->nrows) {
        pluto_matrix_free(newacc);
        free(divs);
        *is_transform = !(i == newacc->nrows - 1);
        *transform_dim = i;
        return 1;
      }
    }
  }

  pluto_matrix_free(newacc);
  free(divs);

  return 0;
}

/* This function calulates the score of each loop in terms of opportunities of SIMD vectorization.
 * Frist of all, the loop has to be either a parallel loop or a reduction loop.
 * We test the reduction loop by exaimining if the carried dependence by the current loop is from the
 * reudction statement.
 * We need users to mark the reduction statement in the reduction.annotation file.
 * Next, we will need to test if all the access functions in the loop has only 0/1-stride access. 
 * If either of the two criteria failss, the loop is non-vectorizable.
 * Finally, we will calculate the score of each loop which will is used to rank the loop given there are 
 * multiple SIMD loops available.
 * The score is calculated as:
 * score = Sigma_{all_accesses_under_the_loop} {is_access_0/1_stride * (1 - is_layout_transformation_required)} 
 *            + 2 * is_loop_parallel + 4 * is_loop_redunction
 * In the current heuristic, we favor reduction loop over parallel loop, because the reduction loop has less overhead
 * compared to the parallel loop. And we favor loops that doesn't lead to layout transformation than loops that require
 * the layout transformation.
 */
int psa_is_loop_vectorizable(Ploop *loop, PlutoProg *prog, PSAAccess ***psa_acc, int *num_psa_acc) {
  bool is_reduction = 0;
  int score = 0;
  
  /* check if the loop is a reduction loop */
  fprintf(stdout, "[PSA] Trying to detect the reduction loop.\n");
  FILE *rdfile = fopen("reduce.annotation", "r");
  if (!rdfile) {
    fprintf(stdout, "[PSA] Annotation file not found!\n");
    fprintf(stdout, "[PSA] The program will proceed as normal but please provide information in the file reduce.annotation if any.\n");    
  } else {
    int *rdstmt_id = (int *)malloc(prog->nstmts * sizeof(int));
    int n_rdstmts;
    for (n_rdstmts = 0; n_rdstmts < loop->nstmts && !feof(rdfile); n_rdstmts++) {
      fscanf(rdfile, "S%d", &rdstmt_id[n_rdstmts]);
    }

    n_rdstmts--;
    for (int i = 0; i < n_rdstmts; i++) {
      fprintf(stdout, "[PSA] Statement S%d identified as reduction statement.\n", rdstmt_id[i]);
    }

    is_reduction = 1;
    for (int i = 0; i < prog->ndeps; i++) {
      Dep *dep = prog->deps[i];
      // filter out RAR dependence
      if (IS_RAR(dep->type))
        continue;
      // dep is alreay satisfied, skip it
      if (loop->depth > dep->satisfaction_level)
        continue;
      // loop-carried dep
      if (dep->dirvec[loop->depth] != DEP_ZERO) {
        // examine if the stmt is reduction stmt
        int src_id, dest_id;
        for (src_id = 0; src_id < n_rdstmts; src_id++) {
          if (rdstmt_id[src_id] == dep->src + 1)
            break;
        }
        for (dest_id = 0; dest_id < n_rdstmts; dest_id++) {
          if (rdstmt_id[dest_id] == dep->dest + 1)
            break;
        }
        if (src_id == n_rdstmts || dest_id == n_rdstmts) {
          is_reduction = 0;
          break;
        }
      }
    }
  }

  // If the loop is not reduction loop and loop is not parallel, it's
  // impossible to do SIMD
  if (!is_reduction && !pluto_loop_is_parallel(prog, loop)) 
    return -1;

  score = 2 * pluto_loop_is_parallel(prog, loop) + 4 * is_reduction;

  // test access function
  int num_accs = 0;
  int i, j;  
  for (i = 0; i < loop->nstmts; i++) {
    Stmt *stmt = loop->stmts[i];    
    *psa_acc = realloc(*psa_acc, (num_accs + stmt->nreads + stmt->nwrites) * sizeof(PSAAccess *));
 
    for (j = 0; j < stmt->nreads; j++) {
      int is_layout_trans_required;
      int acc_simd_dim;
      bool ret = is_stride_zero_one(stmt, stmt->reads[j], loop->depth, &is_layout_trans_required, &acc_simd_dim);
      score += ret * (1 - is_layout_trans_required);

      (*psa_acc)[num_accs++] = (PSAAccess *)malloc(sizeof(PSAAccess));
      (*psa_acc)[num_accs - 1]->acc = stmt->reads[j];
      (*psa_acc)[num_accs - 1]->layout_trans = is_layout_trans_required;
      (*psa_acc)[num_accs - 1]->simd_dim = acc_simd_dim;
    }
    for (j = 0; j < stmt->nwrites; j++) {
      int is_layout_trans_required;
      int acc_simd_dim;
      bool ret = is_stride_zero_one(stmt, stmt->writes[j], loop->depth, &is_layout_trans_required, &acc_simd_dim);
      score += ret * (1 - is_layout_trans_required);
  
      (*psa_acc)[num_accs++] = (PSAAccess *)malloc(sizeof(PSAAccess));
      (*psa_acc)[num_accs - 1]->acc = stmt->writes[j];
      (*psa_acc)[num_accs - 1]->layout_trans = is_layout_trans_required;
      (*psa_acc)[num_accs - 1]->simd_dim = acc_simd_dim;
    }
  }
  *num_psa_acc = num_accs;
  return score;
}

///* This function calulates the score of each loop in terms of opportunities of SIMD vectorization.
// * Frist of all, the loop has to be either a parallel loop or a reduction loop.
// * For the parallel loop, we are currently limited in the case with one single statement.
// * If there is only one single statement inside the loop, we will ask users to mark if the statement
// * is a reduction statement. If so, the loop that carries RAW dep will be a reduction loop and can be put
// * in a SIMD vectorization canddiate loop.
// * Next, we will need to test if all the access functions in the loop has only 0/1-stride access. 
// * The score is calculated as:
// * score = Sigma_{all_accesses_under_the_loop} {is_access_0/1_stride * (1 - is_layout_transformation_required)} 
// *            + 2 * is_loop_parallel + 4 * is_loop_redunction
// * In the current heuristic, we favor reduction loop over parallel loop, because the reduction loop has less overhead
// * compared to the parallel loop. And we favor loops that doesn't lead to layout transformation than loops that require
// * the layout transformation.
// */
//int psa_is_loop_vectorizable(Ploop *loop, PlutoProg *prog, PSAAccess ***psa_acc, int *num_psa_acc) {
//  bool is_reduction = 0;
//  int score = 0;
//  /* check if the loop is a reduction loop with single statement */
//  if (loop->nstmts == 1) { 
//    fprintf(stdout, "[PSA] Trying to detect the reduction loop.\n");
//    fprintf(stdout, "[PSA] The statement to be analyzed: S%d\n", loop->stmts[0]->id + 1);
//    FILE *rdfile = fopen("reduce.annotation", "r");
//
//    if (!rdfile) {
//      fprintf(stdout, "[PSA] Annnotation file not found!\n");
//      fprintf(stdout, "[PSA] Please provide information about statement S%d in the file reduce.annotation.\n", loop->stmts[0]->id + 1);
//      return -1;
//    } else {
//      int *rdstmt_id = (int *)malloc(prog->nstmts * sizeof(int));
//      int n_rdstmts;
//      for (n_rdstmts = 0; n_rdstmts < loop->nstmts && !feof(rdfile); n_rdstmts++) {
//        fscanf(rdfile, "S%d", &rdstmt_id[n_rdstmts]);
//      }
//
//      /* Compare if the current stmt is a reduction statement */
//      for (int i = 0; i < n_rdstmts; i++) {
//        if (rdstmt_id[i] == loop->stmts[0]->id + 1)  {
//          is_reduction = 1;
//          break;
//        }
//      }
//    }
//  }
//
//  // If the loop is not reduction loop and loop is not parallel, it's
//  // impossible to do SIMD
//  if (!is_reduction && !pluto_loop_is_parallel(prog, loop)) 
//    return -1;
//
//  score = 2 * pluto_loop_is_parallel(prog, loop) + 4 * is_reduction;
//
//  // test access function
//  int num_accs = 0;
//  int i, j;  
//  for (i = 0; i < loop->nstmts; i++) {
//    Stmt *stmt = loop->stmts[i];    
//    *psa_acc = realloc(*psa_acc, (num_accs + stmt->nreads + stmt->nwrites) * sizeof(PSAAccess *));
// 
//    for (j = 0; j < stmt->nreads; j++) {
//      int is_layout_trans_required;
//      int acc_simd_dim;
//      bool ret = is_stride_zero_one(stmt, stmt->reads[j], loop->depth, &is_layout_trans_required, &acc_simd_dim);
//      score += ret * (1 - is_layout_trans_required);
//
//      (*psa_acc)[num_accs++] = (PSAAccess *)malloc(sizeof(PSAAccess));
//      (*psa_acc)[num_accs - 1]->acc = stmt->reads[j];
//      (*psa_acc)[num_accs - 1]->layout_trans = is_layout_trans_required;
//      (*psa_acc)[num_accs - 1]->simd_dim = acc_simd_dim;
//    }
//    for (j = 0; j < stmt->nwrites; j++) {
//      int is_layout_trans_required;
//      int acc_simd_dim;
//      bool ret = is_stride_zero_one(stmt, stmt->writes[j], loop->depth, &is_layout_trans_required, &acc_simd_dim);
//      score += ret * (1 - is_layout_trans_required);
//  
//      (*psa_acc)[num_accs++] = (PSAAccess *)malloc(sizeof(PSAAccess));
//      (*psa_acc)[num_accs - 1]->acc = stmt->writes[j];
//      (*psa_acc)[num_accs - 1]->layout_trans = is_layout_trans_required;
//      (*psa_acc)[num_accs - 1]->simd_dim = acc_simd_dim;
//    }
//  }
//  *num_psa_acc = num_accs;
//  return score;
//}

/* This function performs SIMD vectorization optimization */
int psa_simd_vectorization_optimize(PlutoProg *prog, VSA *vsa) {
  unsigned nbands, i;
  bool ret = 0;
  /* Grasp the innermost permutable band */
  Band **bands = pluto_get_innermost_permutable_bands(prog, &nbands);

#ifdef PSA_SIMD_VECTORIZATION_DEBUG
  fprintf(stdout, "[Debug] nband: %d\n", nbands);
#endif

  for (i = 0; i < nbands; i++) {
    ret |= psa_simd_vectorization_optimize_band(bands[i], prog);
  }

  /* Detect hyperplane types */
  pluto_detect_hyperplane_types(prog);
  pluto_detect_hyperplane_types_stmtwise(prog);
  psa_detect_hyperplane_types(prog, prog->array_dim, prog->array_part_dim);
  psa_detect_hyperplane_types_stmtwise(prog, prog->array_dim, prog->array_part_dim);    

  /* Detect properties after transformation */
  pluto_compute_dep_directions(prog);
  pluto_compute_dep_satisfaction(prog);
//  psa_compute_dep_distances_isl(prog);

  pluto_bands_free(bands, nbands);
  return ret;
}

void psa_simd_vectorization_misc_pretty_print(
  const PlutoProg *prog,
  int num_simd_loop,
  Ploop **simd_loops
) {
  int nstmts, i, j;
  nstmts = prog->nstmts;

  for (i = 0; i < nstmts; i++) {
    Stmt *stmt = prog->stmts[i];

    int *mark_loops = (int *)malloc(stmt->trans->nrows * sizeof(int));
    /* initialization */
    for (j = 0; j < stmt->trans->nrows; j++)
      mark_loops[j] = 0;

    for (j = 0; j < num_simd_loop; j++) {
      Ploop *loop = simd_loops[j];
      /* detect if the stmt is under the loop */
      if (pluto_stmt_is_member_of(stmt->id, loop->stmts, loop->nstmts))
        mark_loops[loop->depth] = 1;
    }

    fprintf(stdout, "T(S%d): ", stmt->id + 1);
    int level;
    fprintf(stdout, "(");
    for (level = 0; level < stmt->trans->nrows; level++) {
      pluto_stmt_print_hyperplane(stdout, stmt, level);
      if (level <= stmt->trans->nrows - 2)
        fprintf(stdout, ", ");
    }
    fprintf(stdout, ")\n");
    
    fprintf(stdout, "loop types (");
    for (level = 0; level < stmt->trans->nrows; level++) {
      if (level > 0)
        fprintf(stdout, ", ");
      if (stmt->hyp_types[level] == H_SCALAR)
        fprintf(stdout, "scalar");
      else if (stmt->hyp_types[level] == H_LOOP)
        fprintf(stdout, "loop");
      else if (stmt->hyp_types[level] == H_TILE_SPACE_LOOP)
        fprintf(stdout, "tloop");
      else
        fprintf(stdout, "unknown");
      // the candidate loops comes with an asterisk as the suffix
      if (mark_loops[level])
        fprintf(stdout, "*");
    }
    fprintf(stdout, ")\n");

    fprintf(stdout, "psa loop types (");
    for (level = 0; level < stmt->trans->nrows; level++) {
      if (level > 0)
        fprintf(stdout, ", ");
      if (IS_PSA_SCALAR(stmt->psa_hyp_types[level]))
        fprintf(stdout, "scalar");
      else if (IS_PSA_SPACE_LOOP(stmt->psa_hyp_types[level]))
        fprintf(stdout, "space_loop");
      else if (IS_PSA_TIME_LOOP(stmt->psa_hyp_types[level]))
        fprintf(stdout, "time_loop");
      else if (IS_PSA_ARRAY_PART_LOOP(stmt->psa_hyp_types[level]))
        fprintf(stdout, "array_part_loop");
      else if (IS_PSA_TASK_INTER_LOOP(stmt->psa_hyp_types[level]))
        fprintf(stdout, "task_inter_loop");
      else if (IS_PSA_SIMD_LOOP(stmt->psa_hyp_types[level]))
        fprintf(stdout, "simd_loop");
      else
        fprintf(stdout, "unknown");
    }
    fprintf(stdout, ")\n");

    free(mark_loops);
  }
}

/* 
 * Find the common loop iterator among all operand arrays (only appear in one 
 * index). If the loop is sync-free or pipelined (reduction), tile the loop and
 * move it innermost.
 * NOTE: currently only consider time loops
 */
int psa_simd_vectorization_optimize_band(Band *band, PlutoProg *prog) {  
  unsigned i, j, l, nloops;  
  Ploop **loops;

  /* Band has to be the innermost band as well */
  if (!pluto_is_band_innermost(band, 0)) {
    return PSA_FAILURE;
  }

  /* Mark the loop properties */
  unsigned first_space_hyp, first_time_hyp;
  bool first_array_part_hyp_found = false;
  bool first_space_hyp_found = false;
  bool first_time_hyp_found = false;
  for (i = 0; i < prog->num_hyperplanes; i++) {
    if (IS_PSA_ARRAY_PART_LOOP(prog->hProps[i].psa_type) &&
        !first_array_part_hyp_found) {
      first_array_part_hyp_found = true;
    } 
    if (IS_PSA_SPACE_LOOP(prog->hProps[i].psa_type) && !first_space_hyp_found) {
      first_space_hyp = i;
      first_space_hyp_found = true;
    }
    if (IS_PSA_TIME_LOOP(prog->hProps[i].psa_type) && !first_time_hyp_found) {
      first_time_hyp = i;
      first_time_hyp_found = true;
    }
  }

  /* Grasp all the loops in the current band */
  /* Note: We only consider time loops */
//  loops = pluto_get_loops_under(
//      band->loop->stmts, band->loop->nstmts,
//      PLMAX(band->loop->depth, first_time_hyp), prog, &nloops);
  loops = psa_get_loops_in_band(band, prog, &nloops);

  Ploop *best_loop = NULL;
  Ploop **simd_loops = NULL;
  int num_simd_loops = 0;
  int best_score = 0;

  PSAAccess **best_accs = NULL;
  int best_num_accs = 0;
  PSAAccess **accs = NULL;
  int num_accs = 0;

  for (i = 0; i < nloops; i++) {
    // we only consider time loop so far
    if (loops[i]->depth >= first_time_hyp) {
      // store before task inter or simd loop
      for (j = 0; j < loops[i]->nstmts; j++) {
        Stmt *stmt = loops[i]->stmts[j];
        PSAHypType hyp_type = stmt->psa_hyp_types[loops[i]->depth];
        if (IS_PSA_TASK_INTER_LOOP(hyp_type) || IS_PSA_SIMD_LOOP(hyp_type)) {
          break;
        }
      }
      if (j < loops[i]->nstmts) {
        break;
      }

      int score = psa_is_loop_vectorizable(loops[i], prog, &accs, &num_accs);
      if (score > 0) {
        num_simd_loops++;
        simd_loops = realloc(simd_loops, num_simd_loops * sizeof(Ploop *));
        simd_loops[num_simd_loops - 1] = loops[i];    
      }

      if (score > best_score) {
        best_score = score;
        best_loop = loops[i];
        best_accs = accs;
        best_num_accs = num_accs;
      }
    }
  }

#ifdef PSA_SIMD_VECTORIZATION_DEBUG
  fprintf(stdout, "[Debug] simd_loop: %d\n", num_simd_loops);
#endif

#ifdef PRINT_SIMD_VECTORIZATION_MISC
  psa_simd_vectorization_misc_pretty_print(
      prog,
      num_simd_loops, simd_loops);
  if (best_loop) {
    FILE *ltfile = fopen("layout_transform.info", "w");

    if (!ltfile) {
      for (int i = 0; i < best_num_accs; i++) {
        fprintf(ltfile, "access_name: %s\n", best_accs[i]->acc->name);
        fprintf(ltfile, "access_id: %d\n", best_accs[i]->acc->sym_id);
        fprintf(ltfile, "layout_transform: %d\n", best_accs[i]->layout_trans);
        fprintf(ltfile, "simd_dim: %d\n", best_accs[i]->simd_dim);
      }
    }

    fclose(ltfile);
  }
#endif

  if (best_loop) {
    /* Tile the loop and make the tiled loop the innermost */
    int tile_sizes[1];    
    tile_sizes[0] = DEFAULT_SIMD_TILE_FACTOR;    

    psa_read_simd_tile_sizes(tile_sizes, 1);

    /* complete array_simd_factor */
    prog->array_simd_factor = tile_sizes[0];

    // TODO
    //psa_tile_loop(prog, best_loop, tile_sizes[0], H_TILE_SPACE_LOOP, PSA_H_TIME_LOOP);
    psa_tile_loop_constant(prog, best_loop, tile_sizes[0], H_TILE_SPACE_LOOP, PSA_H_TIME_LOOP);
    
    int num_intra_tile_loops = 0;
    Ploop **intra_tile_loops = pluto_get_loops_immediately_inner(best_loop, prog, &num_intra_tile_loops);
    assert(num_intra_tile_loops == 1);
    
    band->width++;
    psa_make_innermost_loop_band(intra_tile_loops[0], band, prog);

    int last_depth = band->loop->depth + band->width - 1;

//    pluto_make_innermost_loop(intra_tile_loops[0], prog);
//
//    int last_depth = prog->num_hyperplanes - 1;
//    for (i = 0; i < best_loop->nstmts; i++) {
//      Stmt *stmt = best_loop->stmts[i];
//      int d;
//      for (d = best_loop->depth; d < stmt->trans->nrows; d++) {
//        if (pluto_is_hyperplane_scalar(stmt, d)) {
//          break;
//        }
//      }
//      last_depth = PLMIN(last_depth, d - 1);
//    }

    // update the hyperplane properties
    prog->hProps[last_depth].psa_type = PSA_H_SIMD_LOOP;
    for (i = 0; i < best_loop->nstmts; i++) {
      Stmt *stmt = best_loop->stmts[i];
      stmt->psa_hyp_types[last_depth] = PSA_H_SIMD_LOOP;
    }

    return PSA_SUCCESS;
  } else {
    /* no opportunity for SIMD vectorization */
    return PSA_FAILURE;
  }
}

/*
 * Apply techniques in FCCM 2018 to eliminate interior I/O
 * TODO: to be completed.
 * Currently assign on interior I/O to "D" direction.
 */
int psa_pe_io_eliminate_optimize(Band *band, PlutoProg *prog, VSA *vsa) {
  int i, j;
  
  /* Detect if there exists any interior I/Os */
  for (i = 0; i < vsa->op_num; i++) {
    if (!strcmp(vsa->op_channel_dirs[i], "I")) {
      break;
    }
  }

  for (j = 0; j < vsa->res_num; j++) {
    if (!strcmp(vsa->res_channel_dirs[j], "I")) {
      break;
    }
  }

  if ((i != vsa->op_num) || (j != vsa->res_num)) {
    prog->array_io_enable = 1;
  } else {
    prog->array_io_enable = 0;
  }

  vsa->op_io_enable = (bool *)malloc(vsa->op_num * sizeof(bool));
  vsa->res_io_enable = (bool *)malloc(vsa->res_num * sizeof(bool));

  if (prog->array_io_enable) {
    for (i = 0; i < vsa->op_num; i++) {
      if (!strcmp(vsa->op_channel_dirs[i], "I")) {
        vsa->op_io_enable[i] = 1;
        vsa->op_channel_dirs[i] = "D";
      } else {
        vsa->op_io_enable[i] = 0;
      }
    }

    for (i = 0; i < vsa->res_num; i++) {
      if (!strcmp(vsa->res_channel_dirs[i], "I")) {
        vsa->res_io_enable[i] = 1;
        vsa->res_channel_dirs[i] = "D";
      } else {
        vsa->res_io_enable[i] = 0;
      }
    }
  }

  return 1;
}

/* This function performs latency hiding optimization */
int psa_latency_hiding_optimize(PlutoProg *prog, VSA *vsa) {
  unsigned nbands, i;
  bool ret = 0;
  /* Grasp the innermost permutable band */
  Band **bands = pluto_get_innermost_permutable_bands(prog, &nbands);
  
  for (i = 0; i < nbands; i++) {
    ret |= psa_latency_hiding_optimize_band(bands[i], prog);
  }

  /* Detect hyperplane types */
  pluto_detect_hyperplane_types(prog);
  pluto_detect_hyperplane_types_stmtwise(prog);
  psa_detect_hyperplane_types(prog, prog->array_dim, prog->array_part_dim);
  psa_detect_hyperplane_types_stmtwise(prog, prog->array_dim, prog->array_part_dim);    

  /* Detect properties after transformation */
  pluto_compute_dep_directions(prog);
  pluto_compute_dep_satisfaction(prog);
//  psa_compute_dep_distances_isl(prog);

  pluto_bands_free(bands, nbands);
  return ret;
}

/*
 * Intra-PE optimization.
 * Task interleaving, SIMD vectorization, and interior I/O optimization
 */
void psa_pe_optimize(PlutoProg *prog, VSA *vsa) {  
  unsigned nbands, i;
  int ret1, ret2, ret3;  
  Band **bands = pluto_get_outermost_permutable_bands(prog, &nbands);
  assert(nbands == 1);

//#ifdef JIE_DEBUG
//  fprintf(stdout, "[Debug] num_hyp: %d\n", prog->num_hyperplanes);
//  fprintf(stdout, "[Debug] depth: %d\n", bands[0]->loop->depth);
//#endif

//  /* Task Interleaving */  
//  fprintf(stdout, "[PSA] Apply task interleaving.\n");
//  for (i = 0; i < nbands; i++) {
//    ret1 = psa_pe_task_interleave_optimize(bands[i], prog);
//    if (ret1 == 0) {
//      fprintf(stdout, "[PSA] Completed task interleaving optimizaiton.\n");
//      /* Update the fields of VSA */
//      prog->array_il_enable = 1;      
//    } else {
//      fprintf(stdout, "[PSA] Failed task interleaving optimization.\n");
//      prog->array_il_enable = 0;
//    }
//  }
//
//#ifdef JIE_DEBUG
//  for (i = 0; i < prog->num_hyperplanes; i++) {
//    if (IS_PSA_TASK_INTER_LOOP(prog->hProps[i].psa_type)) {
//      fprintf(stdout, "[Debug] after task interleave hyp %d is TASK_INTER_LOOP\n", i);
//    }
//  }
//#endif

//  /* Detect hyperplane types */
//  pluto_detect_hyperplane_types(prog);
//  pluto_detect_hyperplane_types_stmtwise(prog);
//  psa_detect_hyperplane_types(prog, prog->array_dim, prog->array_part_dim);
//  psa_detect_hyperplane_types_stmtwise(prog, prog->array_dim, prog->array_part_dim);    
//
//  /* Detect properties after transformation */
//  pluto_compute_dep_directions(prog);
//  pluto_compute_dep_satisfaction(prog);
//  psa_compute_dep_distances(prog);

//  if (!options->silent) {
//    fprintf(stdout, "[PSA] After task interleaving.\n");
//    pluto_transformations_pretty_print(prog);
//  }

//  /* SIMD Vectorization */
//  fprintf(stdout, "[PSA] Apply SIMD vectorization.\n");  
//  bands = pluto_get_outermost_permutable_bands(prog, &nbands);
//  for (i = 0; i < nbands; i++) {
//    ret2 = psa_pe_simd_optimize(bands[i], prog);
//    if (ret2 == 0) {
//      fprintf(stdout, "[PSA] Completed SIMD vectorization.\n");
//    } else {
//      fprintf(stdout, "[PSA] Failed SIMD vectorization.\n");
//    }
//  }
//
//  /* Detect hyperplane types */
//  pluto_detect_hyperplane_types(prog);
//  pluto_detect_hyperplane_types_stmtwise(prog);
//  psa_detect_hyperplane_types(prog, prog->array_dim, prog->array_part_dim);
//  psa_detect_hyperplane_types_stmtwise(prog, prog->array_dim, prog->array_part_dim);    
//
//  /* Detect properties after transformation */
//  pluto_compute_dep_directions(prog);
//  pluto_compute_dep_satisfaction(prog);
//  psa_compute_dep_distances(prog);
//
//  if (!options->silent) {
//    fprintf(stdout, "[PSA] After SIMD vectorization.\n");
//    pluto_transformations_pretty_print(prog);
//  }

  /* Interior I/O Elimination */
  fprintf(stdout, "[PSA] Apply interior I/O elimination.\n");
  bands = pluto_get_outermost_permutable_bands(prog, &nbands);
  for (i = 0; i < nbands; i++) {
    ret3 = psa_pe_io_eliminate_optimize(bands[i], prog, vsa);
    if (ret3 == 0) {
      fprintf(stdout, "[PSA] Completed Interior I/O elimination.");      
    } else {
      fprintf(stdout, "[PSA] Failed interior I/O elimination.\n");
    }
  }

  /* Detect hyperplane types */
  pluto_detect_hyperplane_types(prog);
  pluto_detect_hyperplane_types_stmtwise(prog);
  psa_detect_hyperplane_types(prog, prog->array_dim, prog->array_part_dim);
  psa_detect_hyperplane_types_stmtwise(prog, prog->array_dim, prog->array_part_dim);      

  /* Detect properties after transformation */
  pluto_compute_dep_directions(prog);
  pluto_compute_dep_satisfaction(prog);
  psa_compute_dep_distances_isl(prog);

  pluto_bands_free(bands, nbands);
}
