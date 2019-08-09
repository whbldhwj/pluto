/*
 * PLUTO: An automatic parallelizer and locality optimizer
 *
 * Copyright (C) 2007-2012 Uday Bondhugula
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pluto.h"
#include "program.h"
#include "transforms.h"
#include "distmem.h"

/* Get list of <Stmt, acc> lists - each list corresponds to
 * statements with accesses to the same data; *num_data gives the number of
 * data elements; nstmts_per_acc[i] is the number of stmts in list[i]
 * Consider statements passed via 'stmts'
 * Return: num_data - number of variables, i.e., number of lists
 * nstmts_per_acc[]: number of stmt, acc pairs in each list
 * */
struct stmt_access_pair ***
get_read_write_access_with_stmts(Stmt **stmts, int nstmts, int *num_data,
                                 int **nstmts_per_acc) {
  int i, j, k, curr_num;
  int *num_stmts_per_acc;
  struct stmt_access_pair ***acc_stmts;

  curr_num = 0;
  num_stmts_per_acc = NULL;
  acc_stmts = NULL;

  for (i = 0; i < nstmts; i++) {
    Stmt *stmt = stmts[i];
    struct stmt_access_pair *new;
    for (k = 0; k < stmt->nreads; k++) {

      if (is_access_scalar(stmt->reads[k]))
        continue;

      new = malloc(sizeof(struct stmt_access_pair));
      new->stmt = stmt;
      new->acc = stmt->reads[k];
      new->acc_rw = 0;

      for (j = 0; j < curr_num; j++) {
        if (!strcmp(stmt->reads[k]->name, acc_stmts[j][0]->acc->name)) {
          /* Add to end of array */
          acc_stmts[j] = (struct stmt_access_pair **)realloc(
              acc_stmts[j],
              (num_stmts_per_acc[j] + 1) * sizeof(struct stmt_access_pair *));
          acc_stmts[j][num_stmts_per_acc[j]] = new;

          num_stmts_per_acc[j]++;
          break;
        }
      }
      if (j == curr_num) {
        /* New data variable */
        acc_stmts = (struct stmt_access_pair ***)realloc(
            acc_stmts, (curr_num + 1) * sizeof(struct stmt_access_pair **));
        acc_stmts[curr_num] = (struct stmt_access_pair **)malloc(
            sizeof(struct stmt_access_pair *));
        acc_stmts[curr_num][0] = new;

        num_stmts_per_acc =
            (int *)realloc(num_stmts_per_acc, (curr_num + 1) * sizeof(int));
        num_stmts_per_acc[curr_num] = 1;
        curr_num++;
      }
    }

    stmt = stmts[i];
    new = malloc(sizeof(struct stmt_access_pair));
    new->stmt = stmt;
    new->acc = stmt->writes[0];
    new->acc_rw = 1;

    for (j = 0; j < curr_num; j++) {
      if (!strcmp(stmt->writes[0]->name, acc_stmts[j][0]->acc->name)) {
        /* Add to end of array */
        acc_stmts[j] = (struct stmt_access_pair **)realloc(
            acc_stmts[j],
            (num_stmts_per_acc[j] + 1) * sizeof(struct stmt_access_pair *));
        acc_stmts[j][num_stmts_per_acc[j]] = new;

        num_stmts_per_acc[j]++;
        break;
      }
    }
    if (j == curr_num) {
      /* New data variable */
      acc_stmts = (struct stmt_access_pair ***)realloc(
          acc_stmts, (curr_num + 1) * sizeof(struct stmt_access_pair **));
      acc_stmts[curr_num] =
          (struct stmt_access_pair **)malloc(sizeof(struct stmt_access_pair *));
      acc_stmts[curr_num][0] = new;

      num_stmts_per_acc =
          (int *)realloc(num_stmts_per_acc, (curr_num + 1) * sizeof(int));
      num_stmts_per_acc[curr_num] = 1;
      curr_num++;
    }
  }

  *num_data = curr_num;
  *nstmts_per_acc = num_stmts_per_acc;

  return acc_stmts;
}

/* Get list of <Stmt, read acc> lists - each list corresponds to
 * statements with read accesses to the same data; *num_data gives the number of
 * data elements; nstmts_per_acc[i] is the number of stmts in list[i]
 * Consider statements passed via 'stmts'
 * Return: num_data - number of variables, i.e., number of lists
 * nstmts_per_acc[]: number of stmt, acc pairs in each list
 * */
