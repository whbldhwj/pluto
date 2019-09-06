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

/* Read tile sizes from tile.sizes */
int psa_read_tile_sizes(
  int *tile_sizes,
  int num_tile_dims, Stmt **stmts, int nstmts, int first_loop
) {
  int i, j;

  FILE *tsfile = fopen("tile.sizes", "r");

  if (!tsfile)
    return 0;

  for (i = 0; i < num_tile_dims && !feof(tsfile); i++) {
    for (j = 0; j < nstmts; j++) {
      if (pluto_is_hyperplane_loop(stmts[j], first_loop + i))
        break;
    }
    bool is_loop = (j < nstmts);
    if (is_loop) {
      fscanf(tsfile, "%d", &tile_sizes[i]);
    } else {
      /* Size set for scalar dimension doesn't matter */
      tile_sizes[i] = 42;
    }
  }

  if (i < num_tile_dims) {
    fprintf(stdout, "[PSA] WARNING: not enough tile sizes provided! The required number: %d\n", num_tile_dims);
    fclose(tsfile);
    return PSA_FAILURE;
  }

  fclose(tsfile);
  return PSA_SUCCESS;
}

/* Manipulate statement domain and transformation to tile scattering dimesnions 
 * from firstD to lastD */
int psa_array_partition_tile_band(PlutoProg *prog, Band *band, int *tile_sizes) {
  int i, j, s;
  int depth, npar;

  npar = prog->npar;

  int firstD = band->loop->depth;
  int lastD = band->loop->depth + band->width - 1;

  int num_domain_supernodes[band->loop->nstmts]; // store the number of domain supernodes for each stmt

  for (s = 0; s < band->loop->nstmts; s++) {
    num_domain_supernodes[s] = 0;
  }

  int array_part_dim = 0;
  int skipped_loop = 0;

  // tile the loops;
  for (depth = firstD; depth <= lastD; depth++) {
    PSAHypType hyp_type = prog->hProps[depth].psa_type;
    // stop at the latency hiding / simd loop
    if (IS_PSA_TASK_INTER_LOOP(hyp_type) || IS_PSA_SIMD_LOOP(hyp_type)) {
      skipped_loop++;
      break;
    }

    for (s = 0; s < band->loop->nstmts; s++) {
      Stmt *stmt = band->loop->stmts[s];
      /* 1. Specify tiles in the original domain */
      /* 1.1 Add additional dimensions */
      char iter[6];
      sprintf(iter, "zT%d", stmt->dim);      

      int hyp_type = (stmt->hyp_types[depth + depth - firstD - skipped_loop] == H_SCALAR)
                          ? H_SCALAR
                          : H_TILE_SPACE_LOOP;

      int psa_hyp_type = (stmt->psa_hyp_types[depth + depth - firstD - skipped_loop] == PSA_H_SCALAR)
                          ? PSA_H_SCALAR
                          : PSA_H_ARRAY_PART_LOOP;

      /* 1.2 Specify tile shapes in the original domain */
      /* Domain supernodes aren't added for scalar dimensions */
      if (hyp_type != H_SCALAR) {
        assert(tile_sizes[depth - firstD - skipped_loop] >= 1);
        pluto_stmt_add_dim(stmt, num_domain_supernodes[s], depth - skipped_loop, iter,
                            hyp_type, psa_hyp_type, prog);

        /* Add relation b/w tile space variable and intra-tile variables like
         * 32*xt <= 2t+i <= 32xt + 31 */
        /* Lower bound */
        pluto_constraints_add_inequality(stmt->domain);

        for (j = num_domain_supernodes[s] + 1; j < stmt->dim + npar; j++) {
          stmt->domain->val[stmt->domain->nrows - 1][j] = 
            stmt->trans->val[1 + (depth - firstD - skipped_loop) + depth][j];
        }

        stmt->domain->val[stmt->domain->nrows - 1][num_domain_supernodes[s]] = 
          -tile_sizes[depth - skipped_loop - firstD];
        
        stmt->domain->val[stmt->domain->nrows - 1][stmt->domain->ncols - 1] = 
          stmt->trans->val[1 + (depth - skipped_loop - firstD) + depth][stmt->dim + prog->npar];

        PlutoConstraints *lb =        
          pluto_constraints_select_row(stmt->domain, stmt->domain->nrows - 1);
        /* Add the new constraints into dependence polyhedron */
        pluto_update_deps(stmt, lb, prog); 
        pluto_constraints_free(lb);

        /* Upper bound */
        pluto_constraints_add_inequality(stmt->domain);
        for (j = num_domain_supernodes[s] + 1; j < stmt->dim + npar; j++) {
          stmt->domain->val[stmt->domain->nrows - 1][j] = 
            -stmt->trans->val[1 + (depth - skipped_loop - firstD) + depth][j];
        }

        stmt->domain->val[stmt->domain->nrows - 1][num_domain_supernodes[s]] =
          tile_sizes[depth - skipped_loop - firstD];

        stmt->domain->val[stmt->domain->nrows - 1][stmt->domain->ncols - 1] = 
          -stmt->trans->val[1 + (depth - skipped_loop - firstD) + depth][stmt->dim + prog->npar] + 
          tile_sizes[depth - skipped_loop - firstD] - 1;

        PlutoConstraints *ub =
          pluto_constraints_select_row(stmt->domain, stmt->domain->nrows - 1);
        /* Add the new constraints into dependence polyhedron */
        pluto_update_deps(stmt, ub, prog);
        pluto_constraints_free(ub);

        num_domain_supernodes[s]++;

      } else {
        /* Scattering function for the tile space iterator is set the same as its
         * associated domain iterator. Dimension is not a loop; tile it trivially
         */
        pluto_stmt_add_hyperplane(stmt, H_SCALAR, PSA_H_SCALAR, depth - skipped_loop);
        for (j = 0; j < stmt->dim + npar + 1; j++) {
          stmt->trans->val[depth - skipped_loop][j] = 
            stmt->trans->val[1 + (depth - skipped_loop - firstD) + depth][j];
        }
      } 
      stmt->num_tiled_loops++; // not used
      stmt->first_tile_dim = firstD; // not used
      stmt->last_tile_dim = lastD; // not used
    } /* all statements */

    array_part_dim++;
  }   /* all scats to be tiled */

  prog->array_part_dim = array_part_dim;
  return array_part_dim;
}

