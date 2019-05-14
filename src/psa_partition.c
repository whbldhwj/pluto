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
    return 0;
  }

  fclose(tsfile);
  return 1;
}

/* Manipulate statement domain and transformation to tile scattering dimesnions 
 * from firstD to lastD */
void psa_tile_band(PlutoProg *prog, Band *band, int *tile_sizes) {
  int j, s;
  int depth, npar;

  npar = prog->npar;

  int firstD = band->loop->depth;
  int lastD = band->loop->depth + band->width - 1;

  int num_domain_supernodes[band->loop->nstmts];

  for (s = 0; s < band->loop->nstmts; s++) {
    num_domain_supernodes[s] = 0;
  }

  for (depth = firstD; depth <= lastD; depth++) {
    for (s = 0; s < band->loop->nstmts; s++) {
      Stmt *stmt = band->loop->stmts[s];
      /* 1. Specify tiles in the original domain */
      /* 1.1 Add additional dimensions */
      char iter[6];
      sprintf(iter, "zT%d", stmt->dim);

      int hyp_type = (stmt->hyp_types[depth + depth - firstD] == H_SCALAR)
                          ? H_SCALAR
                          : H_TILE_SPACE_LOOP;

      int psa_hyp_type = (stmt->psa_hyp_types[depth + depth - firstD] == PSA_H_SCALAR)
                          ? PSA_H_SCALAR
                          : PSA_H_ARRAY_PART_LOOP;

      /* 1.2 Specify tile shapes in the original domain */
      /* Domain supernodes aren't added for scalar dimensions */
      if (hyp_type != H_SCALAR) {
        assert(tile_sizes[depth - firstD] >= 1);
        pluto_stmt_add_dim(stmt, num_domain_supernodes[s], depth, iter,
                            hyp_type, psa_hyp_type, prog);

        /* Add relation b/w tile space variable and intra-tile variables like
         * 32*xt <= 2t+i <= 32xt + 31 */
        /* Lower bound */
        pluto_constraints_add_inequality(stmt->domain);

        for (j = num_domain_supernodes[s] + 1; j < stmt->dim + npar; j++) {
          stmt->domain->val[stmt->domain->nrows - 1][j] = 
            stmt->trans->val[1 + (depth - firstD) + depth][j];
        }

        stmt->domain->val[stmt->domain->nrows - 1][num_domain_supernodes[s]] = 
          -tile_sizes[depth - firstD];
        
        stmt->domain->val[stmt->domain->nrows - 1][stmt->domain->ncols - 1] = 
          stmt->trans->val[1 + (depth - firstD) + depth][stmt->dim + prog->npar];

        PlutoConstraints *lb =        
          pluto_constraints_select_row(stmt->domain, stmt->domain->nrows - 1);
        /* Add the new constraints into dependence polyhedron */
        pluto_update_deps(stmt, lb, prog); 
        pluto_constraints_free(lb);

        /* Upper bound */
        pluto_constraints_add_inequality(stmt->domain);
        for (j = num_domain_supernodes[s] + 1; j < stmt->dim + npar; j++) {
          stmt->domain->val[stmt->domain->nrows - 1][j] = 
            -stmt->trans->val[1 + (depth - firstD) + depth][j];
        }

        stmt->domain->val[stmt->domain->nrows - 1][num_domain_supernodes[s]] =
          tile_sizes[depth - firstD];

        stmt->domain->val[stmt->domain->nrows - 1][stmt->domain->ncols - 1] = 
          -stmt->trans->val[1 + (depth - firstD) + depth][stmt->dim + prog->npar] + 
          tile_sizes[depth - firstD] - 1;

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
        pluto_stmt_add_hyperplane(stmt, H_SCALAR, PSA_H_SCALAR, depth);
        for (j = 0; j < stmt->dim + npar + 1; j++) {
          stmt->trans->val[depth][j] = 
            stmt->trans->val[1 + (depth - firstD) + depth][j];
        }
      } 
      stmt->num_tiled_loops++;
      stmt->first_tile_dim = firstD;
      stmt->last_tile_dim = lastD;
    } /* all statements */
  }   /* all scats to be tiled */
}

/*
 * Tile the loops in the outermost permutable loop band
 */
void psa_tile_outermost_permutable_band(PlutoProg *prog, Band *band) {
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

  /* Tile the band */
  psa_tile_band(prog, band, tile_sizes);

  /* Sink everything to the same depth */
  int max = 0, curr;
  for (i = 0; i < prog->nstmts; i++) {
    max = PLMAX(stmts[i]->trans->nrows, max);    
  }
  for (i = 0; i < prog->nstmts; i++) {
    curr = stmts[i]->trans->nrows;
    for (j = curr; j < max; j++) {
      pluto_sink_transformation(stmts[i], stmts[i]->trans->nrows, prog);
    }
  }

  curr = prog->num_hyperplanes;  
  for (depth = curr; depth < max; depth++) {
    pluto_prog_add_hyperplane(prog, depth, H_UNKNOWN, PSA_H_UNKNOWN);
  }  

  /* Re-detect hyperplane types (H_SCALAR, H_LOOP) */
  pluto_detect_hyperplane_types(prog);
  pluto_detect_hyperplane_types_stmtwise(prog);
  psa_detect_hyperplane_types_stmtwise(prog, prog->array_dim, prog->array_part_dim);
  psa_detect_hyperplane_types(prog, prog->array_dim, prog->array_part_dim);  
}

/*
 * Permute the inter-tile loops for the outermost permutable loop band
 * Place sync-free loops before pipelined loops to reduce communication (w/o RAR)
 */
void psa_permute_outermost_tile_band(PlutoProg *prog, Band *band) {
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
    for (j = 0; j < prog->ndeps; j++) {
      Dep *dep = prog->deps[j];
      assert(dep->disvec != NULL);
      int dep_dis;
      switch (dep->disvec[i]) {
        case DEP_DIS_MINUS_ONE:
          dep_dis = 1;
          break;
        case DEP_DIS_ZERO:
          dep_dis = 0;
          break;
        case DEP_DIS_PLUS_ONE:
          dep_dis = 1;
          break;
        case DEP_DIS_STAR:
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

/*
 * Apply array partitioninig.
 * Apply loop tiling on the outermost permutable loop bands.
 * Place sync-free loops before pipelined loops to reduce communication. */
void psa_array_partition(PlutoProg *prog) {
  /* Get the outermost permutable loop band */
  unsigned nbands;
  Band **bands;
  bands = pluto_get_outermost_permutable_bands(prog, &nbands);

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] loop band number: %d\n", nbands);
//   fprintf(stdout, "[Debug] loop band width: %d\n", bands[0]->width);
// #endif

  /* Tile the outermost permutable loop band */
  assert(nbands == 1);
  psa_tile_outermost_permutable_band(prog, bands[0]);  

  /* Detect properties after tiling */
  pluto_compute_dep_directions(prog);
  pluto_compute_dep_satisfaction(prog);
  psa_compute_dep_distances(prog);

  /* Permuate the inter-tile loops.
   * Place sync-free loops before pipelined loops to reduce communication.
   */
  psa_permute_outermost_tile_band(prog, bands[0]);

  /* Detect properties after permutation */
  pluto_compute_dep_directions(prog);
  pluto_compute_dep_satisfaction(prog);
  psa_compute_dep_distances(prog);  

  if (!options->silent) {
    fprintf(stdout, "[Pluto] After tiling:\n");
    pluto_transformations_pretty_print(prog);
  }

  pluto_bands_free(bands, nbands);
}