// not being used currently
struct stmt_access_pair ***get_read_access_with_stmts(Stmt **stmts, int nstmts,
                                                      int *num_data,
                                                      int **nstmts_per_acc) {
  int i, j, k, curr_num;
  int *num_stmts_per_acc;
  struct stmt_access_pair ***racc_stmts;

  curr_num = 0;
  num_stmts_per_acc = NULL;
  racc_stmts = NULL;

  for (i = 0; i < nstmts; i++) {
    Stmt *stmt = stmts[i];
    for (k = 0; k < stmt->nreads; k++) {

      if (is_access_scalar(stmt->reads[k]))
        continue;

      struct stmt_access_pair *new = malloc(sizeof(struct stmt_access_pair));
      new->stmt = stmt;
      new->acc = stmt->reads[k];

      for (j = 0; j < curr_num; j++) {
        if (!strcmp(stmt->reads[k]->name, racc_stmts[j][0]->acc->name)) {
          /* Add to end of array */
          racc_stmts[j] = (struct stmt_access_pair **)realloc(
              racc_stmts[j],
              (num_stmts_per_acc[j] + 1) * sizeof(struct stmt_access_pair *));
          racc_stmts[j][num_stmts_per_acc[j]] = new;

          num_stmts_per_acc[j]++;
          break;
        }
      }
      if (j == curr_num) {
        /* New data variable */
        racc_stmts = (struct stmt_access_pair ***)realloc(
            racc_stmts, (curr_num + 1) * sizeof(struct stmt_access_pair **));
        racc_stmts[curr_num] = (struct stmt_access_pair **)malloc(
            sizeof(struct stmt_access_pair *));
        racc_stmts[curr_num][0] = new;

        num_stmts_per_acc =
            (int *)realloc(num_stmts_per_acc, (curr_num + 1) * sizeof(int));
        num_stmts_per_acc[curr_num] = 1;
        curr_num++;
      }
    }
  }

  *num_data = curr_num;
  *nstmts_per_acc = num_stmts_per_acc;

  return racc_stmts;
}

/* Get list of <Stmt, write acc> lists - each list corresponds to
 * statements with write accesses to the same data; *num_data gives the number
 * of
 * data elements; nstmts_per_acc[i] is the number of stmts in list[i]
 * Consider statements passed via 'stmts'
 * Return: num_data - number of variables, i.e., number of lists
 * nstmts_per_acc[]: number of stmt, acc pairs in each list
 * */
struct stmt_access_pair ***get_write_access_with_stmts(Stmt **stmts, int nstmts,
                                                       int *num_data,
                                                       int **nstmts_per_acc) {
  int i, j, k, curr_num;
  int *num_stmts_per_acc;
  struct stmt_access_pair ***wacc_stmts;

  curr_num = 0;
  num_stmts_per_acc = NULL;
  wacc_stmts = NULL;

  for (i = 0; i < nstmts; i++) {
    Stmt *stmt = stmts[i];
    for (k = 0; k < stmt->nwrites; k++) {
      struct stmt_access_pair *new = malloc(sizeof(struct stmt_access_pair));
      new->stmt = stmt;
      new->acc = stmt->writes[k];

      for (j = 0; j < curr_num; j++) {
        if (!strcmp(stmt->writes[k]->name, wacc_stmts[j][0]->acc->name)) {
          /* Add to end of array */
          wacc_stmts[j] = (struct stmt_access_pair **)realloc(
              wacc_stmts[j],
              (num_stmts_per_acc[j] + 1) * sizeof(struct stmt_access_pair *));
          wacc_stmts[j][num_stmts_per_acc[j]] = new;

          num_stmts_per_acc[j]++;
          break;
        }
      }
      if (j == curr_num) {
        /* New data variable */
        wacc_stmts = (struct stmt_access_pair ***)realloc(
            wacc_stmts, (curr_num + 1) * sizeof(struct stmt_access_pair **));
        wacc_stmts[curr_num] = (struct stmt_access_pair **)malloc(
            sizeof(struct stmt_access_pair *));
        wacc_stmts[curr_num][0] = new;

        num_stmts_per_acc =
            (int *)realloc(num_stmts_per_acc, (curr_num + 1) * sizeof(int));
        num_stmts_per_acc[curr_num] = 1;
        curr_num++;
      }
    }
  }

  *num_data = curr_num;
  *nstmts_per_acc = num_stmts_per_acc;

  return wacc_stmts;
}

