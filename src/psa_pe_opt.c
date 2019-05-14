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

/* Read tiles sizes from tl_tile.sizes */
int psa_read_task_interleave_tile_sizes(
  int *tile_sizes,
  int num_tile_dims
) {
  int i;
  FILE *tsfile = fopen("ti_tile.sizes", "r");

  if (!tsfile) {
    fprintf(stdout, "[PSA] WARNING: ti_tile.sizes not found!\n");
    return 0;
  }

  for (i = 0; i < num_tile_dims && !feof(tsfile); i++) {
    fscanf(tsfile, "%d", &tile_sizes[i]);
  }

  if (i < num_tile_dims) {
    fprintf(stdout, "[PSA] WARNING: not enough task interleaving tile sizes provided!"
                    "The required number: %d\n", num_tile_dims);
    fclose(tsfile);
    return 0;
  }

  fclose(tsfile);
  return 1;
}

/*
 * For loops in each innermost loop bands (considering only time loops), 
 * move the sync-free (RAW, WAW, WAR) loops inside. If there is no sync-free
 * loop in the current loop bands, see if we could extend it to space loops
 * and tile it twice and move it inside.
 */
int psa_pe_task_interleave_optimize(Band *band, PlutoProg *prog) {  
  unsigned i, j;

  /* Band has to be the innermost band as well */
  /* TODO: This restriction should be removed in the future */
  if (!pluto_is_band_innermost(band, 0)) {
    return 0;
  }
  
  /* Mark the loop properties */
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

  /* Get the time loops */
//  Ploop **loops;
//  loops = pluto_get_loops_under(
//    band->loop->stmts, band->loop->nstmts,
//    first_time_hyp, prog, &nloops);    

  /* Update the hyp properties */
  int *dep_dis_hyp = (int *)malloc(prog->num_hyperplanes * sizeof(int));
  for (i = 0; i < prog->num_hyperplanes; i++) {
    dep_dis_hyp[i] = -2;
  }
  for (i = 0; i < prog->num_hyperplanes; i++) {
    for (j = 0; j < prog->ndeps; j++) {
      Dep *dep = prog->deps[j];
      assert(dep->disvec != NULL);
      int dep_dis;
      switch(dep->disvec[i]) {
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

  /* count if there is any sync-free loop among time loops */  
  unsigned num_parallel_time_loop = 0;
  int *parallel_time_hyp_id = (int *)malloc(prog->num_hyperplanes * sizeof(int));
  for (i = first_time_hyp; i < prog->num_hyperplanes; i++) {
    /* The current hyperplane should be a loop */
    int psa_h_type = prog->hProps[i].psa_type;
    if (IS_PSA_TIME_LOOP(psa_h_type)) {
      if (dep_dis_hyp[i] == 0) {
        parallel_time_hyp_id[num_parallel_time_loop] = i;
        num_parallel_time_loop++;      
      }
    }
  }

  /* count if there is any sync-free loop among space loops */  
  unsigned num_parallel_space_loop = 0;
  int *parallel_space_hyp_id = (int *)malloc(prog->num_hyperplanes * sizeof(int));
  for (i = first_space_hyp; i < first_time_hyp; i++) {
    /* The current hyperplane should be a loop */
    int psa_h_type = prog->hProps[i].psa_type;
    if (IS_PSA_SPACE_LOOP(psa_h_type)) {    
      if (dep_dis_hyp[i] == 0) {
        parallel_space_hyp_id[num_parallel_space_loop] = i;
        num_parallel_space_loop++;
      }
    }
  }  

  if (num_parallel_time_loop) {
    /* Permute sync-free loop innermost */
    for (i = 0; i < num_parallel_time_loop; i++) {
      unsigned cur_depth = parallel_time_hyp_id[i] - i;
      for (j = cur_depth; j < prog->num_hyperplanes - 1; j++) {
        pluto_interchange(prog, j, j + 1);
      }
    } 

    free(dep_dis_hyp);
    free(parallel_time_hyp_id);
    free(parallel_space_hyp_id);

    fprintf(stdout, "[PSA] Completed task interleaving optimization.\n");

    return 1;
  } else if (num_parallel_space_loop) {
    /* Tile the sync-free space loop and permute them innermost */
    int tile_sizes[num_parallel_space_loop];
    for (i = 0; i < num_parallel_space_loop; i++) {
      tile_sizes[i] = DEFAULT_TASK_INTERLEAVE_TILE_FACTOR;
    }

    psa_read_task_interleave_tile_sizes(tile_sizes, num_parallel_space_loop);

    for (i = 0; i < num_parallel_space_loop; i++) {
      unsigned cur_depth = parallel_space_hyp_id[i];
      unsigned nloops;
      Ploop **loops = psa_get_outermost_loops(
        prog, band->loop->stmts, band->loop->nstmts,
        cur_depth, &nloops
      );      
      assert(nloops == 1);
      psa_tile_loop(prog, loops[0], tile_sizes[i], H_TILE_SPACE_LOOP, PSA_H_SPACE_LOOP);      

      for (j = cur_depth + 1; j < prog->num_hyperplanes - 1; j++) {
        pluto_interchange(prog, j, j + 1);
      }      
    }        

    free(dep_dis_hyp);
    free(parallel_time_hyp_id);
    free(parallel_space_hyp_id);

    fprintf(stdout, "[PSA] Completed task interleaving optimization.\n");    

    return 1;
  } else {
    /* No opportunity for task interleaving */
    free(dep_dis_hyp);
    free(parallel_time_hyp_id);
    free(parallel_space_hyp_id);

    fprintf(stdout, "[PSA] Failed task interleaving optimization.\n");

    return 0;
  }  
}

/* 
 * Find the common loop iterator among all operand arrays (only appear in one 
 * index). If the loop is sync-free or pipelined (reduction), tile the loop and
 * move it innermost. If there is no such loop in the current loop band, see if 
 * we could extend to space loops, tile it and move it innermost.
 */
//int psa_pe_simd_optimize(Band *band, PlutoProg *prog) {
//  unsigned num_tiled_levels = 1;
//  unsigned nloops, l;
//
//  /* Band has to be the innermost band as well */
//  if (!pluto_is_band_innermost(band, num_tiled_levels)) {
//    return 0;
//  }
//
//  Ploop **loops;
//}

// int psa_pe_io_eliminate_optimize(Band *band, PlutoProg *prog) {

// }

/*
 * Intra-PE optimization.
 * Task interleaving, SIMD vectorization, and interior I/O optimization
 */
void psa_pe_optimize(PlutoProg *prog) {  
  unsigned nbands, i;
  // int ret1, ret2, ret3;
  Band **bands = pluto_get_outermost_permutable_bands(prog, &nbands);
  assert(nbands == 1);

#ifdef JIE_DEBUG
  fprintf(stdout, "[Debug] num_hyp: %d\n", prog->num_hyperplanes);
  fprintf(stdout, "[Debug] depth: %d\n", bands[0]->loop->depth);
#endif

  /* Task Interleaving */  
  fprintf(stdout, "[PSA] Apply task interleaving.\n");
  for (i = 0; i < nbands; i++) {
    psa_pe_task_interleave_optimize(bands[i], prog);
  }

  /* Detect hyperplane types */
  pluto_detect_hyperplane_types(prog);
  pluto_detect_hyperplane_types_stmtwise(prog);
  psa_detect_hyperplane_types(prog, prog->array_dim, prog->array_part_dim);
  psa_detect_hyperplane_types_stmtwise(prog, prog->array_dim, prog->array_part_dim);    

  /* Detect properties after transformation */
  pluto_compute_dep_directions(prog);
  pluto_compute_dep_satisfaction(prog);
  psa_compute_dep_distances(prog);

  if (!options->silent) {
    fprintf(stdout, "[PSA] After task interleaving.\n");
    pluto_transformations_pretty_print(prog);
  }

//  /* SIMD Vectorization */
//  for (i = 0; i < nbands; i++) {
//    ret2 = psa_pe_simd_optimize(bands[i], prog);
//  }
//
//  /* Detect properties after transformation */
//  pluto_compute_dep_directions(prog);
//  pluto_compute_dep_satisfaction(prog);
//  psa_compute_dep_distances(prog);
//
//  /* Interior I/O Elimination */
//  for (i = 0; i < nbands; i++) {
//    ret3 = psa_pe_io_eliminate_optimize(bands[i], prog);
//  }
//
//  /* Detect properties after transformation */
//  pluto_compute_dep_directions(prog);
//  pluto_compute_dep_satisfaction(prog);
//  psa_compute_dep_distances(prog);

  pluto_bands_free(bands, nbands);


}