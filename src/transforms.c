/*
 * PLUTO: An automatic parallelizer and locality optimizer
 *
 * Copyright (C) 2007-2012 Uday Bondhugula
 *
 * This file is part of Pluto.
 *
 * Pluto is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * A copy of the GNU General Public Licence can be found in the file
 * `LICENSE' in the top-level directory of this distribution.
 *
 */
#include "program.h"
#include "pluto.h"
#include "constraints.h"
#include "transforms.h"

#include "assert.h"

/* Sink statement (domain); depth: 0-indexed */
void pluto_sink_statement(Stmt *stmt, int depth, int val, PlutoProg *prog) {
  assert(stmt->dim == stmt->domain->ncols - prog->npar - 1);

  char iter[13];
  snprintf(iter, sizeof(iter), "d%d", stmt->dim);

  pluto_stmt_add_dim(stmt, depth, -1, iter, H_SCALAR, PSA_H_SCALAR, prog);

  pluto_constraints_set_var(stmt->domain, depth, val);
  stmt->is_orig_loop[depth] = false;
}

/* Stripmine 'dim'th time dimension of stmt by stripmine factor; use
 * 'supernode' as the name of the supernode in the domain */
void pluto_stripmine(Stmt *stmt, int dim, int factor, char *supernode,
                     PlutoProg *prog) {
  pluto_stmt_add_dim(stmt, 0, dim, supernode, H_TILE_SPACE_LOOP, PSA_H_ARRAY_PART_LOOP, prog);

  PlutoConstraints *domain = stmt->domain;

  pluto_constraints_add_inequality(domain);
  domain->val[domain->nrows - 1][0] = -factor;
  assert(stmt->trans->ncols == domain->ncols);
  int i;
  for (i = 1; i < stmt->trans->ncols - 1; i++) {
    domain->val[domain->nrows - 1][i] = stmt->trans->val[dim + 1][i];
  }

  pluto_constraints_add_inequality(domain);
  domain->val[domain->nrows - 1][0] = factor;
  assert(stmt->trans->ncols == domain->ncols);
  for (i = 1; i < stmt->trans->ncols - 1; i++) {
    domain->val[domain->nrows - 1][i] = -stmt->trans->val[dim + 1][i];
  }
  domain->val[domain->nrows - 1][stmt->trans->ncols] += factor;
}

/* Interchange loops for a stmt */
void pluto_stmt_loop_interchange(Stmt *stmt, int level1, int level2,
                                 PlutoProg *prog) {
  int j, tmp;

  for (j = 0; j < stmt->trans->ncols; j++) {
    tmp = stmt->trans->val[level1][j];
    stmt->trans->val[level1][j] = stmt->trans->val[level2][j];
    stmt->trans->val[level2][j] = tmp;    
  }

  /* Jie Added - Start */
  PlutoHypType tmp_htype;
  PSAHypType tmp_psahtype;
  /* Permute the hyperplane types */
  tmp_htype = stmt->hyp_types[level1];
  stmt->hyp_types[level1] = stmt->hyp_types[level2];
  stmt->hyp_types[level2] = tmp_htype;

  /* Permute the PSA hyperplane types */
  tmp_psahtype = stmt->psa_hyp_types[level1];
  stmt->psa_hyp_types[level1] = stmt->psa_hyp_types[level2];
  stmt->psa_hyp_types[level2] = tmp_psahtype;
  /* Jie Added - End */
}

void pluto_interchange(PlutoProg *prog, int level1, int level2) {
  int k;
  HyperplaneProperties hTmp;

  Stmt **stmts = prog->stmts;
  int nstmts = prog->nstmts;

  for (k = 0; k < nstmts; k++) {
    pluto_stmt_loop_interchange(stmts[k], level1, level2, prog);
  }

  hTmp = prog->hProps[level1];
  prog->hProps[level1] = prog->hProps[level2];
  prog->hProps[level2] = hTmp;

}

void pluto_sink_transformation(Stmt *stmt, int pos, PlutoProg *prog) {
  int i, npar;

  npar = stmt->domain->ncols - stmt->dim - 1;

  assert(pos <= stmt->trans->nrows);
  assert(stmt->dim + npar + 1 == stmt->domain->ncols);

  /* Stmt should always have a transformation */
  assert(stmt->trans != NULL);

  pluto_matrix_add_row(stmt->trans, pos);

  stmt->hyp_types = realloc(stmt->hyp_types, sizeof(int) * stmt->trans->nrows);
  for (i = stmt->trans->nrows - 2; i >= pos; i--) {
    stmt->hyp_types[i + 1] = stmt->hyp_types[i];
  }
  stmt->hyp_types[pos] = H_SCALAR;

  /* Jie Added - Start */
  stmt->psa_hyp_types = realloc(stmt->psa_hyp_types, sizeof(int) * stmt->trans->nrows);
  for (i = stmt->trans->nrows - 2; i >= pos; i--) {
    stmt->psa_hyp_types[i + 1] = stmt->psa_hyp_types[i];
  }
  stmt->psa_hyp_types[pos] = PSA_H_SCALAR;
  /* Jie Added - End */
}

