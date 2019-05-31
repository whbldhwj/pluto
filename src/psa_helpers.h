// /*
//  * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
//  * 
//  * Author: Jie Wang
//  */
// #ifndef _PSA_HELPERS_H
// #define _PSA_HLEPERS_H

// #include "pluto.h"
// #include "constraints.h"
// #include "program.h"
// #include "psa_vsa.h"

// #include "clan/clan.h"
// #include "candl/candl.h"

// #include "pet.h"
// #include "osl/scop.h"

// // deps 
// Dep *pluto_dep_prog_dup(Dep *d, int num_hyperplanes);
// DepDis get_dep_distance(const Dep *dep, const PlutoProg *prog, int level);
// void psa_compute_dep_distances(PlutoProg *prog);
// bool systolic_array_dep_checker(PlutoProg *prog);
// void rar_scalar_filter(PlutoProg *prog);

// // plutoProg
// PlutoOptions *pluto_options_dup(const PlutoOptions *options);
// Graph *graph_dup(const Graph *graph);
// PlutoProg *pluto_prog_dup(const PlutoProg *prog);

// // psa_array
// PlutoProg **sa_candidates_generation_band(Band *band, int array_dim, 
//               PlutoProg *prog, int *nprogs);
// PlutoProg **sa_candidates_generation(PlutoProg *prog, int *nprogs_p);

// // psa_tile
// int psa_read_tile_sizes(
//   int *tile_sizes,
//   int num_tile_dims, Stmt **stmts, int nstmts, int first_loop
// );
// void psa_tile_band(PlutoProg *prog, Band *band, int *tile_sizes);
// void psa_tile_outermost_permutable_band(PlutoProg *prog, Band *band);
// void psa_permute_outermost_tile_band(PlutoProg *prog, Band *band);
// void psa_array_partition(PlutoProg *prog);

// // psa_opt
// int psa_read_simd_tile_sizes(
//   int *tile_sizes,
//   int num_tile_dims
// );
// int psa_read_task_interleave_tile_sizes(
//   int *tile_sizes,
//   int num_tile_dims
// );
// int psa_pe_task_interleave_optimize(Band *band, PlutoProg *prog);
// bool is_stride_zero_one(Stmt *stmt, PlutoAccess *acc, int depth);
// bool psa_loop_is_vectorizable(Ploop *loop, PlutoProg *prog);
// int psa_pe_simd_optimize(Band *band, PlutoProg *prog);
// int psa_pe_io_eliminate_optimize(Band *band, PlutoProg *prog);
// void psa_pe_optimize(PlutoProg *prog);



// #endif