Stmt *get_new_anchor_stmt(Stmt **stmts, int nstmts) {
  int i, j;

  int max_dim_index, max_dim;
  max_dim = stmts[0]->dim;
  max_dim_index = 0;
  for (i = 1; i < nstmts; i++) {
    if (stmts[i]->dim > max_dim) {
      max_dim = stmts[i]->dim;
      max_dim_index = i;
    }
  }

  Stmt *anchor_stmt = stmts[max_dim_index];
  PlutoConstraints *domain = pluto_constraints_dup(anchor_stmt->domain);
  for (i = 0; i < nstmts; i++) {
    PlutoConstraints *new_domain = pluto_constraints_dup(stmts[i]->domain);
    int ncols = new_domain->ncols;
    for (j = 0; j < domain->ncols - ncols; j++) {
      pluto_constraints_add_dim(new_domain, stmts[i]->dim, NULL);
    }
    pluto_constraints_unionize(domain, new_domain);
    pluto_constraints_free(new_domain);
  }

  Stmt *new_anchor_stmt = pluto_create_stmt(
      anchor_stmt->dim, domain, anchor_stmt->trans, anchor_stmt->iterators,
      anchor_stmt->text, anchor_stmt->type);
  new_anchor_stmt->id = anchor_stmt->id;
  pluto_constraints_free(domain);
  return new_anchor_stmt;
}

// returns for each domain parallel loop or band of loops:
// outer_dist_loop_level : the level/dimension of the outermost
// distributed/parallel loop
// copy_level : the level/dimension of a statement within the innermost
// distributed/parallel loop
// void init_copy_level(PlutoProg *prog, Ploop **loops, int nloops,
//                      int *copy_level, int *outer_dist_loop_level) {
//   int l, inner_dist_loop_level;
//   for (l = 0; l < nloops; l++) {
//     if (options->multi_level_distribution) {
//       Stmt *stmt = loops[l]->stmts[0];
//       assert(stmt->last_tile_dim >= loops[l]->depth);
// #ifdef distributing_all_inter_tile_loops
//       for (inner_dist_loop_level = stmt->last_tile_dim;
//            inner_dist_loop_level >= loops[l]->depth; inner_dist_loop_level--) {
//         if (pluto_is_hyperplane_loop(stmt, inner_dist_loop_level))
//           break;
//       }
// #else
//       for (inner_dist_loop_level = loops[l]->depth;
//            inner_dist_loop_level < stmt->last_tile_dim;
//            inner_dist_loop_level++) {
//         if (pluto_is_hyperplane_loop(stmt, inner_dist_loop_level + 1)) {
//           inner_dist_loop_level++;
//           break; // for now, distributing only 2 loops at the most
//         }
//       }
// #endif
//       if (options->distmem) {
//         unsigned num_inner_loops = 0;
//         Ploop **inner_loops =
//             pluto_get_loops_under(loops[l]->stmts, loops[l]->nstmts,
//                                   loops[l]->depth, prog, &num_inner_loops);
//         int is_distribution_set = 0;
//         for (unsigned i = 0; i < num_inner_loops; i++) {
//           if (inner_loops[i]->depth <= inner_dist_loop_level) {
//             if (!pluto_loop_is_parallel(prog, inner_loops[i])) {
//               if (options->dynschedule) {
//                 sprintf(prog->decls + strlen(prog->decls),
//                         "__is_multi_partitioned[%d] = 1;\n", l);
//                 is_distribution_set = 1;
//               } else {
//                 inner_dist_loop_level = loops[l]->depth;
//               }
//               break;
//             }
//           }
//         }
//         if (options->dynschedule && !is_distribution_set) {
//           // !!!roshan can use some loop analysis to see if block cyclic is
//           // beneficial
//           sprintf(prog->decls + strlen(prog->decls),
//                   "##ifdef __USE_BLOCK_CYCLIC\n");
//           sprintf(prog->decls + strlen(prog->decls),
//                   "__is_block_cyclic[%d] = 1;\n", l);
//           sprintf(prog->decls + strlen(prog->decls), "##endif\n");
//         }
//         pluto_loops_free(inner_loops, num_inner_loops);
//       }
//     } else {
//       inner_dist_loop_level = loops[l]->depth;
//     }
//     copy_level[l] = inner_dist_loop_level + 1;
//     if (outer_dist_loop_level != NULL)
//       outer_dist_loop_level[l] = loops[l]->depth;
//     if (options->blockcyclic) {
//       /* FIXME: copy_level[l] in the presence of blockcyclic
//        * will depend on the order in which these parallel loops were tiled
//        * for block cylic partitioning (will work if they were in the
//        * increasing order of depths: i.e., loops in ploops array were in
//        * increasing order of their depths
//        */
//       copy_level[l] += l;
//       if (outer_dist_loop_level != NULL)
//         outer_dist_loop_level[l] += l;
//     }
//   }
// }

