/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 * 
 * Author: Jie Wang
 */
#ifndef _PSA_PE_OPT_H
#define _PSA_PE_OPT_H

#include "pluto.h"
#include "constraints.h"
#include "program.h"
#include "psa_vsa.h"
#include "psa_knobs.h"

#include "clan/clan.h"
#include "candl/candl.h"

#include "pet.h"
#include "osl/scop.h"

/* latency hiding */
int psa_read_latency_hiding_tile_sizes(
  int *tile_sizes,
  int num_tile_dims
);
int psa_latency_hiding_optimize(PlutoProg *prog, VSA *vsa);
int psa_laetncy_hiding_optimize_band(Band *band, PlutoProg *prog);
void psa_latency_hiding_misc_pretty_print(
    const PlutoProg *prog, 
    int num_parallel_time_loop, Ploop **parallel_time_loops,
    int num_parallel_space_loop, Ploop **parallel_space_loops);

/* SIMD vectorization */
int psa_read_simd_tile_sizes(
  int *tile_sizes,
  int num_tile_dims
);
bool is_stride_zero_one(Stmt *stmt, PlutoAccess *acc, int depth, int *is_transform, int *transform_dim);
int psa_is_loop_vectorizable(Ploop *loop, PlutoProg *prog, PSAAccess ***psa_acc, int *num_psa_acc);
int psa_simd_vectorization_optimize(PlutoProg *prog, VSA *vsa);
int psa_simd_vectorization_optimize_band(Band *band, PlutoProg *prog);
void psa_simd_vectorization_misc_pretty_print(
    const PlutoProg *prog,
    int num_simd_loop, Ploop **simd_loops);

/* array partitioning */

/* I/O elimination */
int psa_pe_io_eliminate_optimize(Band *band, PlutoProg *prog, VSA *vsa);
void psa_pe_optimize(PlutoProg *prog, VSA *vsa);

#endif