/* Manipulate statement domain and transformation to tile scattering dimesnions 
 * from firstD to lastD */
int psa_tile_band_constant(PlutoProg *prog, Band *band, int *tile_sizes, 
    PlutoHypType htype, PSAHypType psa_htype) {
  int i, j, s;
  int depth, npar;

  npar = prog->npar;

  int firstD = band->loop->depth;
  int lastD = band->loop->depth + band->width - 1;

  int num_domain_supernodes[band->loop->nstmts]; // store the number of domain supernodes for each stmt

  for (s = 0; s < band->loop->nstmts; s++) {
    num_domain_supernodes[s] = 0;
  }

  int array_part_dim = 0;

  // tile the loops;
  for (depth = firstD; depth <= lastD; depth++) {
    for (s = 0; s < band->loop->nstmts; s++) {
      Stmt *stmt = band->loop->stmts[s];
      /* 1. Specify tiles in the original domain */
      /* 1.1 Add additional dimensions */
      char iter[6];
      sprintf(iter, "oT%d", stmt->dim);      

      int hyp_type = (stmt->hyp_types[depth + depth - firstD] == H_SCALAR)
                          ? H_SCALAR
                          : htype;

      int psa_hyp_type = (stmt->psa_hyp_types[depth + depth - firstD] == PSA_H_SCALAR)
                          ? PSA_H_SCALAR
                          : psa_htype;

      /* 1.2 Specify tile shapes in the original domain */
      /* Domain supernodes aren't added for scalar dimensions */
      if (hyp_type != H_SCALAR) {
        assert(tile_sizes[depth - firstD] >= 1);
        pluto_stmt_add_dim(stmt, num_domain_supernodes[s], depth, iter,
                            hyp_type, psa_hyp_type, prog);
        num_domain_supernodes[s]++;

        /* Add relation b/w tile space variable and intra-tile variables like
         * 32*xt <= 2t+i <= 32xt + 31 */
        /* Lower bound */
        pluto_constraints_add_inequality(stmt->domain);

        for (j = num_domain_supernodes[s]; j < stmt->dim + npar + 1; j++) {
          stmt->domain->val[stmt->domain->nrows - 1][j] = 
            stmt->trans->val[1 + (depth - firstD) + depth][j];
        }

        stmt->domain->val[stmt->domain->nrows - 1][num_domain_supernodes[s] - 1] = 
          -tile_sizes[depth - firstD];
        
        PlutoConstraints *lb =        
          pluto_constraints_select_row(stmt->domain, stmt->domain->nrows - 1);
        /* Add the new constraints into dependence polyhedron */
        pluto_update_deps(stmt, lb, prog); 
        pluto_constraints_free(lb);

        /* Upper bound */
        pluto_constraints_add_inequality(stmt->domain);
        for (j = num_domain_supernodes[s]; j < stmt->dim + npar + 1; j++) {
          stmt->domain->val[stmt->domain->nrows - 1][j] = 
            -stmt->trans->val[1 + (depth - firstD) + depth][j];
        }

        stmt->domain->val[stmt->domain->nrows - 1][num_domain_supernodes[s] - 1] =
          tile_sizes[depth - firstD];

        stmt->domain->val[stmt->domain->nrows - 1][stmt->domain->ncols - 1] += 
          tile_sizes[depth - firstD] - 1;

        PlutoConstraints *ub =
          pluto_constraints_select_row(stmt->domain, stmt->domain->nrows - 1);
        /* Add the new constraints into dependence polyhedron */
        pluto_update_deps(stmt, ub, prog);
        pluto_constraints_free(ub);

        // resetting the shceduling row for 2t+i
        // phi = 2t+i - 32*oT
        stmt->trans->val[1 + (depth - firstD) + depth][num_domain_supernodes[s] - 1] = -tile_sizes[depth - firstD];
      } else {
        /* Scattering function for the tile space iterator is set the same as its
         * associated domain iterator. Dimension is not a loop; tile it trivially
         */
        pluto_stmt_add_hyperplane(stmt, H_SCALAR, PSA_H_SCALAR, depth);
        for (j = 0; j < stmt->dim + npar + 1; j++) {
          stmt->trans->val[depth][j] = 
            stmt->trans->val[1 + (depth - firstD) + depth][j];
        }
      } 
      stmt->num_tiled_loops++; // not used
      //stmt->first_tile_dim = firstD; // not used
      //stmt->last_tile_dim = lastD; // not used
    } /* all statements */

    array_part_dim++;
  }   /* all scats to be tiled */

  /* Sink everything to the same depth */
  int max = 0, curr;
  for (i = 0; i < prog->nstmts; i++) {
    max = PLMAX(prog->stmts[i]->trans->nrows, max);    
  }
  for (i = 0; i < prog->nstmts; i++) {
    curr = prog->stmts[i]->trans->nrows;
    for (j = curr; j < max; j++) {
      pluto_sink_transformation(prog->stmts[i], prog->stmts[i]->trans->nrows, prog);
    }
  }

  curr = prog->num_hyperplanes;  
  for (depth = curr; depth < max; depth++) {
    pluto_prog_add_hyperplane(prog, depth, H_UNKNOWN, PSA_H_UNKNOWN);
  }  

  return array_part_dim;
}