/* Get access string */
char *reconstruct_access(PlutoAccess *acc) {
  int ndims, i;
  ndims = acc->mat->nrows;

  char *access;

  access = (char *)malloc(strlen(acc->name) + ndims * (2 + 4) + 1);

  strcpy(access, acc->name);

  for (i = 0; i < ndims; i++) {
    strcat(access, "[");
    char tmp[13];
    sprintf(tmp, "d%d", i + 1);
    strcat(access, tmp);
    strcat(access, "]");
  }

  return access;
}

PlutoConstraints *compute_write_out_iter(
  struct stmt_access_pair *wacc_stmt, int copy_level, PlutoProg *prog
) {
  int i;

  Stmt *wstmt = wacc_stmt->stmt;
  PlutoAccess *wacc = wacc_stmt->acc;
  Dep **deps;
  int ndeps;
  if (options->lastwriter) {
    deps = prog->transdeps;
    ndeps = prog->ntransdeps;
  } else {
    deps = prog->deps;
    ndeps = prog->ndeps;
  }

  PlutoConstraints *srcdomain = pluto_get_new_domain(wstmt);

  /* Locations written to inside this tile */
  // PlutoConstraints *uwcst =
  //     pluto_compute_region_data(wstmt, srcdomain, wacc, copy_level, prog);  

  // requires transitive WAR dependences if they exist
  for (i = 0; i < ndeps; i++) {
    Dep *dep = deps[i];
    // if (dep->type != OSL_DEPENDENCE_WAW && dep->type != OSL_DEPENDENCE_WAR)
    // continue;
    if (dep->type != OSL_DEPENDENCE_WAW)
      continue;

    Stmt *src = prog->stmts[dep->src];
    Stmt *dest = prog->stmts[dep->dest];

    /* Only dependences with this one as source access */
    if (wacc != src->writes[0])
      continue;

    IF_DEBUG(printf("For dep %d\n", dep->id + 1));

    const PlutoAccess *wacc_src = src->writes[0];

// #ifdef JIE_DEBUG
//     pluto_constraints_pretty_print(stdout, dep->dpolytope);
// #endif
    PlutoConstraints *tdpoly = pluto_get_transformed_dpoly(dep, src, dest);
// #ifdef JIE_DEBUG
//     pluto_constraints_pretty_print(stdout, tdpoly);
// #endif

    assert(tdpoly->ncols ==
           src->trans->nrows + dest->trans->nrows + prog->npar + 1);

    // printf("tdpoly\n");
    // pluto_constraints_print(stdout, tdpoly);

    PlutoConstraints *param_eq;

    /* No need to check whether source and target share all of the
     * copy_level loops; handled naturally since those scalar dimensions
     * would be part of the 'copy_level' dimensions. If the statements are
     * separated somewhere, subtracting param_eq will not changed tdpoly
     */
    param_eq =
        get_context_equality(copy_level, src->trans->nrows, tdpoly->ncols);

// #ifdef JIE_DEBUG
//     fprintf(stdout, "[Debug] PARAM_EQ:\n");
//     pluto_constraints_pretty_print(stdout, param_eq);
// #endif

    /* Source/target iteration pairs with target lying outside the tile */
    //PlutoConstraints *tile_dest_outside =
    //    pluto_constraints_difference(tdpoly, param_eq);
    PlutoConstraints *tile_dest_outside = 
          pluto_constraints_difference_isl(tdpoly, param_eq);    

    /* Parameteric in the outer source 'copy_level' dimensions (since
     * uwcst is constructed that way) */
    /* Yields iterations inside the tile whose write locations are again
     * written to outside the tile */
    pluto_constraints_project_out_isl_single(&tile_dest_outside, src->trans->nrows,
                                  dest->trans->nrows);
    
    // /* Locations that will again be written to outside */
    // PlutoConstraints *wcst = pluto_compute_region_data(
    //     src, tile_dest_outside, wacc_src, copy_level, prog);    

    /* Subtracting out locations that will be written to outside */
    //uwcst = pluto_constraints_subtract(uwcst, wcst);
    srcdomain = pluto_constraints_subtract(srcdomain, tile_dest_outside);

    pluto_constraints_free(tdpoly);
    //pluto_constraints_free(wcst);
    pluto_constraints_free(tile_dest_outside);
    pluto_constraints_free(param_eq);
  }
  //pluto_constraints_free(srcdomain);
  // IF_DEBUG(printf("Last write out\n"););
  // IF_DEBUG(pluto_constraints_print(stdout, uwcst););
  //return uwcst;    
  return srcdomain;
}