/* Make loop the innermost loop; all loops below move up by one */
void pluto_make_innermost_loop(Ploop *loop, PlutoProg *prog) {
  int i, d, last_depth;

  last_depth = prog->num_hyperplanes - 1;
  for (i = 0; i < loop->nstmts; i++) {
    Stmt *stmt = loop->stmts[i];
    for (d = loop->depth; d < stmt->trans->nrows; d++) {
      if (pluto_is_hyperplane_scalar(stmt, d)) {
        break;
      }
    }
    last_depth = PLMIN(last_depth, d - 1);
  }

  for (i = 0; i < loop->nstmts; i++) {
    Stmt *stmt = loop->stmts[i];
    for (d = loop->depth; d < last_depth; d++) {
      pluto_stmt_loop_interchange(stmt, d, d + 1, prog);
    }
  }
}

/* Make loop the innermost loop in the permutable loop band */
void psa_make_innermost_loop_band(Ploop *loop, Band *band, PlutoProg *prog) {
  int i, d;
  for (i = 0; i < loop->nstmts; i++) {
    Stmt *stmt = loop->stmts[i];
    int d;
    for (d = loop->depth; d < band->loop->depth + band->width - 1; d++) {
      pluto_stmt_loop_interchange(stmt, d, d + 1, prog);
    }
  }
}