/*
 * Tile the loops in the outermost permutable loop band
 */
int psa_array_partition_optimize_band(PlutoProg *prog, Band *band) {
  int i, j;
  int tile_sizes[band->width];
  int depth;

  Stmt **stmts = prog->stmts;

  for (i = 0; i < band->width; i++) {
    tile_sizes[i] = DEFAULT_L1_TILE_SIZE;
  }

  /* Read tile sizes from File: tile.sizes if exists */
  psa_read_tile_sizes(
    tile_sizes, band->width, band->loop->stmts, 
    band->loop->nstmts, band->loop->depth
  );

  /* Print out the array partition loop candidates */
  // TODO: to fix this function grasp all the loops, which may be beyond the scope of the band
  Ploop **loops = NULL;
  int nloops = 0;
//  loops = pluto_get_loops_under(
//      band->loop->stmts, band->loop->nstmts,
//      band->loop->depth, prog, &nloops);
  loops = psa_get_loops_in_band(band, prog, &nloops);

  int num_array_part_loops = 0;
  Ploop **array_part_loops = NULL;
  for (i = 0; i < nloops; i++) {
    Ploop *loop = loops[i];
//    PSAHypType hyp_type = prog->hProps[loop->depth].psa_type;
//    if (IS_PSA_TASK_INTER_LOOP(hyp_type) || IS_PSA_SIMD_LOOP(hyp_type)) {
//      break;
//    }
    for (j = 0; j < loop->nstmts; j++) {
      Stmt *stmt = loop->stmts[j];
      PSAHypType hyp_type = stmt->psa_hyp_types[loop->depth];
      if (IS_PSA_TASK_INTER_LOOP(hyp_type) || IS_PSA_SIMD_LOOP(hyp_type)) {
        break;
      }
    }
    if (j < loop->nstmts)
      break;

    num_array_part_loops++;
    array_part_loops = realloc(array_part_loops, num_array_part_loops * sizeof(Ploop *));
    array_part_loops[num_array_part_loops - 1] = loop;
  }
#ifdef PSA_ARRAY_PARTITIONING_DEBUG
  fprintf(stdout, "[Debug] Hey.\n");
#endif

#ifdef PRINT_ARRAY_PARTITIONING_MISC

  #ifdef PSA_ARRAY_PARTITIONING_DEBUG
  fprintf(stdout, "[Debug] Enter the print misc.\n");
  #endif

  psa_array_partitioning_misc_pretty_print(
      prog, 
      num_array_part_loops, array_part_loops);
#endif
//  pluto_loops_free(array_part_loops, num_array_part_loops);
//  pluto_loops_free(loops, nloops);

  /* Tile the band */
  band->width = num_array_part_loops;
  //int num_tiled_loops = psa_array_partition_tile_band(prog, band, tile_sizes);
  int num_tiled_loops = psa_tile_band_constant(prog, band, tile_sizes, H_TILE_SPACE_LOOP, PSA_H_ARRAY_PART_LOOP);

  prog->array_part_dim = num_tiled_loops;
  /* Interpret the sa_rows and sa_cols */
  if (prog->array_dim == 1) {
    prog->array_nrow = 1;
    prog->array_ncol = tile_sizes[0];
  } else if (prog->array_dim == 2) {
    prog->array_nrow = tile_sizes[0];
    prog->array_ncol = tile_sizes[1];
  }

  /* Free Memory */
  for (i = 0; i < nloops; i++) {
    pluto_loop_free(loops[i]);
  }
  free(loops);
  free(array_part_loops);
  /* Free Memory */

  return num_tiled_loops;
}