/*
 * Computes write-out set
 * wacc_stmt: access pair for which last writes need to be computed
 * copy_level: number of (outer) schedule rows to be treated as
 * parameters along with global parameters
 *
 * NOTE: requires transitive WAW deps
 *
 * Output format:  [copy_level, acc->nrows, prog->npar + 1]
 */
PlutoConstraints *compute_write_out(struct stmt_access_pair *wacc_stmt,
                                    int copy_level, PlutoProg *prog) {
  int i;

  Stmt *wstmt = wacc_stmt->stmt;
  PlutoAccess *wacc = wacc_stmt->acc;
  Dep **deps;
  int ndeps;
  if (options->lastwriter) {
    // contains transitive dependences only due to WAR dependences
    deps = prog->transdeps;
    ndeps = prog->ntransdeps;
  } else {
    // contains transitive dependences
    deps = prog->deps;
    ndeps = prog->ndeps;
  }

  PlutoConstraints *srcdomain = pluto_get_new_domain(wstmt);

  /* Locations written to inside this tile */
  PlutoConstraints *uwcst =
      pluto_compute_region_data(wstmt, srcdomain, wacc, copy_level, prog);

  IF_DEBUG(printf("Computing write out set for %s\n", wacc->name););

  IF_DEBUG(printf("Data written to in tile\n"););
  IF_DEBUG(pluto_constraints_print(stdout, uwcst););

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] write_out_function\n");
//   pluto_constraints_print(stdout, uwcst);
// #endif

  // requires transitive WAR dependences if they exist
  for (i = 0; i < ndeps; i++) {
    Dep *dep = deps[i];
    // if (dep->type != OSL_DEPENDENCE_WAW && dep->type != OSL_DEPENDENCE_WAR)
    // continue;
    if (dep->type != OSL_DEPENDENCE_WAW)
      continue;

    Stmt *src = prog->stmts[dep->src];
    Stmt *dest = prog->stmts[dep->dest];

    /* Only dependences with this one as source access */
    if (wacc != src->writes[0])
      continue;

    IF_DEBUG(printf("For dep %d\n", dep->id + 1));

// #ifdef JIE_DEBUG
//     fprintf(stdout, "[Debug] For dep %d\n", dep->id + 1);
// #endif

    const PlutoAccess *wacc_src = src->writes[0];

#ifdef JIE_DEBUG
    fprintf(stdout, "[Debug] Print the dep in write-out\n");
    pluto_constraints_pretty_print(stdout, dep->dpolytope);
#endif

    PlutoConstraints *tdpoly = pluto_get_transformed_dpoly(dep, src, dest);
// #ifdef JIE_DEBUG
//     pluto_constraints_pretty_print(stdout, tdpoly);
// #endif

    assert(tdpoly->ncols ==
           src->trans->nrows + dest->trans->nrows + prog->npar + 1);

    // printf("tdpoly\n");
    // pluto_constraints_print(stdout, tdpoly);

    PlutoConstraints *param_eq;

    /* No need to check whether source and target share all of the
     * copy_level loops; handled naturally since those scalar dimensions
     * would be part of the 'copy_level' dimensions. If the statements are
     * separated somewhere, subtracting param_eq will not changed tdpoly
     */
    param_eq =
        get_context_equality(copy_level, src->trans->nrows, tdpoly->ncols);

// #ifdef JIE_DEBUG
//     fprintf(stdout, "[Debug] PARAM_EQ:\n");
//     pluto_constraints_pretty_print(stdout, param_eq);
// #endif

    /* Source/target iteration pairs with target lying outside the tile */
    //PlutoConstraints *tile_dest_outside =
    //    pluto_constraints_difference(tdpoly, param_eq);
    PlutoConstraints *tile_dest_outside = 
          pluto_constraints_difference_isl(tdpoly, param_eq);    

    /* Parameteric in the outer source 'copy_level' dimensions (since
     * uwcst is constructed that way) */
    /* Yields iterations inside the tile whose write locations are again
     * written to outside the tile */
    pluto_constraints_project_out_isl_single(&tile_dest_outside, src->trans->nrows,
                                  dest->trans->nrows);

    IF_DEBUG(printf("Dep target iters outside of tile that write to same "
                    "variable subseq to tile exec\n"););
    IF_DEBUG(pluto_constraints_print(stdout, tile_dest_outside););

    /* Values that'll be written outside the tile with source inside;
     * tile_dest_outside format [ src copy_level params | dest | par | 1]
     */
    /* Locations that will again be written to outside */
    PlutoConstraints *wcst = pluto_compute_region_data(
        src, tile_dest_outside, wacc_src, copy_level, prog);

    IF_DEBUG(printf(
        "Values written outside for Dep %d with source inside tile\n", i + 1););
    IF_DEBUG(pluto_constraints_print(stdout, wcst););

    /* Subtracting out locations that will be written to outside */
    uwcst = pluto_constraints_subtract(uwcst, wcst);

    pluto_constraints_free(tdpoly);
    pluto_constraints_free(wcst);
    pluto_constraints_free(tile_dest_outside);
    pluto_constraints_free(param_eq);
  }
  pluto_constraints_free(srcdomain);
  // IF_DEBUG(printf("Last write out\n"););
  // IF_DEBUG(pluto_constraints_print(stdout, uwcst););
  return uwcst;  
}