/* Tile the single loop */
void psa_tile_loop(PlutoProg *prog, Ploop *loop, int tile_factor, 
                   PlutoHypType htype, PSAHypType psa_htype) {
  int i, j, s;
  int npar;

  npar = prog->npar;

  int firstD = loop->depth;
  int depth = loop->depth;  
  int num_domain_supernodes[loop->nstmts];

  for (s = 0; s < loop->nstmts; s++) {
    num_domain_supernodes[s] = 0;
  }

  for (s = 0; s < loop->nstmts; s++) {
    Stmt *stmt = loop->stmts[s];
    /* 1. Specify tiles in the original domain */
    /* 1.1 Add additional dimensions */
    char iter[6];
    sprintf(iter, "zT%d", stmt->dim);

    int hyp_type = (stmt->hyp_types[depth] == H_SCALAR)
                      ? H_SCALAR    
                      : htype;

    int psa_hyp_type = (stmt->psa_hyp_types[depth] == PSA_H_SCALAR)
                      ? PSA_H_SCALAR
                      : psa_htype;

    /* 1.2 Specify tile shapes in the original domain */
    /* Domain supernodes aren't added for scalar dimensions */
    if (hyp_type != H_SCALAR) {
      assert(tile_factor >= 1);
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
        -tile_factor;
      
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
        tile_factor;

      stmt->domain->val[stmt->domain->nrows - 1][stmt->domain->ncols - 1] = 
        -stmt->trans->val[1 + (depth - firstD) + depth][stmt->dim + prog->npar] + 
        tile_factor - 1;

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
    // stmt->first_tile_dim = firstD;
    // stmt->last_tile_dim = lastD;    
  } /* all statements */

  /* Sink everything to the same depth */
  int max = 0, curr;
  for (i = 0; i < loop->nstmts; i++) {
    max = PLMAX(loop->stmts[i]->trans->nrows, max);
  }
  for (i = 0; i < loop->nstmts; i++) {
    curr = loop->stmts[i]->trans->nrows; 
    for (j = curr; j < max; j++) {
      pluto_sink_transformation(loop->stmts[i], loop->stmts[i]->trans->nrows, prog);
    }
  }

  // curr = prog->num_hyperplanes;  
  curr = firstD + 1;
  pluto_prog_add_hyperplane(prog, depth, H_UNKNOWN, PSA_H_UNKNOWN);  

  /* Re-detect hyperplane types (H_SCALAR, H_LOOP) */
  pluto_detect_hyperplane_types(prog);
  pluto_detect_hyperplane_types_stmtwise(prog);
  psa_detect_hyperplane_types(prog, prog->array_dim, prog->array_part_dim);
  psa_detect_hyperplane_types_stmtwise(prog, prog->array_dim, prog->array_part_dim);  

  /* Detect properties after permuation */
  pluto_compute_dep_directions(prog);
  pluto_compute_dep_satisfaction(prog);
  psa_compute_dep_distances(prog);
}

/* Tile the single loop */
/* 2i+j = 32oT + iT, 0 <= iT <= 31 */
void psa_tile_loop_constant(PlutoProg *prog, Ploop *loop, int tile_factor, 
                   PlutoHypType htype, PSAHypType psa_htype) {
  int i, j, s;
  int npar;

  npar = prog->npar;

  int firstD = loop->depth;
  int depth = loop->depth;  
  int num_domain_internodes[loop->nstmts];
//  int num_domain_intranodes[loop->nstmts];

  for (s = 0; s < loop->nstmts; s++) {
    num_domain_internodes[s] = 0;
//    num_domain_intranodes[s] = 0;
  }

  for (s = 0; s < loop->nstmts; s++) {
    Stmt *stmt = loop->stmts[s];
    /* 1. Specify tiles in the original domain */
    /* 1.1 Add additional dimensions */
    char inter_iter[6];
//    char intra_iter[6];
    sprintf(inter_iter, "oT%d", stmt->dim);
//    sprintf(intra_iter, "iT%d", stmt->dim);

    int hyp_type = (stmt->hyp_types[depth] == H_SCALAR)
                      ? H_SCALAR    
                      : htype;

    int psa_hyp_type = (stmt->psa_hyp_types[depth] == PSA_H_SCALAR)
                      ? PSA_H_SCALAR
                      : psa_htype;

    /* 1.2 Specify tile shapes in the original domain */
    /* Domain supernodes aren't added for scalar dimensions */
    if (hyp_type != H_SCALAR) {
      assert(tile_factor >= 1);
      pluto_stmt_add_dim(stmt, num_domain_internodes[s], depth, inter_iter,
                          hyp_type, psa_hyp_type, prog);
      num_domain_internodes[s]++;
//      pluto_stmt_add_dim(stmt, num_domain_internodes[s] + num_domain_intranodes[s], depth + 1, intra_iter,
//                          hyp_type, psa_hyp_type, prog);
//      num_domain_intranodes[s]++;

      /* Add relation b/w tile space variable and intra-tile variables like
       * 32*oT <= 2t+i <= 32*oT + 31 */
      pluto_constraints_add_inequality(stmt->domain);
      for (j = num_domain_internodes[s]; j < stmt->dim + npar + 1; j++) {
        stmt->domain->val[stmt->domain->nrows - 1][j] = 
          stmt->trans->val[1 + (depth - firstD) + depth][j];        
      }

      stmt->domain->val[stmt->domain->nrows - 1][num_domain_internodes[s] - 1] = -tile_factor;

      PlutoConstraints *lb =        
        pluto_constraints_select_row(stmt->domain, stmt->domain->nrows - 1);
      /* Add the new constraints into dependence polyhedron */
      pluto_update_deps(stmt, lb, prog); 
      pluto_constraints_free(lb);

      pluto_constraints_add_inequality(stmt->domain);
      for (j = num_domain_internodes[s]; j < stmt->dim + npar + 1; j++) {
        stmt->domain->val[stmt->domain->nrows - 1][j] = 
          -stmt->trans->val[1 + (depth - firstD) + depth][j];
      }

      stmt->domain->val[stmt->domain->nrows - 1][num_domain_internodes[s] - 1] = tile_factor;
      stmt->domain->val[stmt->domain->nrows - 1][stmt->dim + npar] += tile_factor - 1;

      lb = pluto_constraints_select_row(stmt->domain, stmt->domain->nrows - 1);
      pluto_update_deps(stmt, lb, prog);
      pluto_constraints_free(lb);

      // resetting the scheduling row for 2t+i
      // phi = 2t+i - 32*oT
      stmt->trans->val[1 + (depth - firstD) + depth][num_domain_internodes[s] - 1] = -tile_factor;
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
    // stmt->first_tile_dim = firstD;
    // stmt->last_tile_dim = lastD;    
  } /* all statements */

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

  // curr = prog->num_hyperplanes;  
  curr = firstD + 1;
  pluto_prog_add_hyperplane(prog, depth, H_UNKNOWN, PSA_H_UNKNOWN);  

//  /* Re-detect hyperplane types (H_SCALAR, H_LOOP) */
//  pluto_detect_hyperplane_types(prog);
//  pluto_detect_hyperplane_types_stmtwise(prog);
//  psa_detect_hyperplane_types(prog, prog->array_dim, prog->array_part_dim);
//  psa_detect_hyperplane_types_stmtwise(prog, prog->array_dim, prog->array_part_dim);  
//
//  /* Detect properties after permuation */
//  pluto_compute_dep_directions(prog);
//  pluto_compute_dep_satisfaction(prog);
//  psa_compute_dep_distances(prog);
}
