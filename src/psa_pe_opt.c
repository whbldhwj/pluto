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
    fprintf(stdout, "[PSA] WARNING: ti_tile.sizes not found!\n");
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
 * NOTE: currently only consider space loops
 */
int psa_pe_task_interleave_optimize(Band *band, PlutoProg *prog) {  
  unsigned i, j;

  /* Band has to be the innermost band as well */
  /* TODO: This restriction should be removed in the future */
  if (!pluto_is_band_innermost(band, 0)) {
    return 1;
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
  // for (i = first_time_hyp; i < prog->num_hyperplanes; i++) {
  //   /* The current hyperplane should be a loop */
  //   int psa_h_type = prog->hProps[i].psa_type;
  //   if (IS_PSA_TIME_LOOP(psa_h_type)) {
  //     if (dep_dis_hyp[i] == 0) {
  //       parallel_time_hyp_id[num_parallel_time_loop] = i;
  //       num_parallel_time_loop++;      
  //     }
  //   }
  // }

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
      prog->hProps[cur_depth].psa_type = PSA_H_TASK_INTER_LOOP;
      for (j = cur_depth; j < prog->num_hyperplanes - 1; j++) {
        pluto_interchange(prog, j, j + 1);
      }
    } 

    free(dep_dis_hyp);
    free(parallel_time_hyp_id);
    free(parallel_space_hyp_id);
    return 0;
  } else if (num_parallel_space_loop) {
    /* Tile the sync-free space loop and permute them innermost */
    int tile_sizes[num_parallel_space_loop];
    for (i = 0; i < num_parallel_space_loop; i++) {
      tile_sizes[i] = DEFAULT_TASK_INTERLEAVE_TILE_FACTOR;
    }

    psa_read_task_interleave_tile_sizes(tile_sizes, num_parallel_space_loop);

    /* Complete the array_row_il_factor and array_col_il_factor */
    prog->array_il_factor[0] = 1;
    prog->array_il_factor[1] = 1;

    if (prog->array_dim == 1) {      
      prog->array_il_factor[1] = tile_sizes[0];
    } else if (prog->array_dim == 2) {      
      for (i = 0; i < num_parallel_space_loop; i++) {
        prog->array_il_factor[parallel_space_hyp_id[i] - first_space_hyp] = tile_sizes[i];
      }      
    }

    for (i = 0; i < num_parallel_space_loop; i++) {
      unsigned cur_depth = parallel_space_hyp_id[i];
      unsigned nloops;
      Ploop **loops = psa_get_outermost_loops(
        prog, band->loop->stmts, band->loop->nstmts,
        cur_depth, &nloops
      );      
      assert(nloops == 1);
      psa_tile_loop(prog, loops[0], tile_sizes[i], H_TILE_SPACE_LOOP, PSA_H_SPACE_LOOP);      

      prog->hProps[cur_depth + 1].psa_type = PSA_H_TASK_INTER_LOOP;

      for (j = cur_depth + 1; j < prog->num_hyperplanes - 1; j++) {
        pluto_interchange(prog, j, j + 1);
      }      
    }        

    free(dep_dis_hyp);
    free(parallel_time_hyp_id);
    free(parallel_space_hyp_id);    
    return 0;
  } else {
    /* No opportunity for task interleaving */
    free(dep_dis_hyp);
    free(parallel_time_hyp_id);
    free(parallel_space_hyp_id);
    return 1;
  }  
}

bool is_stride_zero_one(Stmt *stmt, PlutoAccess *acc, int depth) {
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
      return 1;
    }
  }

  for (i = 0; i < newacc->nrows; i++) {
    /* Stride-1 access */
    if (newacc->val[i][depth] == 1) {
      for (j = 0; j < newacc->nrows; j++) {
        if (j == i)
          continue;
        if (newacc->val[j][depth] != 0) {
          break;
        }
      }
      if (j == newacc->nrows) {
        pluto_matrix_free(newacc);
        free(divs);
        return 1;
      }
    }
    /* Stride-0 access */
    for (j = 0; j < newacc->nrows; j++) {
      if (newacc->val[j][depth] != 0) {
        break;
      }
    }
    if (j == newacc->nrows) {
      pluto_matrix_free(newacc);
      free(divs);
      return 1;
    }
  }

  pluto_matrix_free(newacc);
  free(divs);

  return 0;
}

bool psa_loop_is_vectorizable(Ploop *loop, PlutoProg *prog) {
  /* TODO: check if the loop is a reduction loop with single statement */
  int i, j;
  bool ret = 1;
  for (i = 0; i < loop->nstmts; i++) {
    Stmt *stmt = loop->stmts[i];
    for (j = 0; j < stmt->nreads; j++) {
      ret &= is_stride_zero_one(stmt, stmt->reads[j], loop->depth);
    }
    for (j = 0; j < stmt->nwrites; j++) {
      ret &= is_stride_zero_one(stmt, stmt->writes[j], loop->depth);
    }
  }
  return ret;
}