PlutoConstraints *compute_read_in(
  struct stmt_access_pair *racc_stmt,
  int copy_level, PlutoProg *prog
) {
  Stmt *stmt = racc_stmt->stmt;
  PlutoAccess *acc = racc_stmt->acc;

  PlutoConstraints *new_domain = pluto_get_new_domain(stmt);

  PlutoConstraints *access_region = pluto_compute_region_data(
    stmt, new_domain, acc, copy_level, prog
  );

  pluto_constraints_free(new_domain);
  return access_region;
}

PlutoConstraints *compute_read_in_engine_reuse(
  struct stmt_access_pair *racc_stmt,
  int copy_level, 
  int reuse_level,
  PlutoProg *prog
) {
  Stmt *stmt = racc_stmt->stmt;
  PlutoAccess *acc = racc_stmt->acc;

  PlutoConstraints *new_domain = pluto_get_new_domain(stmt);

  PlutoConstraints *access_region = pluto_compute_region_data(
    stmt, new_domain, acc, copy_level, prog
  );
  
  pluto_constraints_project_out_isl_single(
     &access_region, reuse_level, 1
  );

  pluto_constraints_add_dim(access_region, reuse_level, NULL);

  pluto_constraints_free(new_domain);
  return access_region;
}

/* Compute region(s) of data accessed by 'acc' with 'copy_level' number of
 * outer loops as parameters
 * 'copy_level' outer dimensions will be treated as parameters in addition
 * to global ones
 * domain: set (iterations of stmt) accessing data - in transformed space
 * acc: original access function
 * Input format: [copy_level, stmt->trans->nrows, prog->npar, 1]
 *                or [copy_level, stmt->trans->nrows-copy_level, prog->npar,
 *1]
 *
 * Output format:  [copy_level, acc->nrows, prog->npar + 1]
 * */