void psa_array_partitioning_misc_pretty_print(
  const PlutoProg *prog,
  int num_array_part_loops,
  Ploop **array_part_loops
) {
  int nstmts, i, j;
  nstmts = prog->nstmts;

  for (i = 0; i < nstmts; i++) {
    Stmt *stmt = prog->stmts[i];

    int *mark_loops = (int *)malloc(stmt->trans->nrows * sizeof(int));
    /* initialization */
    for (j = 0; j < stmt->trans->nrows; j++)
      mark_loops[j] = 0;

    for (j = 0; j < num_array_part_loops; j++) {
      Ploop *loop = array_part_loops[j];
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
 * Permute the inter-tile loops for the outermost permutable loop band
 * Place sync-free loops before pipelined loops to reduce communication (w/o RAR)
 * TODO: to be fixed
 */
void psa_reorg_array_partition_band(PlutoProg *prog, Band *band) {
  int i, j;
  int *dep_dis_hyp = (int *)malloc(band->width * sizeof(int));  
  /* Initialization */
  for (i = 0; i < band->width; i++) {
    dep_dis_hyp[i] = -2;
  }

  int firstD = band->loop->depth + band->width;
  // int lastD = band->loop->depth + band->width + band->width - 1;

  /* Calculate loop-carried dependence distance (excluding RAR) */
  for (i = firstD; i < firstD + band->width; i++) {
    if (IS_PSA_ARRAY_PART_LOOP(prog->hProps[i].psa_type))
    for (j = 0; j < prog->ndeps; j++) {
      Dep *dep = prog->deps[j];
      assert(dep->disvec != NULL);
      int dep_dis;
      switch (dep->disvec[i]) {
        case -1:
          dep_dis = 1;
          break;
        case 0:
          dep_dis = 0;
          break;
        case 1:
          dep_dis = 1;
          break;
        default:
          dep_dis = 2;
          break;
      }
      if (!IS_RAR(dep->type)) {
        dep_dis_hyp[i] = PLMAX(dep_dis_hyp[i], dep_dis);
      }
    }
  }

  /* Sort dep_dis_hyp in ascending order
   * Bubble sort
   * If the dep_dis of the current hyperplane is greater than the next one, 
   * permute them
   */
  Ploop *loop = band->loop;
  for (i = 0; i < band->width - 1; i++) {
    for (j = 0; j < band->width - i - 1; j++) {
      if (dep_dis_hyp[j] > dep_dis_hyp[j + 1]) {
        unsigned s;
        for (s = 0; s < loop->nstmts; s++) {
          Stmt *stmt = loop->stmts[s];
          pluto_stmt_loop_interchange(stmt, j, j + 1, prog);
        }
      }
    }
  }

  free(dep_dis_hyp);
}

void psa_reorg_array_part_loops(PlutoProg *prog) {

}

/*
 * Apply array partitioninig.
 * Apply loop tiling on the outermost permutable loop bands (ignore loops that 
 * are intra-tiled loops, for task-interleaving, SIMD vectorization)
 * Place sync-free loops before pipelined loops to reduce communication. */
int psa_array_partition_optimize(PlutoProg *prog, VSA *vsa) {
  /* Get the outermost permutable loop band */
  unsigned nbands;
  Band **bands;
  bands = pluto_get_outermost_permutable_bands(prog, &nbands);
  assert(nbands == 1);

  /* Tile the array partition candidate loops */  
  int ret = psa_array_partition_optimize_band(prog, bands[0]);  

  /* Detect properties after tiling */
  pluto_compute_dep_directions(prog);
  pluto_compute_dep_satisfaction(prog);
  psa_compute_dep_distances_isl(prog);

  /* Re-detect hyperplane types (H_SCALAR, H_LOOP) */
  pluto_detect_hyperplane_types(prog);
  pluto_detect_hyperplane_types_stmtwise(prog);
  psa_detect_hyperplane_types_stmtwise(prog, prog->array_dim, prog->array_part_dim);
  psa_detect_hyperplane_types(prog, prog->array_dim, prog->array_part_dim);  

  /* Permuate the inter-tile loops.
   * Place sync-free loops before pipelined loops to reduce communication.
   */
  // TODO
  //psa_reorg_array_partition_band(prog, bands[0]);  

  /* Detect properties after permutation */
  pluto_compute_dep_directions(prog);
  pluto_compute_dep_satisfaction(prog);
  psa_compute_dep_distances_isl(prog);  

  /* Re-detect hyperplane types (H_SCALAR, H_LOOP) */
  pluto_detect_hyperplane_types(prog);
  pluto_detect_hyperplane_types_stmtwise(prog);
  psa_detect_hyperplane_types_stmtwise(prog, prog->array_dim, prog->array_part_dim);
  psa_detect_hyperplane_types(prog, prog->array_dim, prog->array_part_dim);  

  pluto_bands_free(bands, nbands);

  if (ret > 0) {
    return PSA_SUCCESS;
  } else {
    return PSA_FAILURE;
  }
}