/* 
 * Find the common loop iterator among all operand arrays (only appear in one 
 * index). If the loop is sync-free or pipelined (reduction), tile the loop and
 * move it innermost.
 * TODO: currently only support single-statement reduction vectorization 
 * NOTE: currently only consider time loops
 */
int psa_pe_simd_optimize(Band *band, PlutoProg *prog) {  
  unsigned i, j, l, nloops;  

  /* Band has to be the innermost band as well */
  if (!pluto_is_band_innermost(band, 0)) {
    return 1;
  }

  if (prog->nstmts != 1) {
    /* TODO: Add checker for reduction operator */
    fprintf(stdout, "[PSA] Warning: only support single-statement loop for SIMD vectorization now.\n");
    return 1;
  }

  /* Mark the loop properties */
  unsigned first_space_hyp, first_time_hyp;
  bool first_array_part_hyp_found = false;
  bool first_space_hyp_found = false;
  bool first_time_hyp_found = false;
  for (i = 0; i < prog->num_hyperplanes; i++) {
//#ifdef JIE_DEBUG
//    fprintf(stdout, "[Debug] psa_type: %d\n", prog->hProps[i].psa_type);
//    fprintf(stdout, "[Debug] psa_type_cmp: %d\n", prog->hProps[i].psa_type & 0x04);
//    fprintf(stdout, "[Debug] psa_cmp_result: %d\n", IS_PSA_ARRAY_PART_LOOP(prog->hProps[i].psa_type));
//#endif
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

  Ploop **loops;
  loops = pluto_get_loops_under(
      band->loop->stmts, band->loop->nstmts,
      band->loop->depth + first_time_hyp, prog, &nloops
  );

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
  
  Ploop *best_loop = NULL;
  for (l = 0; l < nloops; l++) {
    /* TODO: add reduction loop detection */
    int psa_h_type = prog->hProps[loops[l]->depth].psa_type;
    if (IS_PSA_TIME_LOOP(psa_h_type) && dep_dis_hyp[loops[l]->depth] != 0) {
      bool is_vectorizable = psa_loop_is_vectorizable(loops[l], prog);
      if (is_vectorizable) {
        best_loop = loops[l];
      }
    }
  }

  if (best_loop) {
    /* Tile the loop and make the tiled loop the innermost */
    int tile_sizes[1];    
    tile_sizes[0] = DEFAULT_SIMD_TILE_FACTOR;    

    psa_read_simd_tile_sizes(tile_sizes, 1);

    /* complete array_simd_factor */
    prog->array_simd_factor = tile_sizes[0];

    psa_tile_loop(prog, best_loop, tile_sizes[0], H_TILE_SPACE_LOOP, PSA_H_TIME_LOOP);
    
    unsigned cur_depth = best_loop->depth;
    prog->hProps[cur_depth + 1].psa_type = PSA_H_SIMD_LOOP;

    for (j = cur_depth + 1; j < prog->num_hyperplanes - 1; j++) {
      pluto_interchange(prog, j, j + 1);
    }

    pluto_loops_free(loops, nloops);
    free(dep_dis_hyp);
    return 0;
  } else {
    /* no opportunity for SIMD vectorization */
    pluto_loops_free(loops, nloops);
    free(dep_dis_hyp);
    return 1;
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

  /* Task Interleaving */  
  fprintf(stdout, "[PSA] Apply task interleaving.\n");
  for (i = 0; i < nbands; i++) {
    ret1 = psa_pe_task_interleave_optimize(bands[i], prog);
    if (ret1 == 0) {
      fprintf(stdout, "[PSA] Completed task interleaving optimizaiton.\n");
      /* Update the fields of VSA */
      prog->array_il_enable = 1;      
    } else {
      fprintf(stdout, "[PSA] Failed task interleaving optimization.\n");
      prog->array_il_enable = 0;
    }
  }

#ifdef JIE_DEBUG
  for (i = 0; i < prog->num_hyperplanes; i++) {
    if (IS_PSA_TASK_INTER_LOOP(prog->hProps[i].psa_type)) {
      fprintf(stdout, "[Debug] after task interleave hyp %d is TASK_INTER_LOOP\n", i);
    }
  }
#endif

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

  /* SIMD Vectorization */
  fprintf(stdout, "[PSA] Apply SIMD vectorization.\n");  
  bands = pluto_get_outermost_permutable_bands(prog, &nbands);
  for (i = 0; i < nbands; i++) {
    ret2 = psa_pe_simd_optimize(bands[i], prog);
    if (ret2 == 0) {
      fprintf(stdout, "[PSA] Completed SIMD vectorization.\n");
    } else {
      fprintf(stdout, "[PSA] Failed SIMD vectorization.\n");
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
  psa_compute_dep_distances(prog);

  if (!options->silent) {
    fprintf(stdout, "[PSA] After SIMD vectorization.\n");
    pluto_transformations_pretty_print(prog);
  }

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
  psa_compute_dep_distances(prog);

  pluto_bands_free(bands, nbands);
}