PlutoConstraints *pluto_compute_region_data(const Stmt *stmt,
                                            const PlutoConstraints *domain,
                                            const PlutoAccess *acc,
                                            int copy_level,
                                            const PlutoProg *prog) {
  int i, k, npar, *divs;

  assert(acc->mat != NULL);
  assert(copy_level <= stmt->trans->nrows);

  npar = prog->npar;

  assert((stmt->trans->nrows + npar + 1 == domain->ncols) ||
         (copy_level + stmt->trans->nrows + npar + 1 == domain->ncols));

  PlutoMatrix *newacc = pluto_get_new_access_func(stmt, acc->mat, &divs);

  PlutoConstraints *datadom = pluto_constraints_dup(domain);

  assert(newacc->ncols == stmt->trans->nrows + npar + 1);

  for (k = 0; k < newacc->nrows; k++) {
    pluto_matrix_negate_row(newacc, newacc->nrows - 1 - k);
    pluto_matrix_add_col(newacc, stmt->trans->nrows);
    newacc->val[newacc->nrows - 1 - k][stmt->trans->nrows] = divs[k];

    pluto_constraints_add_dim(datadom, domain->ncols - prog->npar - 1, NULL);
  }

  PlutoConstraints *acc_cst = pluto_constraints_from_equalities(newacc);

  for (i = 0; i < domain->ncols - stmt->trans->nrows - npar - 1; i++) {
    pluto_constraints_add_dim(acc_cst, 0, NULL);
  }

  pluto_constraints_add_to_each(datadom, acc_cst);

  pluto_constraints_project_out_isl_single(&datadom, copy_level, datadom->ncols -
                                                         copy_level - npar - 1 -
                                                         newacc->nrows);

  // IF_DEBUG(printf("compute_region_data: data set written to\n"););
  // IF_DEBUG(pluto_constraints_print(stdout, datadom););

  pluto_constraints_free(acc_cst);
  pluto_matrix_free(newacc);

  if (domain->next != NULL) {
    datadom->next =
        pluto_compute_region_data(stmt, domain->next, acc, copy_level, prog);
  }

  return datadom;
}

PlutoConstraints *pluto_get_transformed_dpoly(const Dep *dep, Stmt *src,
                                              Stmt *dest) {
  int i, npar;
  PlutoConstraints *src_sched, *dest_sched;

  npar = src->domain->ncols - src->dim - 1;

  assert(dep->dpolytope->ncols == src->dim + dest->dim + npar + 1);

  PlutoConstraints *dpoly = pluto_constraints_dup(dep->dpolytope);

  for (i = 0; i < src->trans->nrows; i++) {
    pluto_constraints_add_dim(dpoly, 0, NULL);
  }
  for (i = 0; i < dest->trans->nrows; i++) {
    pluto_constraints_add_dim(dpoly, src->trans->nrows + src->dim, NULL);
  }

  src_sched = pluto_stmt_get_schedule(src);
  dest_sched = pluto_stmt_get_schedule(dest);

  for (i = 0; i < dest->trans->nrows + dest->dim; i++) {
    pluto_constraints_add_dim(src_sched, src->trans->nrows + src->dim, NULL);
  }

  for (i = 0; i < src->trans->nrows + src->dim; i++) {
    pluto_constraints_add_dim(dest_sched, 0, NULL);
  }

  pluto_constraints_add(dpoly, src_sched);
  pluto_constraints_add(dpoly, dest_sched);

  // IF_DEBUG(printf("New pre-domain is \n"););
  // IF_DEBUG(pluto_constraints_print(stdout, newdom););

// #ifdef JIE_DEBUG  
//   fprintf(stdout, "SRC_SCHEDULE\n");
//   pluto_constraints_pretty_print(stdout, src_sched);
//   fprintf(stdout, "DEST SCHEDULE\n");
//   pluto_constraints_pretty_print(stdout, dest_sched);
//   fprintf(stdout, "DPOLY\n");
//   pluto_constraints_pretty_print(stdout, dpoly);
// #endif

  //pluto_constraints_project_out(dpoly, src->trans->nrows, src->dim);

  //pluto_constraints_project_out(dpoly, src->trans->nrows + dest->trans->nrows,
  //                              dest->dim);
  pluto_constraints_project_out_isl_single(&dpoly, src->trans->nrows, src->dim);

  pluto_constraints_project_out_isl_single(&dpoly, src->trans->nrows + dest->trans->nrows, dest->dim);

  // IF_DEBUG(printf("New domain is \n"););
  // IF_DEBUG(pluto_constraints_print(stdout, newdom););

  pluto_constraints_free(src_sched);
  pluto_constraints_free(dest_sched);

  return dpoly;
}

/* Create constraints equating first copy_level dimensions to copy_level
 * dimensions after the first src_nrows variables
 */
static PlutoConstraints *get_context_equality(int copy_level, int src_nrows,
                                              int ncols) {
  int i;
  PlutoConstraints *cst = pluto_constraints_alloc(copy_level, ncols);
  for (i = 0; i < copy_level; i++) {
    pluto_constraints_add_equality(cst);
    cst->val[cst->nrows - 1][i] = 1;
    cst->val[cst->nrows - 1][src_nrows + i] = -1;
  }
  return cst;
}
